// keyselect.c

#include "SDLU.h"

#include <ctype.h>

#include "main.h"
#include "players.h"
#include "keyselect.h"


SDL_Keycode playerKeys[2][4] =
{
	{ SDLK_A, SDLK_D, SDLK_X, SDLK_S },
	{ SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP }
};

const SDL_Keycode defaultPlayerKeys[2][4] =
{
	{ SDLK_A, SDLK_D, SDLK_X, SDLK_S },
	{ SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP }
};


void CheckKeys()
{
	int player;
	int arraySize;
	const bool* pressedKeys;
				                 
	SDLU_PumpEvents();          
	pressedKeys = SDL_GetKeyboardState( &arraySize );
    
    // Check for game keys
	for( player = 0; player < 2; player++ )
	{
        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][0], NULL)])
			hitKey[player].left++;
		else
			hitKey[player].left = 0;


        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][1], NULL)])
			hitKey[player].right++;
		else
			hitKey[player].right = 0;


        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][2], NULL)])
			hitKey[player].drop++;
		else
			hitKey[player].drop = 0;


        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][3], NULL)])
			hitKey[player].rotate++;
		else
			hitKey[player].rotate = 0;
	}
	
	pauseKey = pressedKeys[SDL_SCANCODE_ESCAPE];
}
