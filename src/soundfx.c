// soundfx.c

#include "support/cmixer.h"
#include <stdio.h>

#include "main.h"
#include "soundfx.h"
#include "music.h"

MBoolean soundOn = true;

#define k_playerStereoSeparation (0.5f)
#define k_soundEffectGain (0.7f)
static CMVoicePtr s_soundBank[kNumSounds];

void InitSound()
{
    cmixer_InitWithSDL();

    for (int i = 0; i < kNumSounds; i++)
    {
        const char* path = QuickResourceName("snd", i+128, ".wav");
        if (!FileExists(path))
        {
            Error(path);
        }

        s_soundBank[i] = CMVoice_LoadWAV(path);
        CMVoice_SetInterpolation(s_soundBank[i], true);
    }
}

void ShutdownSound()
{
    for (int i = 0; i < kNumSounds; i++)
    {
        CMVoice_Free(s_soundBank[i]);
        s_soundBank[i] = NULL;
    }

    cmixer_ShutdownWithSDL();
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
        CMVoicePtr effect = s_soundBank[which];

        double pan;
        switch (player)
        {
            case 0: pan = -k_playerStereoSeparation; break;
            case 1: pan = +k_playerStereoSeparation; break;
            default: pan = 0.0; break;
        }

        //CMVoice_Stop(effect);
        CMVoice_Rewind(effect);
        CMVoice_SetGain(effect, k_soundEffectGain);
        CMVoice_SetPan(effect, pan);
        CMVoice_SetPitch(effect, 1.0 + freq/16.0);
        CMVoice_Play(effect);

        UpdateSound();
    }
}

void UpdateSound()
{
}
