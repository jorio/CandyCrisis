///
///  RegAlgorithm.h
///



#include "MTypes.h"

extern const unsigned int keylist[13];
extern const char* scrambleKey;


// for code generator programs only!
unsigned int hash( unsigned char* k, unsigned int length, unsigned int initval );
void CopyFlattened( char* out, const char* in );

// for client code...
MBoolean ValidateCode( const char* name, const char* key );

