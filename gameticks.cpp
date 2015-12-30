// gameticks.c

#include "stdafx.h"
#include "gameticks.h"

unsigned int  baseTickCount, freezeTickCount;
int freezeLevel;

unsigned int  MTickCount()
{
	return (unsigned int ) ((float)SDL_GetTicks() * 0.06f);
}

void InitGameTickCount( void )
{
	baseTickCount = freezeTickCount = 0;
	freezeLevel = 0;
}

void FreezeGameTickCount( void )
{
	if( freezeLevel	== 0 ) 
    {
        freezeTickCount = MTickCount( );
    }
	freezeLevel--;
}

void UnfreezeGameTickCount( void )
{
	freezeLevel++;
	if( freezeLevel >= 0 )
	{
		freezeLevel = 0;
		baseTickCount += MTickCount( ) - freezeTickCount;
	}
}

unsigned int  GameTickCount( void )
{
	if( freezeLevel < 0 )
		return freezeTickCount - baseTickCount;

	return MTickCount( ) - baseTickCount;
}
