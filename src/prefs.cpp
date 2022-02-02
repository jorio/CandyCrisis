// prefs.c

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
    { "MusicOn",            &musicOn,           sizeof(MBoolean         ) },
    { "SoundOn",            &soundOn,           sizeof(MBoolean         ) },
    { "KeyBindings",        playerKeys,         sizeof(playerKeys       ) },
    { "HighScores",         scores,             sizeof(scores           ) },
    { "BestCombo",          &best,              sizeof(best             ) },
    { "Fullscreen",         &fullscreen,        sizeof(fullscreen       ) },
    { "Widescreen",         &widescreen,        sizeof(widescreen       ) },
    { "CrispUpscaling",     &crispUpscaling,    sizeof(crispUpscaling   ) },
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

void ParseCommandLine(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        const char* arg = argv[i];

        if (!strcmp(arg, "--crisp")) crispUpscaling = true;
        if (!strcmp(arg, "--fullscreen")) fullscreen = true;
        if (!strcmp(arg, "--widescreen")) widescreen = true;

        if (!strcmp(arg, "--no-crisp")) crispUpscaling = false;
        if (!strcmp(arg, "--no-fullscreen")) fullscreen = false;
        if (!strcmp(arg, "--no-widescreen")) widescreen = false;

        if (!strcmp(arg, "--help") || !strcmp(arg, "-h"))
        {
            printf(
                    "Candy Crisis source port - https://github.com/jorio/candycrisis\n"
                    "\n"
                    "    --crisp           pixel-perfect upscaling\n"
                    "    --no-crisp        upscale with bilinear filtering\n"
                    "    --fullscreen      run the game fullscreen\n"
                    "    --no-fullscreen   run the game in a window\n"
                    "    --widescreen      crop viewport to 16:9 aspect ratio\n"
                    "    --no-widescreen   use original 4:3 aspect ratio\n"
                    "\n"
            );
            exit(0);
        }
    }
}
