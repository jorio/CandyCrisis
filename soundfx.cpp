// soundfx.c

#include "stdafx.h"
#include "main.h"
#include "soundfx.h"
#include "music.h"

#include "fmod.hpp"
#include "fmod_errors.h"
#include <stdio.h>

FMOD::System              *g_fmod;
static FMOD::Sound        *s_sound[kNumSounds];
MBoolean                   soundOn = true;

void FMOD_ERRCHECK(int result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(FMOD_RESULT(result)));
        abort();
    }
}

void InitSound( void )
{
    FMOD_RESULT   result = FMOD::System_Create(&g_fmod);
    FMOD_ERRCHECK(result);
    
    unsigned int  version;
    result = g_fmod->getVersion(&version);
    FMOD_ERRCHECK(result);
    
    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        abort();
    }
    
    result = g_fmod->init(64, FMOD_INIT_NORMAL, 0);
    FMOD_ERRCHECK(result);
    
    for (int index=0; index<kNumSounds; index++)
    {
        /* NOTE: don't replace the sound flags with FMOD_DEFAULT! This will make some WAVs loop (and fail to release their channels). */
        result = g_fmod->createSound(QuickResourceName("snd", index+128, ".wav"), FMOD_LOOP_OFF | FMOD_2D | FMOD_HARDWARE, 0, &s_sound[index]);
        FMOD_ERRCHECK(result);
    }
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
    struct SpeakerMix
    {
        float left, right, center;
    };
    
    SpeakerMix speakerMixForPlayer[] =
    {
        { 1.0, 0.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 },
    };
    
    const SpeakerMix& mix = speakerMixForPlayer[player];
    
    if (soundOn)
    {
        FMOD::Channel*    channel = NULL;
        FMOD_RESULT       result = g_fmod->playSound(FMOD_CHANNEL_FREE, s_sound[which], true, &channel);
        FMOD_ERRCHECK(result);
        
        result = channel->setSpeakerMix(mix.left, mix.right, mix.center, 0.0, 0.0, 0.0, 0.0, 0.0);
        FMOD_ERRCHECK(result);
        
        float channelFrequency;
        result = s_sound[which]->getDefaults(&channelFrequency, NULL, NULL, NULL);
        FMOD_ERRCHECK(result);
        
        result = channel->setFrequency((channelFrequency * (16 + freq)) / 16);
        FMOD_ERRCHECK(result);
        
        result = channel->setPaused(false);
        FMOD_ERRCHECK(result);
        
        UpdateSound();
    }
}

void UpdateSound()
{
    g_fmod->update();
}
