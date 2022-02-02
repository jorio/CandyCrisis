// prefs.c

#include "stdafx.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "prefs.h"
#include "music.h"
#include "soundfx.h"
#include "hiscore.h"
#include "keyselect.h"

struct Preference
{
    const char*   keyName;
    void*         valuePtr;
    unsigned int  valueLength;
};

Preference prefList[] =
{
    { "MusicOn",        &musicOn,     sizeof(MBoolean  ) },
    { "SoundOn",        &soundOn,     sizeof(MBoolean  ) },
    { "KeyBindings",    playerKeys,   sizeof(playerKeys) },
    { "HighScores",     scores,       sizeof(scores    ) },
    { "BestCombo",      &best,        sizeof(best      ) },
    { "Fullscreen",     &fullscreen,  sizeof(fullscreen) },
};

static std::fstream GetPrefsStream(std::ios::openmode openmode)
{
    static char path[1024];
    const char* userDir = SDL_GetPrefPath(NULL, "CandyCrisis");
    snprintf(path, sizeof(path), "%sCandyCrisisPrefs.bin", userDir);

    return std::fstream(path, std::ios::binary | openmode);
}

void LoadPrefs()
{
    std::fstream stream;
    try
    {
        stream = GetPrefsStream(std::ios::in);
    }
    catch (...)
    {
        return;
    }

    if (!stream.good())
    {
        return;
    }

    for (Preference& pref: prefList)
    {
        stream.seekg(0, std::ios::beg);
        while (!stream.eof())
        {
            int keyLength;
            char key[256];
            unsigned int contentsLength;

            keyLength = stream.get();
            if (stream.eof()) break;
            stream.read(key, keyLength);
            key[keyLength] = '\0';
            stream.read((char*)&contentsLength, sizeof(contentsLength));

            if (!strncmp(key, pref.keyName, strlen(pref.keyName)))
            {
                if (contentsLength != pref.valueLength)
                    break;
                stream.read((char*) pref.valuePtr, pref.valueLength);
                break;
            }
            else
            {
                stream.seekg(contentsLength, std::ios::cur);
            }
        }
    }
}

void SavePrefs()
{
    std::fstream stream;
    try
    {
        stream = GetPrefsStream(std::ios::out);
    }
    catch (...)
    {
        return;
    }

    if (!stream.good())
    {
        return;
    }

    for (Preference& pref: prefList)
    {
        stream.put(strlen(pref.keyName));
        stream.write(pref.keyName, strlen(pref.keyName));
        stream.write((const char*)&pref.valueLength, sizeof(pref.valueLength));
        stream.write((const char*)pref.valuePtr, pref.valueLength);
    }
}
