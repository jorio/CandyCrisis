// gameticks.c

#include "SDL.h"
#include "gameticks.h"

unsigned long baseTickCount, freezeTickCount;
int freezeLevel;

unsigned long MTickCount()
{
	return (unsigned long) ((float)SDL_GetTicks() * 0.06f);
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

unsigned long GameTickCount( void )
{
	if( freezeLevel < 0 )
		return freezeTickCount - baseTickCount;

	return MTickCount( ) - baseTickCount;
}
