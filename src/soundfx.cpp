// soundfx.c

#include "stdafx.h"
#include "main.h"
#include "soundfx.h"
#include "music.h"

#include "support/cmixer.h"
#include <stdio.h>

static std::vector<cmixer::WavStream> soundBank;
MBoolean                   soundOn = true;
float playerStereoSeparation = 1.0;

void InitSound()
{
    cmixer::InitWithSDL();
    
    for (int index=0; index<kNumSounds; index++)
    {
        const char* path = QuickResourceName("snd", index+128, ".wav");
        if (!FileExists(path))
        {
            Error(path);
        }

        soundBank.emplace_back();
        soundBank.back().InitFromWAVFile(path) ;
    }
}

void ShutdownSound()
{
    soundBank.clear();
    cmixer::ShutdownWithSDL();
}

void PlayMono( short which )
{
    PlayStereoFrequency(2, which, 0);
}

void PlayStereo( short player, short which )
{
    PlayStereoFrequency(player, which, 0);
}

void PlayStereoFrequency( short player, short which, short freq )
{
    if (soundOn)
    {
        auto& effect = soundBank.at(which);
        
        double pan;
        switch (player) {
            case 0: pan = -playerStereoSeparation; break;
            case 1: pan = +playerStereoSeparation; break;
            default: pan = 0.0; break;
        }
        
        effect.SetPan(pan);
        effect.SetPitch(1.0 + freq/16.0);
        effect.Play();
        
        UpdateSound();
    }
}

void UpdateSound()
{
}
