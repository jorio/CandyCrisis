// prefs.c

#include "stdafx.h"

#if __APPLE__
#include <Cocoa/Cocoa.h>
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
#endif

#if _WIN32
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


void LoadPrefs()
{
    for (Preference& pref: prefList)
    {
        #if __APPLE__
            NSData* data = [[NSUserDefaults standardUserDefaults] dataForKey:pref.keyName];
            if ([data length] == pref.valueLength)
            {
                memcpy(pref.valuePtr, [data bytes], pref.valueLength);
            }
        #endif
        #if _WIN32
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
        #endif
    }
}


void SavePrefs()
{
    for (Preference& pref: prefList)
    {
        #if __APPLE__
            [[NSUserDefaults standardUserDefaults]
                setObject:[NSData dataWithBytes:pref.valuePtr length:pref.valueLength]
                   forKey:pref.keyName];
        #endif
        #if _WIN32
            HKEY hKey;
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\CandyCrisis"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL))
            {
                RegSetValueEx(hKey, pref.keyName, 0, REG_BINARY, reinterpret_cast<BYTE*>(pref.valuePtr), pref.valueLength);
                RegCloseKey(hKey);
            }
        #endif
    }

#if __APPLE__
    [[NSUserDefaults standardUserDefaults] synchronize];
#endif
}
