// random.c

#include "stdafx.h"
#include <stdlib.h>

#include "main.h"
#include "random.h"

unsigned int  randomSeed[2], pieceCount[2], grenadeTimer[2];
int pieceMap[kBlobTypes], numPieces;


static unsigned int  internalRandomSeed = 1;

static int internalRandom() // uses a generic rand() algorithm
{
	internalRandomSeed = internalRandomSeed * 1103515245 + 12345;
	return ((internalRandomSeed >> 16) & 0x7FFF);
}


void InitRandom( int inNumPieces )
{
	int count, swap, swapWith;
	
	numPieces = inNumPieces;
	randomSeed[0] = randomSeed[1] = SDL_GetTicks();
	pieceCount[0] = pieceCount[1] = 0;
	grenadeTimer[0] = grenadeTimer[1] = 40;
	
	for( count=0; count<kBlobTypes; count++ )
	{
		pieceMap[count] = count+1;
	}

	for( count=0; count<kBlobTypes; count++ )
	{
		swapWith = RandomBefore( kBlobTypes );
		swap = pieceMap[swapWith];
		pieceMap[swapWith] = pieceMap[count];
		pieceMap[count] = swap;
	}
}

void AddExtraPiece( void )
{
	numPieces++;
}

int GetPiece( int player )
{
	int result;
	unsigned int  realSeed;
	
	realSeed = internalRandomSeed;
	
	internalRandomSeed = randomSeed[player];
	result = pieceMap[RandomBefore(numPieces)];
	randomSeed[player] = internalRandomSeed;
	
	internalRandomSeed = realSeed;

	return result;
}

int GetMagic( int player )
{
	int result;
	int realSeed;
	
	realSeed = internalRandomSeed;
	
	internalRandomSeed = randomSeed[player];
	result = (RandomBefore(19) == 0)? true: false;
	randomSeed[player] = internalRandomSeed;
	
	internalRandomSeed = realSeed;

	return result;
}

int GetGrenade( int player )
{
	pieceCount[player]++;
	if( pieceCount[player] == grenadeTimer[player] )
	{
		grenadeTimer[player] += grenadeTimer[player] * 3 / 2;
		return true;
	}
	else
	{
		return false;
	}
}

static inline int RandomRange( double min, double max )
{
	const double kMinRand = 0.0;
	const double kMaxRand = 32767.0;
	double x;
	
	x = (internalRandom() - kMinRand) / (kMaxRand - kMinRand + 1.0);
	return (int) (x * (max + 1.0 - min) + min);
}

int RandomBefore( int what )
{	
	return RandomRange( 0.0, what-1 );
}

