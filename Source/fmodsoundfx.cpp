// soundfx.c

#include "main.h"
#include "soundfx.h"
#include "music.h"

#include "fmod.h"
#include "fmod_errors.h"

FSOUND_SAMPLE*  sound[kNumSounds];
MBoolean        soundOn = true;

void InitSound( void )
{
	int  index;
	
    if (!FSOUND_Init(44100, 64, FSOUND_INIT_USEDEFAULTMIDISYNTH))
	{
		musicOn = soundOn = false; 
		return;
	}
	
	for( index=0; index<kNumSounds; index++ )
	{
		sound[index] = FSOUND_Sample_Load( FSOUND_UNMANAGED, QuickResourceName( "snd", index+128, ".wav" ), FSOUND_NORMAL | FSOUND_LOOP_OFF | FSOUND_2D, 0 );
		if( sound[index] == NULL )
		{
			Error( "InitSound: files are missing" );
		}
	}
}


void PlayMono( short which )
{
	if( soundOn )
	{
		int chanHandle = FSOUND_PlaySound( FSOUND_FREE, sound[which] );
	}
}

void PlayStereo( short player, short which )
{
	PlayStereoFrequency( player, which, 0 );
}

void PlayStereoFrequency( short player, short which, short freq )
{
	if( soundOn )
	{
		int chanHandle = FSOUND_PlaySoundEx( FSOUND_FREE, sound[which], NULL, true );
		FSOUND_SetPan( chanHandle, player? 255: 0 );
		FSOUND_SetFrequency( chanHandle, (FSOUND_GetFrequency(chanHandle) * (16 + freq)) / 16 );
	    FSOUND_SetPaused( chanHandle, false );
	}
}

void UpdateSound()
{
	// no-op!
}
