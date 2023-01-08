cmake_minimum_required(VERSION 3.16)

set(GAME_TARGET "CandyCrisis")
set(GAME_MAC_BUNDLE_ID "io.jor.candycrisis")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Target macOS architectures")

set(CMAKE_CXX_STANDARD 20)
if (${CMAKE_VERSION} VERSION_LESS "3.21")
	set(CMAKE_C_STANDARD	11)
else()
	set(CMAKE_C_STANDARD	17)
endif()

project(
	${GAME_TARGET} LANGUAGES C CXX
	VERSION 3.0.0
	)



# Create an option to switch between a system sdl library and a vendored sdl library
option(SYSTEM_SDL "Use system SDL" OFF)

# Set Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${GAME_TARGET})

set(GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#------------------------------------------------------------------------------
# FIND SDL
#------------------------------------------------------------------------------

if(SYSTEM_SDL)
	# 1. Look for an SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
	find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
	
	# 1. Look for an SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available 
	find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
else()
	add_subdirectory(SDL EXCLUDE_FROM_ALL)
endif()

#------------------------------------------------------------------------------
# GENERATED FILES
#------------------------------------------------------------------------------

# Write header file containing version info
configure_file(${GAME_SOURCE_DIR}/version.h.in ${GAME_SOURCE_DIR}/version.h)

#------------------------------------------------------------------------------
# EXECUTABLE TARGET
#------------------------------------------------------------------------------


file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS ${GAME_SOURCE_DIR}/*.cpp ${GAME_SOURCE_DIR}/*.c ${GAME_SOURCE_DIR}/*.h)

if(WIN32)
	list(APPEND GAME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/app.exe.rc)

	add_executable(${GAME_TARGET} WIN32 MACOSX_BUNDLE ${GAME_SOURCES})
elseif(APPLE)
	list(APPEND GAME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/app.icns)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/packaging/app.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

	add_executable(${GAME_TARGET} MACOSX_BUNDLE ${GAME_SOURCES})
else()
	# Math lib, explicitly required on some Linux systems
	#list(APPEND GAME_LIBRARIES m)

	add_executable(${GAME_TARGET} ${GAME_SOURCES})
endif()

target_include_directories(${GAME_TARGET} PRIVATE ${GAME_SOURCE_DIR})



set_target_properties(${GAME_TARGET} PROPERTIES
	#--------------------------------------------------------------------------
	# MSVC/WIN32
	#--------------------------------------------------------------------------

	VS_DEBUGGER_WORKING_DIRECTORY		"${CMAKE_SOURCE_DIR}"
	VS_DPI_AWARE						"PerMonitor"

	#--------------------------------------------------------------------------
	# APPLE
	#--------------------------------------------------------------------------

	# Set framework search path to (App bundle)/Contents/Frameworks so the game can use its embedded SDL2.framework
	XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"

	# Explicitly turn off code signing, otherwise downloaded app will be quarantined forever
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""

	# Set up Info.plist values
	MACOSX_BUNDLE_ICON_FILE				"app.icns"	# CFBundleIconFile
	MACOSX_BUNDLE_EXECUTABLE_NAME		${GAME_TARGET}			# CFBundleExecutable - executable name inside the bundle
	MACOSX_BUNDLE_SHORT_VERSION_STRING	${PROJECT_VERSION}		# CFBundleShortVersionString
#	MACOSX_BUNDLE_COPYRIGHT				${GAME_MAC_COPYRIGHT}	# NSHumanReadableCopyright (supersedes CFBundleGetInfoString (MACOSX_BUNDLE_INFO_STRING))
	MACOSX_BUNDLE_BUNDLE_NAME			"Candy Crisis"			# CFBundleName - user-visible short name for the bundle
	MACOSX_BUNDLE_GUI_IDENTIFIER		${GAME_MAC_BUNDLE_ID}	# CFBundleIdentifier - unique bundle ID in reverse-DNS format

	# Bundle ID required for code signing - must match CFBundleIdentifier otherwise xcode will complain
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${GAME_MAC_BUNDLE_ID}

	# Don't bother with universal builds when we're working on the debug version
	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"

	XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME			"YES"		# required for notarization to pass
)

#------------------------------------------------------------------------------
# LINK LIBRARIES
#------------------------------------------------------------------------------

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications  
if(TARGET SDL2::SDL2main)
	# It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
	target_link_libraries(${GAME_TARGET} PRIVATE SDL2::SDL2main)
endif()

if(SYSTEM_SDL)
	target_link_libraries(${GAME_TARGET} PRIVATE SDL2::SDL2)
else()
	target_link_libraries(${GAME_TARGET} PRIVATE SDL2::SDL2-static)
endif()

#------------------------------------------------------------------------------
# COPY ASSETS
#------------------------------------------------------------------------------

if(APPLE)
	set(GAME_DATA_TARGET_LOCATION "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources")
else()
	set(GAME_DATA_TARGET_LOCATION "$<TARGET_FILE_DIR:${PROJECT_NAME}>/CandyCrisisResources")
endif()

add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/CandyCrisisResources" "${GAME_DATA_TARGET_LOCATION}")

#------------------------------------------------------------------------------
# INSTALL
#------------------------------------------------------------------------------

# Install Windows-specific libraries
if(WIN32)
	# When installing (cmake --install), copy Visual Studio redistributable DLLs to install location
	include(InstallRequiredSystemLibraries)
endif()
