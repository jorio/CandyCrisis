// prefs.h

void LoadPrefs( void );
void SavePrefs( void );

void GeneratePrefsFile( void );
long UniqueSystem( void );

unsigned char *FindPrefsLine( unsigned char *prefsText, long prefsLength, long searchCode, long dataQuantity );

typedef struct
{
	long itemName;
	void *itemPointer;
	short size;
}
PrefList;
