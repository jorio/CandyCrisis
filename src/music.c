// music.c

#include <string.h>
#include "main.h"
#include "music.h"
#include "gworld.h"
#include "gameticks.h"
#include "soundfx.h"
#include "graphics.h"
#include "support/cmixer.h"

#define k_noMusic (-1)
#define k_songs 14

MBoolean                musicOn = true;
int                     musicSelection = k_noMusic;

static MBoolean         s_musicFast = false;
int                     s_musicPaused = 0;

static struct CMVoice* s_musicChannel = NULL;

void EnableMusic( MBoolean on )
{
    if (s_musicChannel)
    {
        CMVoice_SetGain(s_musicChannel, on ? 1 : 0);
    }
}

void FastMusic( void )
{
    if (s_musicChannel && !s_musicFast)
    {
        CMVoice_SetMODPlaybackSpeed(s_musicChannel, 1.3);
        s_musicFast = true;
    }
}

void SlowMusic( void )
{
    if (s_musicChannel && s_musicFast)
    {
        CMVoice_SetMODPlaybackSpeed(s_musicChannel, 1.0);
        s_musicFast = false;
    }
}

void PauseMusic( void )
{
    if (s_musicChannel)
    {
        CMVoice_Pause(s_musicChannel);
        s_musicPaused++;
    }
}

void ResumeMusic( void )
{
    if (s_musicChannel)
    {
        CMVoice_Play(s_musicChannel);
        s_musicPaused--;
    }
}

int GetCurrentMusic( void )
{
	return musicSelection;
}

void ChooseMusic( short which )
{
    // Kill existing song first, if any
    ShutdownMusic();

    musicSelection = -1;

    if (which >= 0 && which <= k_songs)
    {
        const char* qrn = QuickResourceName("mod", which+128, ".mod");
        if (!FileExists(qrn))
        {
            qrn = QuickResourceName("mod", which+128, ".s3m");
        }
        if (!FileExists(qrn))
        {
            return;
        }

        s_musicChannel = CMVoice_LoadMOD(qrn);

        EnableMusic(musicOn);
        CMVoice_Play(s_musicChannel);
    
        musicSelection = which;
        s_musicPaused  = 0;
    }
}

void ShutdownMusic()
{
    if (s_musicChannel)
    {
        CMVoice_Free(s_musicChannel);
        s_musicChannel = NULL;
    }
}
