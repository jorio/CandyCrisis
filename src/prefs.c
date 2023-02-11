// prefs.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static FILE* GetPrefsStream(const char* openmode)
{
    static char path[1024];
    const char* userDir = SDL_GetPrefPath(NULL, "CandyCrisis");
    snprintf(path, sizeof(path), "%sCandyCrisisPrefs.bin", userDir);
    return fopen(path, openmode);
}

void LoadPrefs()
{
    FILE* stream = GetPrefsStream("rb");
    if (!stream)
    {
        return;
    }

    for (int i = 0; i < arrsize(prefList); i++)
    {
        Preference* pref = &prefList[i];
        
        fseek(stream, 0, SEEK_SET);

        while (!feof(stream))
        {
            int keyLength;
            char key[256];
            unsigned int contentsLength;

            keyLength = fgetc(stream);
            if (keyLength < 0 || feof(stream))
                break;
            fread(key, keyLength, 1, stream);
            key[keyLength] = '\0';
            fread(&contentsLength, sizeof(contentsLength), 1, stream);

            if (!strncmp(key, pref->keyName, strlen(pref->keyName)))
            {
                if (contentsLength != pref->valueLength)
                    break;
                fread(pref->valuePtr, pref->valueLength, 1, stream);
                break;
            }
            else
            {
                fseek(stream, contentsLength, SEEK_CUR);
            }
        }
    }

    fclose(stream);
}

void SavePrefs()
{
    FILE* stream = GetPrefsStream("wb");
    if (!stream)
    {
        return;
    }

    for (int i = 0; i < arrsize(prefList); i++)
    {
        const Preference* pref = &prefList[i];
        fputc((uint8_t) strlen(pref->keyName), stream);
        fwrite(pref->keyName, strlen(pref->keyName), 1, stream);
        fwrite(&pref->valueLength, sizeof(pref->valueLength), 1, stream);
        fwrite(pref->valuePtr, pref->valueLength, 1, stream);
    }
    
    fclose(stream);
}
