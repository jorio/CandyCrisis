// soundfx.c

#include "main.h"
#include "soundfx.h"
#include "music.h"

#include "support/cmixer.h"
#include <stdio.h>

MBoolean soundOn = true;

static std::vector<cmixer::WavStream> s_soundBank;
static constexpr float k_playerStereoSeparation = 0.5f;
static constexpr float k_soundEffectGain = 0.7f;

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

        s_soundBank.emplace_back(cmixer::LoadWAVFromFile(path));
        s_soundBank.back().SetInterpolation(true);
    }
}

void ShutdownSound()
{
    for (auto& wavStream : s_soundBank)
    {
        wavStream.RemoveFromMixer();
    }
    s_soundBank.clear();
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
        auto& effect = s_soundBank.at(which);

        double pan;
        switch (player)
        {
            case 0: pan = -k_playerStereoSeparation; break;
            case 1: pan = +k_playerStereoSeparation; break;
            default: pan = 0.0; break;
        }

        effect.Stop();
        effect.SetGain(k_soundEffectGain);
        effect.SetPan(pan);
        effect.SetPitch(1.0 + freq/16.0);
        effect.Play();

        UpdateSound();
    }
}

void UpdateSound()
{
}
