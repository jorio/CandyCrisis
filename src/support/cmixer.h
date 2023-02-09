#pragma once

enum
{
	CM_STATE_STOPPED,
	CM_STATE_PLAYING,
	CM_STATE_PAUSED
};

typedef struct CMVoice* CMVoicePtr;
typedef const struct CMVoice* CMVoiceConstPtr;

void					CMVoice_Free(CMVoicePtr voice);
void					CMVoice_Rewind(CMVoicePtr voice);
double					CMVoice_GetLength(CMVoiceConstPtr voice);
double					CMVoice_GetPosition(CMVoiceConstPtr voice);
int						CMVoice_GetState(CMVoiceConstPtr voice);
void					CMVoice_SetGain(CMVoicePtr voice, double gain);
void					CMVoice_SetPan(CMVoicePtr voice, double pan);
void					CMVoice_SetPitch(CMVoicePtr voice, double pitch);
void					CMVoice_SetLoop(CMVoicePtr voice, int loop);
void					CMVoice_SetInterpolation(CMVoicePtr voice, int interpolation);
void					CMVoice_Play(CMVoicePtr voice);
void					CMVoice_Pause(CMVoicePtr voice);
void					CMVoice_TogglePause(CMVoicePtr voice);
void					CMVoice_Stop(CMVoicePtr voice);

CMVoicePtr				CMVoice_LoadWAV(const char* path);

CMVoicePtr				CMVoice_LoadMOD(const char* path);
void					CMVoice_SetMODPlaybackSpeed(CMVoicePtr voice, double speed);

void					cmixer_InitWithSDL(void);
void					cmixer_ShutdownWithSDL(void);
double					cmixer_GetMasterGain(void);
void					cmixer_SetMasterGain(double newGain);
