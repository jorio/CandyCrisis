///
///  RegAlgorithm.cpp
///
///  Registration code algorithm based on hashing functions at
///  http://burtleburtle.net/bob/hash/#lookup
///
///  John Stiles, 2002/10/31
///


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "MTypes.h"
#include "RegAlgorithm.h"


// These bits are completely random, generated from atmospheric noise.
// (http://www.random.org/) They have no significance other than being 
// seeds to the hash function below. 
const unsigned int keylist[13] = {  0x40691e27,
									0xe451380b,
									0x603bb484,
									0x93e4865e,
									0x964c9a5d,
									0xc19591e2,
									0xff90b43a,
									0x0b34b5c3,
									0x5ed19fb7,
									0xdcb4dcaf,
									0x6700fcfa,
									0x7c7ee303,
									0xe2016ffc  }; 


// The letters that the end user will receive in their code. They correspond
// to hex digits [0-F] consecutively. Notice that the letters contain no vowels
// (to avoid making comprehensible words), and no overtly ambiguous characters 
// (O's, I's, L's, zeros, ones, etc).
const char* scrambleKey = "PWCDRSHBNTVFGJMZ";

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bit set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}


/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k     : the key (the unaligned variable-length array of bytes)
  len   : the length of the key, counting by bytes
  level : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 36+6len instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burlteburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

unsigned int hash( unsigned char* k, unsigned int length, unsigned int initval )
{
   unsigned int a, b, c, len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;           /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] + ((unsigned int)k[1]<<8) + ((unsigned int)k[2]<<16)  + ((unsigned int)k[3]<<24));
      b += (k[4] + ((unsigned int)k[5]<<8) + ((unsigned int)k[6]<<16)  + ((unsigned int)k[7]<<24));
      c += (k[8] + ((unsigned int)k[9]<<8) + ((unsigned int)k[10]<<16) + ((unsigned int)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
	   case 11: c+=((unsigned int)k[10]<<24);
	   case 10: c+=((unsigned int)k[9]<<16);
	   case 9 : c+=((unsigned int)k[8]<<8);
	      /* the first byte of c is reserved for the length */
	   case 8 : b+=((unsigned int)k[7]<<24);
	   case 7 : b+=((unsigned int)k[6]<<16);
	   case 6 : b+=((unsigned int)k[5]<<8);
	   case 5 : b+=k[4];
	   case 4 : a+=((unsigned int)k[3]<<24);
	   case 3 : a+=((unsigned int)k[2]<<16);
	   case 2 : a+=((unsigned int)k[1]<<8);
	   case 1 : a+=k[0];
	     /* case 0: nothing left to add */
   }
   
   mix(a,b,c);

   /*-------------------------------------------- report the result */
   return c;
}


void CopyFlattened( char* out, const char* in )
{
	// Convert a name like "John Stiles" to "JOHN STILES" or
	// "Patty O'furniture" to "PATTY OFURNITURE".
	char* start = out;
	
	while( *in == ' ' ) in++;
	
	for( ;; )
	{
		char c = *in++;
		if( c == '\0' ) break;
		
		c = toupper(c);
		if( c >= 'A' && c <= 'Z' || c == ' ' )
		{
			*out++ = c;
		}
	}
	
	while( (out > start) && (out[-1] == ' ') ) out--;
	
    *out++ = '\0';
}


MBoolean ValidateCode( const char* name, const char* key )
{
	// Convert the key (a jumble of letters like BFGJ-TVFF) into a 32-bit
	// hex value. 
	char*        unscrambled[8];
	int          index;
	unsigned int value = 0;
	
	if( strlen(key) != 9 ) return false;
	
	if( key[4] != '-' ) return false;
	
	unscrambled[0] = strchr( scrambleKey, toupper(key[0]) );
	unscrambled[1] = strchr( scrambleKey, toupper(key[1]) );
	unscrambled[2] = strchr( scrambleKey, toupper(key[2]) );
	unscrambled[3] = strchr( scrambleKey, toupper(key[3]) );
	unscrambled[4] = strchr( scrambleKey, toupper(key[5]) ); 
	unscrambled[5] = strchr( scrambleKey, toupper(key[6]) );
	unscrambled[6] = strchr( scrambleKey, toupper(key[7]) );
	unscrambled[7] = strchr( scrambleKey, toupper(key[8]) );
	
	for( index=0; index<8; index++ )
	{
		if( unscrambled[index] == NULL ) return false;
		value = (value << 4) | (unsigned int)(unscrambled[index] - scrambleKey);
	}

	// Get the flattened user name.
	char flatName[256];
	CopyFlattened( flatName, name );
	
	// Return true if any of the hash functions succeed.
	// Note that the hashes and comparisons mix up the ordering of bogus 
	// keys and valid keys to prevent early identification of bogus keys.
	
	int flatLength = strlen(flatName);
	if( flatLength < 4 ) return false;
	
	unsigned int result0  = hash( (unsigned char*) flatName, flatLength, keylist[0] );
	unsigned int result1  = hash( (unsigned char*) flatName, flatLength, keylist[1] );
	unsigned int result2  = hash( (unsigned char*) flatName, flatLength, keylist[2] );
	unsigned int result3  = hash( (unsigned char*) flatName, flatLength, keylist[3] );
	unsigned int result4  = hash( (unsigned char*) flatName, flatLength, keylist[4] );
	unsigned int result5  = hash( (unsigned char*) flatName, flatLength, keylist[5] );
	unsigned int result6  = hash( (unsigned char*) flatName, flatLength, keylist[6] );
	unsigned int result7  = hash( (unsigned char*) flatName, flatLength, keylist[7] );
	unsigned int result8  = hash( (unsigned char*) flatName, flatLength, keylist[8] );
	unsigned int result9  = hash( (unsigned char*) flatName, flatLength, keylist[9] );
	unsigned int result10 = hash( (unsigned char*) flatName, flatLength, keylist[10] );
	unsigned int result11 = hash( (unsigned char*) flatName, flatLength, keylist[11] );
	unsigned int result12 = hash( (unsigned char*) flatName, flatLength, keylist[12] );

	return (value == result0)  ||
	       (value == result1)  ||
	       (value == result2)  ||
	       (value == result3)  ||
	       (value == result4)  ||
	       (value == result5)  ||
	       (value == result6)  ||
	       (value == result7)  ||
	       (value == result8)  ||
	       (value == result9)  ||
	       (value == result10) ||
	       (value == result11) ||	       
	       (value == result12);	       

/* A future version should become:

	return ((value == result0)  && (flatName[0] == 'A' || flatName[0] == 'B')) ||
	       ((value == result1)  && (flatName[0] == 'C' || flatName[0] == 'D')) ||
	       ((value == result2)  && (flatName[0] == 'E' || flatName[0] == 'F')) ||
	       ((value == result3)  && (flatName[0] == 'G' || flatName[0] == 'H')) ||
	       ((value == result4)  && (flatName[0] == 'I' || flatName[0] == 'J')) ||
	       ((value == result5)  && (flatName[0] == 'K' || flatName[0] == 'L')) ||
	       ((value == result6)  && (flatName[0] == 'M' || flatName[0] == 'N')) ||
	       ((value == result7)  && (flatName[0] == 'O' || flatName[0] == 'P')) ||
	       ((value == result8)  && (flatName[0] == 'Q' || flatName[0] == 'R')) ||
	       ((value == result9)  && (flatName[0] == 'S' || flatName[0] == 'T')) ||
	       ((value == result10) && (flatName[0] == 'U' || flatName[0] == 'V')) ||
	       ((value == result11) && (flatName[0] == 'W' || flatName[0] == 'X')) ||
	       ((value == result12) && (flatName[0] == 'Y' || flatName[0] == 'Z'));

*/
}


