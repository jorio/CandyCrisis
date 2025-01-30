// prefs.c

#include <SDL3/SDL.h>

#include "main.h"
#include "prefs.h"
#include "music.h"
#include "soundfx.h"
#include "hiscore.h"
#include "keyselect.h"

typedef struct Preference
{
    const char*   keyName;
    void*         valuePtr;
    unsigned int  valueLength;
} Preference;

Preference prefList[] =
{
    { "MusicOn",            &musicOn,           sizeof(MBoolean         ) },
    { "SoundOn",            &soundOn,           sizeof(MBoolean         ) },
    { "KeyBindings",        playerKeys,         sizeof(playerKeys       ) },
    { "HighScores",         scores,             sizeof(scores           ) },
    { "BestCombo",          &best,              sizeof(best             ) },
    { "Fullscreen",         &fullscreen,        sizeof(fullscreen       ) },
    { "Widescreen",         &widescreen,        sizeof(widescreen       ) },
    { "CrispUpscaling",     &crispUpscaling,    sizeof(crispUpscaling   ) },
};

static SDL_IOStream* GetPrefsStream(const char* openmode)
{
	static char path[1024];
	char* userDir = SDL_GetPrefPath(NULL, "CandyCrisis");
	SDL_snprintf(path, sizeof(path), "%sCandyCrisisPrefs.bin", userDir);
	SDL_free(userDir);
	return SDL_IOFromFile(path, openmode);
}

void LoadPrefs()
{
	SDL_IOStream* stream = GetPrefsStream("rb");
	if (!stream)
	{
		return;
	}

	for (int i = 0; i < arrsize(prefList); i++)
	{
		Preference* pref = &prefList[i];

		SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET);

		while (1)
		{
			size_t numRead;
			Uint8 keyLength;
			char key[256];
			unsigned int contentsLength;

			numRead = SDL_ReadIO(stream, &keyLength, sizeof(keyLength));
			if (!numRead)
				break;
			SDL_ReadIO(stream, key, keyLength);
			key[keyLength] = '\0';
			SDL_ReadIO(stream, &contentsLength, sizeof(contentsLength));

			if (!SDL_strncmp(key, pref->keyName, SDL_strlen(pref->keyName)))
			{
				if (contentsLength != pref->valueLength)
					break;
				SDL_ReadIO(stream, pref->valuePtr, pref->valueLength);
				break;
			}
			else
			{
				SDL_SeekIO(stream, contentsLength, SDL_IO_SEEK_CUR);
			}
		}
	}

	SDL_CloseIO(stream);
}

void SavePrefs()
{
	SDL_IOStream* stream = GetPrefsStream("wb");
	if (!stream)
	{
		return;
	}

	for (int i = 0; i < arrsize(prefList); i++)
	{
		const Preference* pref = &prefList[i];

		Uint8 keyLength = SDL_strlen(pref->keyName);

		SDL_WriteIO(stream, &keyLength, sizeof(keyLength));
		SDL_WriteIO(stream, pref->keyName, SDL_strlen(pref->keyName));
		SDL_WriteIO(stream, &pref->valueLength, sizeof(pref->valueLength));
		SDL_WriteIO(stream, pref->valuePtr, pref->valueLength);
	}

	SDL_CloseIO(stream);
}
