cmake_minimum_required(VERSION 3.16)
project(Candy_Crisis)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(SDL2 REQUIRED)
find_package( SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

include_directories(.)

add_executable(Candy_Crisis
        src/support/cmixer.cpp
        src/support/cmixer.h
        src/support/ibxm.c
        src/support/ibxm.h
        src/support/ModStream.cpp
        src/support/ModStream.h
        src/blitter.cpp
        src/blitter.h
        src/CandyCrisis.cpp
        src/CandyCrisis.h
        src/control.cpp
        src/control.h
        src/font.cpp
        src/font.h
        src/gameticks.cpp
        src/gameticks.h
        src/graphics.cpp
        src/graphics.h
        src/graymonitor.cpp
        src/graymonitor.h
        src/grays.cpp
        src/grays.h
        src/gworld.cpp
        src/gworld.h
        src/hiscore.cpp
        src/hiscore.h
        src/keyselect.cpp
        src/keyselect.h
        src/level.cpp
        src/level.h
        src/main.cpp
        src/main.h
        src/moving.cpp
        src/moving.h
        src/MTypes.cpp
        src/MTypes.h
        src/music.cpp
        src/music.h
        src/next.cpp
        src/next.h
        src/opponent.cpp
        src/opponent.h
        src/pause.cpp
        src/pause.h
        src/players.cpp
        src/players.h
        src/prefs.cpp
        src/prefs.h
        src/random.cpp
        src/random.h
        src/Resource.h
        src/score.cpp
        src/score.h
        src/SDLU.cpp
        src/SDLU.h
        src/soundfx.cpp
        src/soundfx.h
        src/stdafx.cpp
        src/stdafx.h
        src/targetver.h
        src/tutorial.cpp
        src/tutorial.h
        src/tweak.cpp
        src/tweak.h
        src/victory.cpp
        src/victory.h
        src/zap.cpp
        src/zap.h)

target_link_libraries(Candy_Crisis ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} SDL2 SDL2_image)
