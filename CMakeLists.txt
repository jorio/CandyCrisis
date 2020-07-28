cmake_minimum_required(VERSION 3.16)
project(Candy_Crisis)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(SDL2 REQUIRED)
find_package( SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

include_directories(.)

add_executable(Candy_Crisis
        blitter.cpp
        blitter.h
        CandyCrisis.cpp
        CandyCrisis.h
        control.cpp
        control.h
        font.cpp
        font.h
        gameticks.cpp
        gameticks.h
        graphics.cpp
        graphics.h
        graymonitor.cpp
        graymonitor.h
        grays.cpp
        grays.h
        gworld.cpp
        gworld.h
        hiscore.cpp
        hiscore.h
        keyselect.cpp
        keyselect.h
        level.cpp
        level.h
        main.cpp
        main.h
        moving.cpp
        moving.h
        MTypes.cpp
        MTypes.h
        music.cpp
        music.h
        next.cpp
        next.h
        opponent.cpp
        opponent.h
        pause.cpp
        pause.h
        players.cpp
        players.h
        prefs.cpp
        prefs.h
        random.cpp
        random.h
        Resource.h
        score.cpp
        score.h
        SDLU.cpp
        SDLU.h
        soundfx.cpp
        soundfx.h
        stdafx.cpp
        stdafx.h
        targetver.h
        tutorial.cpp
        tutorial.h
        tweak.cpp
        tweak.h
        victory.cpp
        victory.h
        zap.cpp
        zap.h)

target_link_libraries(Candy_Crisis ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} SDL2 SDL2_image)