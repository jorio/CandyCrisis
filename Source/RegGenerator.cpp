///
///  RegGenerator.cpp
///
///  Command-line tool for generating registration codes.
///
///  John Stiles, 2002/11/1
///


#include <stdio.h>
#include "RegAlgorithm.h"


static void GenerateCode( const char* name, char* outCode )
{
	int          pool;
	char         flatName[256];
	unsigned int value;
	
	CopyFlattened( flatName, name );
	
	switch( flatName[0] )
	{
		case 'A': case 'B': pool = 0;  break;
		case 'C': case 'D': pool = 1;  break;
		case 'E': case 'F': pool = 2;  break;
		case 'G': case 'H': pool = 3;  break;
		case 'I': case 'J': pool = 4;  break;
		case 'K': case 'L': pool = 5;  break;
		case 'M': case 'N': pool = 6;  break;
		case 'O': case 'P': pool = 7;  break;
		case 'Q': case 'R': pool = 8;  break;
		case 'S': case 'T': pool = 9;  break;
		case 'U': case 'V': pool = 10; break;
		case 'W': case 'X': pool = 11; break;
		case 'Y': case 'Z': pool = 12; break;
	}
	
	int flatLength = strlen(flatName);
    
	value = hash( (unsigned char*) flatName, flatLength, keylist[pool] );

	sprintf( outCode, "%c%c%c%c-%c%c%c%c",
						scrambleKey[ (value >> 28) & 0xf ],
						scrambleKey[ (value >> 24) & 0xf ],
						scrambleKey[ (value >> 20) & 0xf ],
						scrambleKey[ (value >> 16) & 0xf ],
						scrambleKey[ (value >> 12) & 0xf ],
						scrambleKey[ (value >>  8) & 0xf ],
						scrambleKey[ (value >>  4) & 0xf ],
						scrambleKey[ (value >>  0) & 0xf ]  );						
}


int main( int argc, char* argv[] )
{
	char code[64];
	
	if( argc != 2 )
	{
		fprintf( stderr, "usage: crisisgen \"User Name\"\n" );
		exit(1);
	}
	
	if( strlen(argv[1]) < 4 )
    {
        fprintf( stderr, "ERROR: name too short\n" );
        exit(1);
    }
    
	GenerateCode( argv[1], code );
 
    if( !ValidateCode( argv[1], code ) )
    {
        fprintf( stderr, "ERROR: code generation failure for name \"%s\"!", argv[1] );
        exit(1);
    }   
    
	printf( "%s\n", code );
	
	return 0;
}
