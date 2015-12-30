///
///  stdafx.h
///

#pragma once

#if _WIN32

#define _CRT_SECURE_NO_WARNINGS 1

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "SDL.h"
#include "SDL_endian.h"
#include "SDL_image.h"

#endif
#if __APPLE__

#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include <SDL2_image/SDL_image.h>

#endif