cmake_minimum_required(VERSION 3.21)

if(APPLE OR WIN32)
	set(GAME_TARGET "CandyCrisis")
else()
	set(GAME_TARGET "candycrisis")
endif()
set(GAME_FRIENDLY_NAME "Candy Crisis")
set(GAME_BUNDLE_ID "io.jor.candycrisis")
set(GAME_DATA_DIR "CandyCrisisResources")
set(GAME_MAC_COPYRIGHT "https://github.com/jorio/CandyCrisis")

# Apply macOS deployment target and architectures to all subprojects
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version")  # Keep in sync with SDL3's Info.plist
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Target macOS architectures")

set(CMAKE_C_STANDARD	17)

project(${GAME_TARGET} LANGUAGES C
	VERSION 3.0.2)

# Create an option to switch between a system sdl library and a vendored sdl library
option(BUILD_SDL_FROM_SOURCE "Build SDL from source" OFF)
option(SDL_STATIC "Static link SDL" OFF)
option(SANITIZE "Build with asan/ubsan" OFF)
set(CODE_SIGN_IDENTITY "" CACHE STRING "macOS code signing identity. If omitted, the app won't be signed.")

if(NOT WIN32 AND NOT APPLE)
	if(SANITIZE)
		message("Sanitizers enabled")
	else()
		message("Sanitizers disabled (pass -DSANITIZE=1 to enable)")
	endif()
endif()

# Set Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${GAME_TARGET})

set(GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#------------------------------------------------------------------------------
# FIND SDL
#------------------------------------------------------------------------------

# If SDL3_DIR wasn't specified, discover if the user put some prebuilt SDL3 package in the project's root directory
if(NOT DEFINED SDL3_DIR)
	if(APPLE)
		set(_sdl3_maybe "${CMAKE_SOURCE_DIR}/SDL3.framework/Resources/CMake")
	elseif(WIN32)
		set(_sdl3_maybe "${CMAKE_SOURCE_DIR}/SDL3/cmake")
	endif()

	if(DEFINED _sdl3_maybe AND EXISTS "${_sdl3_maybe}")
		set(SDL3_DIR "${_sdl3_maybe}")
	else()
		message("Couldn't find pre-built SDL3 package in: ${_sdl3_maybe}")
	endif()
	unset(_sdl3_maybe)
endif()

if(NOT BUILD_SDL_FROM_SOURCE)
	find_package(SDL3 CONFIG REQUIRED)
	message("Found pre-built SDL3")
else()
	if(NOT DEFINED SDL3_DIR)
		set(SDL3_DIR "${CMAKE_SOURCE_DIR}/SDL")
	endif()
	message("Building SDL3 from source: " ${SDL3_DIR})
	add_subdirectory("${SDL3_DIR}" EXCLUDE_FROM_ALL)
endif()

#------------------------------------------------------------------------------
# GENERATED FILES
#------------------------------------------------------------------------------

# Write header file containing version info
configure_file(${GAME_SOURCE_DIR}/version.h.in ${GAME_SOURCE_DIR}/version.h)

# Readme file containing version info
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/ReadMe.txt.in ${CMAKE_CURRENT_BINARY_DIR}/ReadMe.txt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/version.txt.in ${CMAKE_CURRENT_BINARY_DIR}/version.txt)

#------------------------------------------------------------------------------
# EXECUTABLE TARGET
#------------------------------------------------------------------------------

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS ${GAME_SOURCE_DIR}/*.c ${GAME_SOURCE_DIR}/*.h)

if(WIN32)
	list(APPEND GAME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/app.exe.rc)
elseif(APPLE)
	list(APPEND GAME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/app.icns)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/packaging/app.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

# Executable
add_executable(${GAME_TARGET} ${GAME_SOURCES})

target_include_directories(${GAME_TARGET} PRIVATE ${GAME_SOURCE_DIR})

# Create macOS app bundle with friendly name
if(APPLE)
	set_target_properties(${GAME_TARGET} PROPERTIES OUTPUT_NAME "${GAME_FRIENDLY_NAME}")
endif()

set_target_properties(${GAME_TARGET} PROPERTIES
	#--------------------------------------------------------------------------
	# MSVC/WIN32
	#--------------------------------------------------------------------------

	WIN32_EXECUTABLE					TRUE					# GUI application instead of console application
	VS_DEBUGGER_WORKING_DIRECTORY		"${CMAKE_SOURCE_DIR}"
	VS_DPI_AWARE						"PerMonitor"

	#--------------------------------------------------------------------------
	# APPLE
	#--------------------------------------------------------------------------

	# Build it as an .app bundle
	MACOSX_BUNDLE						TRUE

	# Set up Info.plist values
	MACOSX_BUNDLE_ICON_FILE				"app.icns"	# CFBundleIconFile
	MACOSX_BUNDLE_EXECUTABLE_NAME		"${GAME_TARGET}"		# CFBundleExecutable - executable name inside the bundle
	MACOSX_BUNDLE_SHORT_VERSION_STRING	"${PROJECT_VERSION}"	# CFBundleShortVersionString
	MACOSX_BUNDLE_COPYRIGHT				"${GAME_MAC_COPYRIGHT}"	# NSHumanReadableCopyright (supersedes CFBundleGetInfoString (MACOSX_BUNDLE_INFO_STRING))
	MACOSX_BUNDLE_BUNDLE_NAME			"${GAME_FRIENDLY_NAME}"	# CFBundleName - user-visible short name for the bundle
	MACOSX_BUNDLE_GUI_IDENTIFIER		"${GAME_BUNDLE_ID}"		# CFBundleIdentifier - unique bundle ID in reverse-DNS format

	# Set framework search path to (App bundle)/Contents/Frameworks so the game can use its embedded SDL3.framework
	XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"

	# If CODE_SIGN_IDENTITY is NOT empty: tell Xcode to codesign the app properly
	# Otherwise, if it's empty: explicitly turn off code signing, otherwise downloaded app will be quarantined forever
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY[variant=Debug] ""		# don't bother signing debug build

	# Bundle ID required for code signing - must match CFBundleIdentifier otherwise xcode will complain
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${GAME_BUNDLE_ID}

	# Don't bother with universal builds when we're working on the debug version
	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"

	XCODE_EMBED_FRAMEWORKS							"SDL3.framework"
	XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY		"YES"		# frameworks must be signed by the same developer as the binary
	XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY	"YES"		# not strictly necessary, but that's cleaner
	XCODE_ATTRIBUTE_COPY_PHASE_STRIP[variant=Debug]	"NO"		# avoid "skipping copy phase strip" warning while working on Debug config

	# The following is to pass notarization requirements
	XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME			"YES"
	XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS "NO"
	XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS			"--options=runtime --timestamp"
)

#------------------------------------------------------------------------------
# COMPILER/LINKER OPTIONS
#------------------------------------------------------------------------------

add_compile_definitions(
	"$<$<CONFIG:DEBUG>:_DEBUG>"
)

if(NOT MSVC)
	target_compile_options(${GAME_TARGET} PRIVATE
		-Wall
		-Wextra
		#-Wshadow  # Note: Candy Crisis goes crazy with shadowing
		-Werror=return-type
		-Wstrict-aliasing=2
		-Wno-multichar
	)

	# Sanitizers in debug mode (Linux only)
	# When using a debugger, you should export LSAN_OPTIONS=detect_leaks=0
	if(SANITIZE)
		list(INSERT GAME_LIBRARIES 0 asan ubsan)
		target_compile_options(${GAME_TARGET} PRIVATE
			-fsanitize=alignment
			-fsanitize=address
			-fsanitize=leak
			-fsanitize=undefined
			-fno-omit-frame-pointer
		)
	endif()
else()
	# On Win32, we need NOGDI and NOUSER to be able to define some Mac functions
	# whose names are otherwise taken by Windows APIs.
	target_compile_definitions(${GAME_TARGET} PRIVATE
		WIN32_LEAN_AND_MEAN
		_CRT_SECURE_NO_WARNINGS		# quit whining about snprintf_s
	)

	target_compile_options(${GAME_TARGET} PRIVATE
		/W4
		/wd5105 # see https://developercommunity.visualstudio.com/t/1249671
		/we4013	# treat warning as error: undefined function (assuming extern returning int)
		/MP		# multiprocessor build
		/Zi		# output info to PDB
	)

	# Let executable be debugged with PDB, even in Release builds
	target_link_options(${GAME_TARGET} PRIVATE /DEBUG)

	# Enable console for debug builds
	set_target_properties(${GAME_TARGET} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set_target_properties(${GAME_TARGET} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
endif()

#------------------------------------------------------------------------------
# LINK LIBRARIES
#------------------------------------------------------------------------------

# Explicitly link math lib on Linux
if(NOT APPLE AND NOT WIN32)
	target_link_libraries(${GAME_TARGET} PRIVATE m)
endif()

if(NOT SDL_STATIC)
	target_link_libraries(${GAME_TARGET} PRIVATE SDL3::SDL3)
else()
	target_link_libraries(${GAME_TARGET} PRIVATE SDL3::SDL3-static)
endif()

#------------------------------------------------------------------------------
# COPY ASSETS
#------------------------------------------------------------------------------

if(APPLE)
	set(GAME_DATA_TARGET_LOCATION "$<TARGET_FILE_DIR:${GAME_TARGET}>/../Resources")
else()
	set(GAME_DATA_TARGET_LOCATION "$<TARGET_FILE_DIR:${GAME_TARGET}>/${GAME_DATA_DIR}")
endif()

add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/${GAME_DATA_DIR}" "${GAME_DATA_TARGET_LOCATION}")

# Copy SDL3 DLL for convenience (WARNING: TARGET_RUNTIME_DLLS requires CMake 3.21, so this copy command was separated from the command above.)
if(WIN32)
	add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${GAME_TARGET}> $<TARGET_FILE_DIR:${GAME_TARGET}>)
endif()

#------------------------------------------------------------------------------
# INSTALL
#------------------------------------------------------------------------------

# Install Windows-specific libraries (cmake --install): copy Visual Studio redistributable DLLs to install location
if(WIN32)
	include(InstallRequiredSystemLibraries)
elseif(APPLE)
	# no-op
else()
	install(TARGETS ${GAME_TARGET} DESTINATION bin)
	install(FILES ${CMAKE_SOURCE_DIR}/packaging/${GAME_TARGET}.desktop DESTINATION share/applications/)
	install(FILES ${CMAKE_SOURCE_DIR}/packaging/${GAME_TARGET}.png DESTINATION share/icons/hicolor/128x128/apps/)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/${GAME_DATA_DIR}/ DESTINATION share/${GAME_TARGET}/)
endif()
