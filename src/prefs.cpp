// prefs.c

#include "stdafx.h"

#if __APPLE__
#include <Cocoa/Cocoa.h>
#else
#include <filesystem>
#include <fstream>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "prefs.h"
#include "music.h"
#include "soundfx.h"
#include "hiscore.h"
#include "keyselect.h"

#if __APPLE__
    #define PREF_NAME(x)   @x
    typedef NSString*      PrefKeyName;
#else
    #define PREF_NAME(x)   x
    typedef const char*    PrefKeyName;
#endif

struct Preference
{
    PrefKeyName   keyName;
    void*         valuePtr;
    unsigned int  valueLength;
};

Preference prefList[] =
{
    { PREF_NAME("MusicOn"),        &musicOn,     sizeof(MBoolean  ) },
    { PREF_NAME("SoundOn"),        &soundOn,     sizeof(MBoolean  ) },
    { PREF_NAME("KeyBindings"),    playerKeys,   sizeof(playerKeys) },
    { PREF_NAME("HighScores"),     scores,       sizeof(scores    ) },
    { PREF_NAME("BestCombo"),      &best,        sizeof(best      ) }
};

#if !__APPLE__ && !_WIN32
static std::fstream GetPrefsStream(std::ios::openmode openmode)
{
    std::filesystem::path path;
    const char *home = getenv("XDG_CONFIG_HOME");
    if (home) {
        path = std::filesystem::path(home);
    } else {
        home = getenv("HOME");
        if (!home) {
            throw std::exception();
        }
        path = std::filesystem::path(home) / ".config";
    }
    path = path.lexically_normal();
    bool exists = std::filesystem::exists(path);
    
    if (!exists) {
        if (openmode == std::ios::out)
            std::filesystem::create_directories(path);
        else
            throw std::exception();
    } else if (!std::filesystem::is_directory(path)) {
        throw;
    }
    path /= "CandyCrisis.prefs";
    
    if (openmode == std::ios::in && !std::filesystem::is_regular_file(path))
        throw std::exception();

    return std::fstream(path, std::ios::binary | openmode);
}
#endif

void LoadPrefs()
{
#if !__APPLE__ && !_WIN32
    std::fstream stream;
    try {
        stream = GetPrefsStream(std::ios::in);
    } catch (...) {
        return;
    }
#endif
    
    for (Preference& pref: prefList)
    {
        #if __APPLE__
            NSData* data = [[NSUserDefaults standardUserDefaults] dataForKey:pref.keyName];
            if ([data length] == pref.valueLength)
            {
                memcpy(pref.valuePtr, [data bytes], pref.valueLength);
            }
        #elif _WIN32
            HKEY hKey;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\CandyCrisis"), 0, KEY_QUERY_VALUE, &hKey))
            {
                DWORD dwType = REG_BINARY;
                DWORD dwLength = pref.valueLength;
                
                if ((ERROR_SUCCESS == RegQueryValueEx(hKey, pref.keyName, 0, &dwType, NULL, &dwLength))
                    && dwType == REG_BINARY
                    && dwLength == pref.valueLength)
                {
                    RegQueryValueEx(hKey, pref.keyName, 0, &dwType, reinterpret_cast<BYTE*>(pref.valuePtr), &dwLength);
                }
                
                RegCloseKey(hKey);
            }
        #else
            stream.seekg(0, std::ios::beg);
            while (!stream.eof()) {
                int keyLength;
                char key[256];
                unsigned int contentsLength;
                
                keyLength = stream.get();
                if (stream.eof()) break;
                stream.read(key, keyLength);
                key[keyLength] = '\0';
                stream.read((char*)&contentsLength, sizeof(contentsLength));
                
                if (!strncmp(key, pref.keyName, strlen(pref.keyName))) {
                    if (contentsLength != pref.valueLength)
                        break;
                    stream.read((char*) pref.valuePtr, pref.valueLength);
                    break;
                } else {
                    stream.seekg(contentsLength, std::ios::cur);
                }
            }
        #endif
    }
}


void SavePrefs()
{
#if !__APPLE__ && !_WIN32
    std::fstream stream;
    try {
        stream = GetPrefsStream(std::ios::out);
    } catch (...) {
        return;
    }
#endif
    
    for (Preference& pref: prefList)
    {
        #if __APPLE__
            [[NSUserDefaults standardUserDefaults]
                setObject:[NSData dataWithBytes:pref.valuePtr length:pref.valueLength]
                   forKey:pref.keyName];
        #elif _WIN32
            HKEY hKey;
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\CandyCrisis"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL))
            {
                RegSetValueEx(hKey, pref.keyName, 0, REG_BINARY, reinterpret_cast<BYTE*>(pref.valuePtr), pref.valueLength);
                RegCloseKey(hKey);
            }
        #else
            stream.put(strlen(pref.keyName));
            stream.write(pref.keyName, strlen(pref.keyName));
            stream.write((const char*)&pref.valueLength, sizeof(pref.valueLength));
            stream.write((const char*)pref.valuePtr, pref.valueLength);
        #endif
    }

#if __APPLE__
    [[NSUserDefaults standardUserDefaults] synchronize];
#endif
}
