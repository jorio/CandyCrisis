// fmodmusic.c

#include <string.h>

#include "main.h"
#include "music.h"
#include "gworld.h"
#include "gameticks.h"
#include "soundfx.h"
#include "graphics.h"

#include "fmod.h"
#include "fmod_errors.h"

const int kNoMusic = -1;

MBoolean musicOn = true, musicFast = false;
int musicLevel = 0, musicSelection = kNoMusic;
FMUSIC_MODULE *musicModule = NULL;

void EnableMusic( MBoolean on )
{
	FMUSIC_SetMasterVolume( musicModule, on? 192: 0 );
}

void FastMusic( void )
{
	if( musicModule && !musicFast )
	{
		FMUSIC_SetMasterSpeed( musicModule, 1.3f );
		musicFast = true;
	}
}

void SlowMusic( void )
{
	if( musicModule && musicFast )
	{
		FMUSIC_SetMasterSpeed( musicModule, 1.0f );
		musicFast = false;
	}
}

void PauseMusic( void )
{
	if( musicSelection >= 0 && musicSelection <= kSongs )
	{
		FMUSIC_SetPaused( musicModule, true );
		musicLevel++;
	}
}

void ResumeMusic( void )
{
	if( musicSelection >= 0 && musicSelection <= kSongs )
	{
		musicLevel--;
		FMUSIC_SetPaused( musicModule, false );
	}
}

void ChooseMusic( short which )
{
	if( musicSelection >= 0 && musicSelection <= kSongs )	
	{
		FMUSIC_StopSong( musicModule );
		FMUSIC_FreeSong( musicModule );
		musicModule = NULL;
	}

	if( which >= 0 && which <= kSongs )
	{
		musicModule = FMUSIC_LoadSong( QuickResourceName( "mod", which+128, "" ) );		
		if( musicModule != NULL )
		{
			FMUSIC_SetMasterVolume( musicModule, musicOn? 192: 0 );
			FMUSIC_SetLooping( musicModule, true );
			FMUSIC_PlaySong( musicModule );
			musicSelection = which;
			musicLevel     = 0;
		}
	}
}
