// prefs.c

// NOTE THAT NONE OF THIS CODE IS ENDIAN-SAVVY.
// PREFERENCES FILES WILL NOT TRANSFER BETWEEN PLATFORMS.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "prefs.h"
#include "music.h"
#include "soundfx.h"
#include "hiscore.h"
#include "keyselect.h"

#define kPrefsMaxSize 65536

PrefList prefList[] = {
		{ 'mod ', &musicOn,                    sizeof( MBoolean       ) },
		{ 'sfx ', &soundOn,                    sizeof( MBoolean       ) },
		{ 'size', registeredKey,               sizeof( registeredKey  ) },
		{ 'keys', playerKeys,                  sizeof( playerKeys     ) },
		{ 'high', scores,                      sizeof( scores         ) },
		{ 'cmbx', &best,                       sizeof( best           ) },
		{ 'user', registeredName,              sizeof( registeredName ) }
					  };

#define kPrefListSize (sizeof(prefList)/sizeof(prefList[0]))


/* Loads the preferences from a file in the System Folder:Preferences. */

void LoadPrefs( void )
{
	FILE *F;
	int fileSize, count, digitsLeft;
	unsigned char info, *infoAt, *dataAt, *fileData;
	
	F = fopen( QuickResourceName( "Preferences", 0, ".txt" ), "r" );

	if( F != NULL )
	{
		fileData = (unsigned char*) calloc( 1, kPrefsMaxSize );
		if( fileData != NULL )
		{
			fileSize = fread( fileData, 1, kPrefsMaxSize, F );
			if( fileSize >= 0 )
			{
				for( count=0; count<kPrefListSize; count++ )
				{
					infoAt = FindPrefsLine( fileData, fileSize, prefList[count].itemName, prefList[count].size );
					if( infoAt )
					{
						dataAt = (unsigned char*) prefList[count].itemPointer;
						digitsLeft = prefList[count].size;
						
						while( digitsLeft-- )
						{
							info  = ((*infoAt >= 'A')? (*infoAt - 'A' + 0xA): (*infoAt - '0')) << 4;
							infoAt++;
							info |= ((*infoAt >= 'A')? (*infoAt - 'A' + 0xA): (*infoAt - '0'));
							infoAt++;
							
							*dataAt++ = info;
						}
					}
				}
			}
			
			free( fileData );
		}

		fclose( F );
	}
}

/* Finds a specific line in the prefs. */

unsigned char* FindPrefsLine( unsigned char *prefsText, long prefsLength, long searchCode, long dataQuantity )
{
	unsigned char *prefsAt, *check, *endCheck;
	
	for( prefsAt = prefsText; prefsAt < (prefsText+prefsLength-3); prefsAt++ )
	{
		if( (prefsAt[0] == ((searchCode >> 24) & 0xFF)) &&
		    (prefsAt[1] == ((searchCode >> 16) & 0xFF)) &&
		    (prefsAt[2] == ((searchCode >>  8) & 0xFF)) &&
		    (prefsAt[3] == ((searchCode      ) & 0xFF))    ) 
		{
			prefsAt += 6;
			
			// perform sizing check
			
			dataQuantity *= 2; // hexadecimal bytes are 2 chars
			
			if( ((prefsAt + dataQuantity) - prefsText) > prefsLength ) return NULL; // prefs block ended too early
			
			check = prefsAt;
			endCheck = check + dataQuantity;
			while( check < endCheck )
			{
				if( (*check < '0' || *check > '9') && (*check < 'A' || *check > 'F') )
				{
					return NULL; // incorrect size, too short
				}
					
				check++;
			}
			
			if( (*endCheck >= '0' && *endCheck <= '9') || (*endCheck >= 'A' && *endCheck <= 'F') )
			{
				return NULL; // incorrect size, too long
			}
			
			return prefsAt;
		}
	}
	
	return NULL;
}

/* Saves out preferences into a file. */

void SavePrefs( void )
{
	FILE *F;
	short count, size;
	unsigned char* dataAt;
	
	F = fopen( QuickResourceName( "Preferences", 0, ".txt" ), "w" );
	
	if( F != NULL )
	{
		for( count=0; count<kPrefListSize; count++ )
		{
			fprintf( F, "%c%c%c%c: ", (prefList[count].itemName >> 24) & 0xFF, 
			                          (prefList[count].itemName >> 16) & 0xFF,
			                          (prefList[count].itemName >>  8) & 0xFF,
			                          (prefList[count].itemName      ) & 0xFF   );
			
			dataAt = (unsigned char*) prefList[count].itemPointer;
			for( size=0; size<prefList[count].size; size++ )
			{
				fprintf( F, "%02X", *dataAt );
				dataAt++;
			}
			
			fputc( '\n', F );
		}
	}
			
	fclose( F );
}
