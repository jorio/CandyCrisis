/*

Derivative work of cmixer by rxi (https://github.com/rxi/cmixer)

Copyright (c) 2017 rxi
Copyright (c) 2023 Iliyas Jorio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/

#include "cmixer.h"
#include "ibxm.h"

#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_CONCURRENT_VOICES 8
#define BUFFER_SIZE 512

#define CM_DIE(message) \
do { \
	char buf[256]; \
	SDL_snprintf(buf, sizeof(buf), "%s:%d: %s", __func__, __LINE__, (message)); \
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "cmixer", buf, NULL); \
	abort(); \
} while(0)

#define CM_ASSERT(assertion, message) do { if (!(assertion)) CM_DIE(message); } while(0)

#define FX_BITS (12L)
#define FX_UNIT (1L << FX_BITS)
#define FX_MASK (FX_UNIT - 1)

#define BUFFER_MASK (BUFFER_SIZE - 1)

enum
{
	PCMFORMAT_NULL		= 0x00,
	PCMFORMAT_1CH_8		= 0x11,
	PCMFORMAT_2CH_8		= 0x21,
	PCMFORMAT_1CH_LE16	= 0x12,
	PCMFORMAT_2CH_LE16	= 0x22,
	PCMFORMAT_1CH_BE16	= PCMFORMAT_1CH_LE16 | 0x80,
	PCMFORMAT_2CH_BE16	= PCMFORMAT_2CH_LE16 | 0x80,
};

struct CMWavStream
{
	uint8_t pcmformat;
	int idx;

	char* data;
	size_t dataLength;
	bool ownData;

	uint32_t cookie;
};

struct CMModStream
{
	struct module* module;
	struct replay* replay;

	char* moduleFileMemory;
	int* replayBuffer;

	int replayBufferOffset;
	int replayBufferSamples;
	double playbackSpeedMult;

	uint32_t cookie;
};

struct CMVoice
{
	int16_t pcmbuf[BUFFER_SIZE];	// Internal buffer with raw stereo PCM
	int sampleRate;					// Stream's native samplerate
	int sampleCount;				// Stream's length in frames
	int sustainOffset;				// Offset of the sustain loop in frames
	int end;						// End index for the current play-through
	int state;						// Current state (playing|paused|stopped)
	int64_t position;				// Current playhead position (fixed point)
	int lgain, rgain;				// Left and right gain (fixed point)
	int rate;						// Playback rate (fixed point)
	int nextfill;					// Next frame idx where the buffer needs to be filled
	bool loop;						// Whether the voice will loop when `end` is reached
	bool rewind;					// Whether the voice will rewind before playing
	bool active;					// Whether the voice is part of `voices` list
	bool interpolate;				// Interpolated resampling when played back at a non-native rate
	double gain;					// Gain set by `cm_set_gain()`
	double pan;						// Pan set by `cm_set_pan()`

	struct
	{
		void (*fillBuffer)(struct CMVoice* voice, int16_t* into, int len);
		void (*completed)(struct CMVoice* voice);
		void (*rewind)(struct CMVoice* voice);
		void (*free)(struct CMVoice* voice);
	} callbacks;

	uint32_t cookie;

	union
	{
		struct CMWavStream wav;
		struct CMModStream mod;
	};
};

typedef struct CMVoice CMVoice;
typedef struct CMWavStream CMWavStream;
typedef struct CMModStream CMModStream;

static void CMVoice_RemoveFromMixer(CMVoice* voice);
static void CMVoice_AddToMix(CMVoice* voice, int len, int32_t* dst);

static inline CMWavStream* CMWavStream_Check(CMVoice* voice);
static void StreamWav(CMVoice* voice, int16_t* output, int length);
static void RewindWav(CMVoice* voice);
static void FreeWav(CMVoice* voice);

static inline CMModStream* CMModStream_Check(CMVoice* voice);
static void StreamMod(CMVoice* voice, int16_t* output, int length);
static void FreeMod(CMVoice* voice);

//-----------------------------------------------------------------------------
// Utilities

static inline int DoubleToFixed(double f)
{
	return (int) (f * FX_UNIT);
}

static inline double FixedToDouble(int f)
{
	return (double) f / FX_UNIT;
}

static inline int FixedLerp(int a, int b, int p)
{
	return a + (((b - a) * p) >> FX_BITS);
}

static inline int16_t UnpackI16BE(const void* data)
{
#if __BIG_ENDIAN__
	// no-op on big-endian systems
	return *(const uint16_t*) data;
#else
	const uint8_t* p = (uint8_t*) data;
	return	( p[0] << 8 )
		|	( p[1]      );
#endif
}

static inline int16_t UnpackI16LE(const void* data)
{
#if __BIG_ENDIAN__
	const uint8_t* p = (uint8_t*) data;
	return	( p[0]      )
		|	( p[1] << 8 );
#else
	// no-op on little-endian systems
	return *(const uint16_t*) data;
#endif
}

static inline int MinInt(int a, int b) { return a < b ? a : b; }
static inline int MaxInt(int a, int b) { return a < b ? b : a; }

static inline int ClampInt(int x, int a, int b) { return x < a ? a : x > b ? b : x; }
static inline double ClampDouble(double x, double a, double b) { return x < a ? a : x > b ? b : x; }

static char* LoadFile(const char* filename, size_t* outSize)
{
	SDL_IOStream* ifs = SDL_IOFromFile(filename, "rb");
	if (!ifs)
		return NULL;

	SDL_SeekIO(ifs, 0, SDL_IO_SEEK_END);
	long filesize = SDL_TellIO(ifs);
	SDL_SeekIO(ifs, 0, SDL_IO_SEEK_SET);

	void* bytes = SDL_malloc(filesize);
	SDL_ReadIO(ifs, bytes, filesize);
	SDL_CloseIO(ifs);

	if (outSize)
		*outSize = filesize;

	return (char*)bytes;
}

static uint8_t BuildPCMFormat(int bitdepth, int channels, bool bigEndian)
{
	return ((!!bigEndian) << 7)
		| (channels << 4)
		| (bitdepth / 8);
}

//-----------------------------------------------------------------------------
// Global mixer

static struct Mixer
{
	SDL_Mutex* sdlAudioMutex;
	CMVoice* voices[MAX_CONCURRENT_VOICES];	// List of active (playing) voices
	int32_t pcmmixbuf[BUFFER_SIZE];			// Internal master buffer
	int16_t pcmclipbuf[BUFFER_SIZE];		// Internal clip buffer
	int samplerate;							// Master samplerate
	int gain;								// Master gain (fixed point)
} gMixer;

static void Mixer_Init(struct Mixer* mixer, int samplerate);
static void SDLCALL Mixer_Callback(void* opaqueMixer, SDL_AudioStream* stream, int additionalAmount, int totalAmount);
static void Mixer_Lock(struct Mixer* mixer);
static void Mixer_Unlock(struct Mixer* mixer);
static void Mixer_SetMasterGain(struct Mixer* mixer, double newGain);

//-----------------------------------------------------------------------------
// Global init/shutdown

static bool sdlAudioSubSystemInited = false;
static SDL_AudioDeviceID sdlDeviceID = 0;

// SDL2 used to offer SDL_AUDIO_ALLOW_FREQUENCY_CHANGE to avoid crackles and
// pops if the hardware doesn't work at the exact frequency we've asked for
// (typically we'd ask for 44100 and get back 48000).
// I couldn't find an equivalent to SDL_AUDIO_ALLOW_FREQUENCY_CHANGE in SDL3.
// So, use this function before opening the audio device to query the hardware
// for its preferred frequency (sample rate).
static int GetHardwareFrequency(void)
{
	SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL, NULL, NULL);
	CM_ASSERT(stream, SDL_GetError());

	SDL_AudioSpec spec;
	bool success = SDL_GetAudioStreamFormat(stream, NULL, &spec);
	CM_ASSERT(success, SDL_GetError());

	sdlDeviceID = SDL_GetAudioStreamDevice(stream);
	CM_ASSERT(sdlDeviceID, SDL_GetError());
	SDL_CloseAudioDevice(sdlDeviceID);

	return spec.freq;
}

void cmixer_InitWithSDL(void)
{
	CM_ASSERT(!sdlAudioSubSystemInited, "SDL audio subsystem already inited");

	if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
		CM_DIE(SDL_GetError());

	sdlAudioSubSystemInited = true;

	// Init SDL audio
	SDL_AudioSpec spec = {.format=SDL_AUDIO_S16, .channels=2, .freq=GetHardwareFrequency()};
	SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Mixer_Callback, &gMixer);
	CM_ASSERT(stream, SDL_GetError());

	sdlDeviceID = SDL_GetAudioStreamDevice(stream);
	CM_ASSERT(sdlDeviceID, SDL_GetError());

	// Init mixer library
	Mixer_Init(&gMixer, spec.freq);
	Mixer_SetMasterGain(&gMixer, 0.5);

	// Start audio
	SDL_ResumeAudioDevice(sdlDeviceID);
}

void cmixer_ShutdownWithSDL()
{
	if (sdlDeviceID)
	{
		SDL_CloseAudioDevice(sdlDeviceID);
		sdlDeviceID = 0;
	}
	if (gMixer.sdlAudioMutex)
	{
		SDL_DestroyMutex(gMixer.sdlAudioMutex);
		gMixer.sdlAudioMutex = NULL;
	}
	if (sdlAudioSubSystemInited)
	{
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		sdlAudioSubSystemInited = false;
	}
}

double cmixer_GetMasterGain()
{
	return FixedToDouble(gMixer.gain);
}

void cmixer_SetMasterGain(double newGain)
{
	Mixer_SetMasterGain(&gMixer, newGain);
}

//-----------------------------------------------------------------------------
// Global mixer impl

static void Mixer_Init(struct Mixer* mixer, int newSamplerate)
{
	SDL_memset(mixer, 0, sizeof(*mixer));

	mixer->sdlAudioMutex = SDL_CreateMutex();

	mixer->samplerate = newSamplerate;
	mixer->gain = FX_UNIT;
}

static void Mixer_Lock(struct Mixer* mixer)
{
	SDL_LockMutex(mixer->sdlAudioMutex);
}

static void Mixer_Unlock(struct Mixer* mixer)
{
	SDL_UnlockMutex(mixer->sdlAudioMutex);
}

static void Mixer_SetMasterGain(struct Mixer* mixer, double newGain)
{
	if (newGain < 0)
		newGain = 0;
	mixer->gain = DoubleToFixed(newGain);
}

static int Mixer_AddVoice(struct Mixer* mixer, CMVoice* voice)
{
	CM_ASSERT(voice->callbacks.fillBuffer, "fill buffer callback not set");

	// Look for a free slot
	for (int i = 0; i < MAX_CONCURRENT_VOICES; i++)
	{
		CM_ASSERT(mixer->voices[i] != voice, "voice added twice to mixer");

		if (!mixer->voices[i])
		{
			mixer->voices[i] = voice;
			return i;
		}
	}

	return -1;
}

static void Mixer_RemoveVoice(struct Mixer* mixer, CMVoice* voice)
{
	for (int i = 0; i < MAX_CONCURRENT_VOICES; i++)
	{
		if (mixer->voices[i] == voice)
		{
			mixer->voices[i] = NULL;
			break;
		}
	}
}

static void SDLCALL Mixer_Callback(void* opaqueMixer, SDL_AudioStream *stream, int additionalAmount, int dontcare)
{
	(void) dontcare;

	struct Mixer* mixer = (struct Mixer*) opaqueMixer;
	const int sz = 2;

	int len = additionalAmount / sz;

	// Process in chunks of BUFFER_SIZE if `len` is larger than BUFFER_SIZE
	while (len > BUFFER_SIZE)
	{
		Mixer_Callback(opaqueMixer, stream, BUFFER_SIZE * sz, -1);
		len -= BUFFER_SIZE;
	}

	// Zeroset internal buffer
	SDL_memset(mixer->pcmmixbuf, 0, len * sizeof(mixer->pcmmixbuf[0]));

	// Process active voices
	Mixer_Lock(mixer);
	for (int i = 0; i < MAX_CONCURRENT_VOICES; i++)
	{
		CMVoice* voice = mixer->voices[i];

		if (!voice)
			continue;

		CMVoice_AddToMix(voice, len, mixer->pcmmixbuf);

		// Remove voice from list if it is no longer playing
		if (voice->state != CM_STATE_PLAYING)
		{
			voice->active = false;
			mixer->voices[i] = NULL;
		}
	}
	Mixer_Unlock(mixer);

	// Copy internal buffer to destination and clip
	for (int i = 0; i < len; i++)
	{
		int x = (mixer->pcmmixbuf[i] * mixer->gain) >> FX_BITS;
		mixer->pcmclipbuf[i] = ClampInt(x, -32768, 32767);
	}

	// Feed SDL audio stream
	SDL_PutAudioStreamData(stream, mixer->pcmclipbuf, len * sz);
}

//-----------------------------------------------------------------------------
// Voice implementation

static inline CMVoice* CMVoice_Check(void* ptr)
{
	CMVoice* voice = (CMVoice*) ptr;
	CM_ASSERT(voice->cookie == 'VOIX', "VOIX cookie not found");
	return voice;
}

static CMVoice* CMVoice_New(int sampleRate, int sampleCount)
{
	CMVoice* voice = SDL_calloc(1, sizeof(CMVoice));
	
	voice->cookie = 'VOIX';
	voice->sampleRate = 0;
	voice->sampleCount = 0;
	voice->end = 0;
	voice->state = CM_STATE_STOPPED;
	voice->position = 0;
	voice->lgain = 0;
	voice->rgain = 0;
	voice->rate = 0;
	voice->nextfill = 0;
	voice->loop = false;
	voice->rewind = true;
	voice->interpolate = false;
	voice->gain = 0;
	voice->pan = 0;

	voice->active = false;

	voice->sampleRate = sampleRate;
	voice->sampleCount = sampleCount;
	voice->sustainOffset = 0;
	CMVoice_SetGain(voice, 1);
	CMVoice_SetPan(voice, 0);
	CMVoice_SetPitch(voice, 1);
	CMVoice_SetLoop(voice, false);
	CMVoice_Stop(voice);

	return voice;
}

void CMVoice_Free(CMVoice* voice)
{
	CMVoice_Check(voice);
	CMVoice_RemoveFromMixer(voice);

	if (voice->callbacks.free)
		voice->callbacks.free(voice);

	voice->cookie = 'DEAD';
	SDL_free(voice);
}

static void CMVoice_RemoveFromMixer(CMVoice* voice)
{
	CMVoice_Check(voice);

	Mixer_Lock(&gMixer);
	if (voice->active)
	{
		Mixer_RemoveVoice(&gMixer, voice);
		voice->active = false;
	}
	Mixer_Unlock(&gMixer);
}

void CMVoice_Rewind(CMVoice* voice)
{
	if (voice->callbacks.rewind)
		voice->callbacks.rewind(voice);

	voice->position = 0;
	voice->rewind = false;
	voice->end = voice->sampleCount;
	voice->nextfill = 0;
}

static void CMVoice_AddToMix(CMVoice* voice, int len, int32_t* dst)
{
	CMVoice_Check(voice);		// check pointer validity

	// Do rewind if flag is set
	if (voice->rewind)
	{
		CMVoice_Rewind(voice);
	}

	// Don't process if not playing
	if (voice->state != CM_STATE_PLAYING)
	{
		return;
	}

	// Process audio
	while (len > 0)
	{
		// Get current position frame
		int frame = (int) (voice->position >> FX_BITS);

		// Fill buffer if required
		if (frame + 3 >= voice->nextfill)
		{
			int fillOffset = (voice->nextfill * 2) & BUFFER_MASK;
			int fillLength = BUFFER_SIZE / 2;

			voice->callbacks.fillBuffer(voice, voice->pcmbuf + fillOffset, fillLength);
			voice->nextfill += BUFFER_SIZE / 4;
		}

		// Handle reaching the end of the playthrough
		if (frame >= voice->end)
		{
			// As streams continuously fill the raw buffer in a loop,
			// increment the end idx by one length
			// and continue reading from it another playthrough
			voice->end = frame + voice->sampleCount;

			// Set state and stop processing if we're not set to loop
			if (!voice->loop)
			{
				voice->state = CM_STATE_STOPPED;

				if (voice->callbacks.completed)
					voice->callbacks.completed(voice);

				break;
			}
		}

		// Work out how many frames we should process in the loop
		int n = MinInt(voice->nextfill - 2, voice->end) - frame;
		int count = (n << FX_BITS) / voice->rate;
		count = MaxInt(count, 1);
		count = MinInt(count, len / 2);
		len -= count * 2;

		// Add audio to master buffer
		if (voice->rate == FX_UNIT)
		{
			// Add audio to buffer -- basic
			n = frame * 2;
			for (int i = 0; i < count; i++)
			{
				dst[0] += (voice->pcmbuf[(n    ) & BUFFER_MASK] * voice->lgain) >> FX_BITS;
				dst[1] += (voice->pcmbuf[(n + 1) & BUFFER_MASK] * voice->rgain) >> FX_BITS;
				n += 2;
				dst += 2;
			}
			voice->position += count * FX_UNIT;
		}
		else if (voice->interpolate)
		{
			// Resample audio (with linear interpolation) and add to buffer
			for (int i = 0; i < count; i++)
			{
				n = (int) (voice->position >> FX_BITS) * 2;
				int p = voice->position & FX_MASK;
				int a = voice->pcmbuf[(n    ) & BUFFER_MASK];
				int b = voice->pcmbuf[(n + 2) & BUFFER_MASK];
				dst[0] += (FixedLerp(a, b, p) * voice->lgain) >> FX_BITS;
				n++;
				a = voice->pcmbuf[(n    ) & BUFFER_MASK];
				b = voice->pcmbuf[(n + 2) & BUFFER_MASK];
				dst[1] += (FixedLerp(a, b, p) * voice->rgain) >> FX_BITS;
				voice->position += voice->rate;
				dst += 2;
			}
		}
		else
		{
			// Resample audio (without interpolation) and add to buffer
			for (int i = 0; i < count; i++)
			{
				n = (int) (voice->position >> FX_BITS) * 2;
				dst[0] += (voice->pcmbuf[(n    ) & BUFFER_MASK] * voice->lgain) >> FX_BITS;
				dst[1] += (voice->pcmbuf[(n + 1) & BUFFER_MASK] * voice->rgain) >> FX_BITS;
				voice->position += voice->rate;
				dst += 2;
			}
		}
	}
}

double CMVoice_GetLength(const CMVoice* voice)
{
	return voice->sampleCount / (double) voice->sampleRate;
}

double CMVoice_GetPosition(const CMVoice* voice)
{
	return ((voice->position >> FX_BITS) % voice->sampleCount) / (double) voice->sampleRate;
}

int CMVoice_GetState(const CMVoice* voice)
{
	return voice->state;
}

static void CMVoice_RecalcGains(CMVoice* voice)
{
	double l = voice->gain * (voice->pan <= 0. ? 1. : 1. - voice->pan);
	double r = voice->gain * (voice->pan >= 0. ? 1. : 1. + voice->pan);
	voice->lgain = DoubleToFixed(l);
	voice->rgain = DoubleToFixed(r);
}

void CMVoice_SetGain(CMVoice* voice, double newGain)
{
	voice->gain = newGain;
	CMVoice_RecalcGains(voice);
}

void CMVoice_SetPan(CMVoice* voice, double newPan)
{
	voice->pan = ClampDouble(newPan, -1.0, 1.0);
	CMVoice_RecalcGains(voice);
}

void CMVoice_SetPitch(CMVoice* voice, double newPitch)
{
	double newRate;
	if (newPitch > 0.)
	{
		newRate = (double)voice->sampleRate / (double) gMixer.samplerate * newPitch;
	}
	else
	{
		newRate = 0.001;
	}
	voice->rate = DoubleToFixed(newRate);
}

void CMVoice_SetLoop(CMVoice* voice, int newLoop)
{
	voice->loop = newLoop;
}

void CMVoice_SetInterpolation(CMVoice* voice, int newInterpolation)
{
	voice->interpolate = newInterpolation;
}

void CMVoice_Play(CMVoice* voice)
{
	CMVoice_Check(voice);	// check pointer validity

	if (voice->sampleCount == 0)
	{
		// Don't attempt to play an empty voice as this would result
		// in instant starvation when filling mixer buffer
		return;
	}

	Mixer_Lock(&gMixer);
	if (!voice->active)
	{
		int rc = Mixer_AddVoice(&gMixer, voice);
		if (rc < 0)
		{
			// couldn't add voice
		}
		else
		{
			voice->state = CM_STATE_PLAYING;
			voice->active = true;
		}
	}
	Mixer_Unlock(&gMixer);
}

void CMVoice_Pause(CMVoice* voice)
{
	voice->state = CM_STATE_PAUSED;
}

void CMVoice_TogglePause(CMVoice* voice)
{
	if (voice->state == CM_STATE_PAUSED)
		CMVoice_Play(voice);
	else if (voice->state == CM_STATE_PLAYING)
		CMVoice_Pause(voice);
}

void CMVoice_Stop(CMVoice* voice)
{
	voice->state = CM_STATE_STOPPED;
	voice->rewind = true;
}

//-----------------------------------------------------------------------------
// WavStream implementation

static inline CMWavStream* CMWavStream_Check(CMVoice* voice)
{
	CM_ASSERT(voice->cookie == 'VOIX', "VOIX cookie not found");
	CM_ASSERT(voice->wav.cookie == 'WAVS', "WAVS cookie not found");
	return &voice->wav;
}

static CMWavStream* InstallWavStream(CMVoice* voice)
{
	CMWavStream* wav = &voice->wav;

	wav->cookie = 'WAVS';
	wav->pcmformat = PCMFORMAT_NULL;
	wav->idx = 0;

	voice->callbacks.fillBuffer = StreamWav;
	voice->callbacks.rewind = RewindWav;
	voice->callbacks.free = FreeWav;

	return wav;
}

static void FreeWav(CMVoice* voice)
{
	CMWavStream* wav = CMWavStream_Check(voice);

	if (!wav->data)
	{
		return;
	}

	if (wav->ownData)
	{
		SDL_free(wav->data);
	}

	wav->data = NULL;
	wav->dataLength = 0;
	wav->ownData = false;
	wav->cookie = 'DEAD';
}

static void RewindWav(CMVoice* voice)
{
	CMWavStream* wav = CMWavStream_Check(voice);
	wav->idx = 0;
}

static void StreamWav(CMVoice* voice, int16_t* dst, int fillLength)
{
	CMWavStream* wav = CMWavStream_Check(voice);

	int x, n;

	fillLength /= 2;

	const int16_t* data16 = (const int16_t*) wav->data;
	const uint8_t* data8 = (const uint8_t*) wav->data;

#define WAV_PROCESS_LOOP(X) \
	while (n--)             \
	{                       \
		X                   \
		dst += 2;           \
		wav->idx++;         \
	}

	while (fillLength > 0)
	{
		n = MinInt(fillLength, voice->sampleCount - wav->idx);

		fillLength -= n;

		switch (wav->pcmformat)
		{
			case PCMFORMAT_1CH_BE16:
				WAV_PROCESS_LOOP({
					dst[0] = dst[1] = UnpackI16BE(&data16[wav->idx]);
				});
				break;

			case PCMFORMAT_2CH_BE16:
				WAV_PROCESS_LOOP({
					x = wav->idx * 2;
					dst[0] = UnpackI16BE(&data16[x]);
					dst[1] = UnpackI16BE(&data16[x + 1]);
				});
				break;

			case PCMFORMAT_1CH_LE16:
				WAV_PROCESS_LOOP({
					dst[0] = dst[1] = UnpackI16LE(&data16[wav->idx]);
				});
				break;
		
			case PCMFORMAT_2CH_LE16:
				WAV_PROCESS_LOOP({
					x = wav->idx * 2;
					dst[0] = UnpackI16LE(&data16[x]);
					dst[1] = UnpackI16LE(&data16[x + 1]);
				});
				break;

			case PCMFORMAT_1CH_8:
			case PCMFORMAT_1CH_8 | 0x80:		// with big-endian flag
				WAV_PROCESS_LOOP({
					dst[0] = dst[1] = (data8[wav->idx] - 128) << 8;
				});
				break;

			case PCMFORMAT_2CH_8:
			case PCMFORMAT_2CH_8 | 0x80:		// with big-endian flag
				WAV_PROCESS_LOOP({
					x = wav->idx * 2;
					dst[0] = (data8[x] - 128) << 8;
					dst[1] = (data8[x + 1] - 128) << 8;
				});
				break;

			default:
				CM_DIE("unknown pcmformat");
				break;
		}

		// Loop back and continue filling buffer if we didn't fill the buffer
		if (fillLength > 0)
		{
			wav->idx = voice->sustainOffset;
		}
	}

#undef WAV_PROCESS_LOOP
}

//-----------------------------------------------------------------------------
// LoadWAVFromFile

static const char* FindRIFFChunk(const char* data, size_t len, const char* id, int* size)
{
	// TODO : Error handling on malformed wav file
	size_t idlen = SDL_strlen(id);
	const char* p = data + 12;
next:
	*size = *((uint32_t*)(p + 4));
	if (SDL_memcmp(p, id, idlen))
	{
		p += 8 + *size;
		if (p > data + len)
			return NULL;
		goto next;
	}
	return p + 8;
}

CMVoice* CMVoice_LoadWAV(const char* path)
{
	int sz;

	size_t len = 0;
	char *const data = LoadFile(path, &len);

	const char* p = (char*)data;

	// Check header
	if (SDL_memcmp(p, "RIFF", 4) || SDL_memcmp(p + 8, "WAVE", 4))
		CM_DIE("not a WAVE file");

	// Find fmt subchunk
	p = FindRIFFChunk(data, len, "fmt ", &sz);
	CM_ASSERT(p, "no fmt chunk in WAVE");

	// Load fmt info
	int format		= *((uint16_t*)(p));
	int channels	= *((uint16_t*)(p + 2));
	int samplerate	= *((uint32_t*)(p + 4));
	int bitdepth	= *((uint16_t*)(p + 14));
	CM_ASSERT(format == 1, "unsupported WAVE format");
	CM_ASSERT(channels == 1 || channels == 2, "unsupported channel count");
	CM_ASSERT(bitdepth == 8 || bitdepth == 16, "unsupported bitdepth");
	CM_ASSERT(samplerate != 0, "weird samplerate");

	// Find data subchunk
	p = FindRIFFChunk(data, len, "data", &sz);
	CM_ASSERT(p, "no data chunk in WAVE");

	const char* sampleData = p;
	int sampleDataLength = sz;
	int samplecount = (sampleDataLength / (bitdepth / 8)) / channels;

	CMVoice* voice = CMVoice_New(samplerate, samplecount);
	CMWavStream* wav = InstallWavStream(voice);

	wav->pcmformat = BuildPCMFormat(bitdepth, channels, 0);
	wav->data = SDL_malloc(sampleDataLength);
	wav->dataLength = sampleDataLength;
	wav->ownData = true;
	SDL_memcpy(wav->data, sampleData, sampleDataLength);

	SDL_free(data);

	CM_ASSERT(wav->pcmformat != 0, "weird pcmformat");

	return voice;
}

//-----------------------------------------------------------------------------
// ModStream

static inline CMModStream* CMModStream_Check(CMVoice* voice)
{
	CM_ASSERT(voice->cookie == 'VOIX', "VOIX cookie not found");
	CM_ASSERT(voice->mod.cookie == 'MODS', "MODS cookie not found");
	return &voice->mod;
}

CMVoice* CMVoice_LoadMOD(const char* path)
{
	char errors[64];
	errors[0] = '\0';

	size_t moduleFileSize = 0;
	char* moduleFile = LoadFile(path, &moduleFileSize);
	struct data d = { .buffer = moduleFile, .length = (int)moduleFileSize };

	CMVoice* voice = CMVoice_New(gMixer.samplerate, INT_MAX);
	voice->callbacks.fillBuffer = StreamMod;
	voice->callbacks.free = FreeMod;

	voice->mod.cookie = 'MODS';
	voice->mod.replayBuffer = SDL_calloc(1, 2048 * 8 * sizeof(voice->mod.replayBuffer[0]));
	voice->mod.replayBufferOffset = 0;
	voice->mod.replayBufferSamples = 0;
	voice->mod.playbackSpeedMult = 1.0;
	voice->mod.moduleFileMemory = moduleFile;
	voice->mod.module = module_load(&d, errors);
	voice->mod.replay = new_replay(voice->mod.module, gMixer.samplerate, 0);

	CM_ASSERT(!errors[0], errors);

	return voice;
}

static void FreeMod(CMVoice* voice)
{
	CMModStream* mod = CMModStream_Check(voice);

	dispose_module(mod->module);
	SDL_free(mod->moduleFileMemory);
	SDL_free(mod->replayBuffer);

	mod->cookie = 'DEAD';
}

void CMVoice_SetMODPlaybackSpeed(CMVoice* voice, double speed)
{
	CMModStream* mod = CMModStream_Check(voice);

	mod->playbackSpeedMult = speed;
}

static void StreamMod(CMVoice* voice, int16_t* output, int length)
{
	CMModStream* mod = CMModStream_Check(voice);

	length /= 2;

	while (length > 0)
	{
		// refill replay buffer if exhausted
		if (mod->replayBufferSamples == 0)
		{
			mod->replayBufferOffset = 0;
			mod->replayBufferSamples = replay_get_audio(mod->replay, mod->replayBuffer, 0, (int)(mod->playbackSpeedMult * 100.0));
		}

		// number of stereo samples to copy from replay buffer to output buffer
		int nToCopy = MinInt(mod->replayBufferSamples, length);

		int* input = &mod->replayBuffer[mod->replayBufferOffset * 2];

		// Copy samples
		for (int i = 0; i < nToCopy * 2; i++)
		{
			int sample = *(input++);
			sample = ClampInt(sample, -32768, 32767);
			*(output++) = sample;
		}

		mod->replayBufferOffset += nToCopy;
		mod->replayBufferSamples -= nToCopy;
		length -= nToCopy;
	}
}
