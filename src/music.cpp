// music.c

#include "stdafx.h"

#include <string.h>
#include <vector>
#include <fstream>

#include "main.h"
#include "music.h"
#include "gworld.h"
#include "gameticks.h"
#include "soundfx.h"
#include "graphics.h"

#include "support/ModStream.h"

const int               k_noMusic = -1;
const int               k_songs = 14;

MBoolean                musicOn = true;
int                     musicSelection = k_noMusic;

static MBoolean         s_musicFast = false;
int                     s_musicPaused = 0;

static cmixer::ModStream* s_musicChannel = NULL;

void EnableMusic( MBoolean on )
{
    if (s_musicChannel)
    {
        s_musicChannel->SetGain(on? 1.0: 0.0);
    }
}

void FastMusic( void )
{
    if (s_musicChannel && !s_musicFast)
    {
        s_musicChannel->SetPlaybackSpeed(1.3);
        s_musicFast = true;
    }
}

void SlowMusic( void )
{
    if (s_musicChannel && s_musicFast)
    {
        s_musicChannel->SetPlaybackSpeed(1.0);
        s_musicFast = false;
    }
}

void PauseMusic( void )
{
    if (s_musicChannel)
    {
        s_musicChannel->Pause();
        s_musicPaused++;
    }
}

void ResumeMusic( void )
{
    if (s_musicChannel)
    {
        s_musicChannel->Play();
        s_musicPaused--;
    }
}

static std::vector<char> LoadFile(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    auto pos = ifs.tellg();
    std::vector<char> bytes(pos);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&bytes[0], pos);
    return bytes;
}

void ChooseMusic( short which )
{
    if (s_musicChannel != NULL)
    {
        delete s_musicChannel;
        s_musicChannel = NULL;
    }

    musicSelection = -1;
    
    if (which >= 0 && which <= k_songs)
    {
        //printf("Music: %d\n" , which + 128);
        
        auto qrn = QuickResourceName("mod", which+128, ".mod");
        if (!FileExists(qrn)) {
            qrn = QuickResourceName("mod", which+128, ".s3m");
        }
        if (!FileExists(qrn)) {
            return;
        }
        auto rawFileData = LoadFile(qrn);

        s_musicChannel = new cmixer::ModStream(LoadFile(qrn));

        EnableMusic(musicOn);
        s_musicChannel->Play();
    
        musicSelection = which;
        s_musicPaused  = 0;
    }
}

void ShutdownMusic()
{
    if (s_musicChannel)
    {
        delete s_musicChannel;
        s_musicChannel = NULL;
    }
}
