// tweak.c

#include "stdafx.h"
#include <math.h>

#include "main.h"
#include "tweak.h"
#include "gworld.h"
#include "moving.h"
#include "gameticks.h"
#include "graphics.h"
#include "players.h"

int  xTweakTime[2], yTweakTime[2], rTweakTime[2];
int yTweak[2], xTweak[2], xDirection[2], rTweak[2], rDirection[2];
int lastShadow[2];
int tweakOffsetX[4][11], tweakOffsetY[4][11];

void InitTweak( void )
{
	int rTweakValues[] = { 0, 5, 10, 30, 50, 70, 90, 110, 130, 150, 170 };
	int count, rotate;
	double tweakRad;
	
	for( rotate = 0; rotate<2; rotate++ )
	{
		for( count=0; count<=10; count++ )
		{
			tweakRad = d2r( (90*rotate) - rTweakValues[count] );
			tweakOffsetX[rotate][count] = (int) floor( 0.5 + cos( tweakRad ) * kBlobHorizSize );
			tweakOffsetY[rotate][count] = (int) floor( 0.5 + sin( tweakRad ) * kBlobVertSize  );

			tweakOffsetX[rotate+2][count] = -tweakOffsetX[rotate][count];
			tweakOffsetY[rotate+2][count] = -tweakOffsetY[rotate][count];
		}
	}
}

void TweakFirstBlob( int player, MRect *first )
{
	int tweakValues[] = {0, -1, -2, -3, -6, -12};
	
	if( xTweak[player] > 0 )
	{
		OffsetMRect( first, xDirection[player] * tweakValues[xTweak[player]], 0 );
	}

	if( yTweak[player] > 0 )
	{
		OffsetMRect( first, 0, tweakValues[yTweak[player]] );
	}
}

void TweakSecondBlob( int player, MRect *second )
{
	int x, y;
	
	CalcSecondBlobOffset( player, &x, &y );
	OffsetMRect( second,
				 tweakOffsetX[blobR[player]][rTweak[player]],
				 tweakOffsetY[blobR[player]][rTweak[player]] );
}

void StartTweak( int player, int direction, int rotate, int fall )
{
	if( fall != 0 )
	{
		yTweak[player] = 3;
		yTweakTime[player] = GameTickCount() + kTweakDelay;
	}

	if( direction != 0 )
	{
		xDirection[player] = direction;
		xTweak[player] = 5;
		xTweakTime[player] = GameTickCount() + kTweakDelay;
	}
	
	if( rotate != 0 )
	{
		rTweak[player] = rotate * 5; 
		rDirection[player] = rotate;
		rTweakTime[player] = GameTickCount() + kTweakDelay;
	}
}

void UpdateTweak( int player, int animation )
{
	MBoolean isXTweaked, isYTweaked, isRTweaked, isAnimTweaked = false;
	
	if( GameTickCount( ) >= animTime[player] )
	{
		isAnimTweaked = true;
		animTime[player] += 2;
		anim[player]++;	
			
		HandleMagic( player );
	}

	isXTweaked = ( (GameTickCount() >= xTweakTime[player]) && (xTweak[player] > 0) );
	isYTweaked = ( (GameTickCount() >= yTweakTime[player]) && (yTweak[player] > 0) );
	isRTweaked = ( (GameTickCount() >= rTweakTime[player]) && (rTweak[player] > 0) );
	
	if( isXTweaked || isRTweaked || isYTweaked || 
	    isAnimTweaked || (shadowDepth[player] != lastShadow[player]) )
	{	
		EraseSpriteBlobs( player );
		
		if( isXTweaked )
		{
			xTweak[player]--;
			xTweakTime[player] += kTweakDelay;
		}
		
		if( isYTweaked )
		{
			yTweak[player]--;
			yTweakTime[player] += kTweakDelay;
		}

		if( isRTweaked )
		{
			rTweak[player]--;
			rTweakTime[player] += kTweakDelay;		
		}
		
		DrawSpriteBlobs( player, animation );
	}
}
