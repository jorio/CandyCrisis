cmake_minimum_required(VERSION 3.16)
project(CandyCrisis)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(SDL2 REQUIRED)
find_package( SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.c src/*.h)

add_executable(CandyCrisis ${GAME_SOURCES})

target_link_libraries(CandyCrisis ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} SDL2 SDL2_image)
