// keyselect.c

#include "stdafx.h"
#include "SDLU.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "players.h"
#include "keyselect.h"


SDL_Keycode playerKeys[2][4] =
{
	{ SDLK_a, SDLK_d, SDLK_x, SDLK_s },
	{ SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP }
};

const SDL_Keycode defaultPlayerKeys[2][4] =
{
	{ SDLK_a, SDLK_d, SDLK_x, SDLK_s },
	{ SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP }
};


void CheckKeys()
{
	int player;
	int arraySize;
	const Uint8* pressedKeys;
				                 
	SDLU_PumpEvents();          
	pressedKeys = SDL_GetKeyboardState( &arraySize );
    
    // Check for game keys
	for( player = 0; player < 2; player++ )
	{
        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][0])])
			hitKey[player].left++; 
		else
			hitKey[player].left = 0;


        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][1])])
			hitKey[player].right++;
		else
			hitKey[player].right = 0;


        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][2])])
			hitKey[player].drop++;
		else
			hitKey[player].drop = 0;


        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][3])])
			hitKey[player].rotate++;
		else
			hitKey[player].rotate = 0;
	}
	
	pauseKey = pressedKeys[SDL_SCANCODE_ESCAPE];
}
