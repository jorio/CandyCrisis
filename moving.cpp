// moving.c

#include "stdafx.h"
#include "main.h"
#include "moving.h"
#include "players.h"
#include "graphics.h"
#include "soundfx.h"
#include "tweak.h"
#include "gameticks.h"
#include "level.h"

void CalcSecondBlobOffset( int player, int *x, int *y )
{
	*x = *y = 0;
	
	switch( blobR[player] )
	{
		case rightRotate:
			*x = 1;
			break;
		
		case downRotate:
			*y = 1;
			break;
			
		case leftRotate:
			*x = -1;
			break;
		
		case upRotate:
			*y = -1;
			break;
	}
}

MBoolean CanGoLeft( int player )
{
	return CanMoveDirection( player, -1, halfway[player]? 1: 0 );
}

void GoLeft( int player )
{
	EraseSpriteBlobs( player );
	blobX[player]--;
	StartTweak( player, -1, 0, 0 );
	DrawSpriteBlobs( player, kNoSuction );
	
	PlayStereo( player, kShift );
}

MBoolean CanGoRight( int player )
{
	return CanMoveDirection( player, 1, halfway[player]? 1: 0 );
}

void GoRight( int player )
{
	EraseSpriteBlobs( player );
	blobX[player]++;
	StartTweak( player, 1, 0, 0 );
	DrawSpriteBlobs( player, kNoSuction );
	
	PlayStereo( player, kShift );
}

MBoolean CanFall( int player )
{
	return CanMoveDirection( player, 0, 1 );
}

MBoolean CanMoveDirection( int player, int dirX, int dirY )
{
	int currentX = blobX[player], currentY = blobY[player], x, y;
	
	currentX += dirX;
	currentY += dirY;
	
	if( currentX < 0 || currentX >= kGridAcross || currentY >= kGridDown )
		return false;
	
	if( currentY >= 0 )
		if( grid[player][currentX][currentY] != kEmpty )
			return false;

	CalcSecondBlobOffset( player, &x, &y );
	
	currentX += x;
	currentY += y;
	
	if( currentX < 0 || currentX >= kGridAcross || currentY >= kGridDown )
		return false;
	
	if( currentY >= 0 )
		if( grid[player][currentX][currentY] != kEmpty )
			return false;

	return true;
}

void DoFall( int player )
{
	EraseSpriteBlobs( player );
	
	if( halfway[player] )
		blobY[player]++;
	halfway[player] = !halfway[player];
	
	StartTweak( player, 0, 0, 1 );

	DrawSpriteBlobs( player, kNoSuction );
}

MBoolean CanRotate( int player )
{
	if( role[player] == kChooseDifficulty ) return false;
	
	if( grenade[player] ) return false;
	
	return true;
	
}

void DoRotate( int player )
{
	MBoolean possible;
	
	EraseSpriteBlobs( player );
	
	blobR[player] = ( blobR[player] + 1 ) % 4;
	possible = CanMoveDirection( player, 0, halfway[player]? 1: 0 );
	StartTweak( player, 0, 1, 0 ); // only rotates clockwise
	
	if( !possible )
	{
		if( blobR[player] == downRotate )
		{
			if( halfway[player] )
				halfway[player] = false;
			else
				blobY[player]--;
				
			if( ++blobSpin[player] >= 4 )
			{
				blobTime[player] = animTime[player] = GameTickCount( );
				role[player] = kLockdownBlobs;
				anim[player] = 0;
				PlayStereoFrequency( player, kPlace, player );
			}
		}
		
		if( blobR[player] == leftRotate )
		{
			if( CanGoRight(player) )
				GoRight( player );
			else
			{
				blobR[player]++;
				StartTweak( player, 0, 2, 0 );
			}
		}
		
		if( blobR[player] == rightRotate  )
		{
			if( CanGoLeft(player) )
				GoLeft( player );
			else
			{
				blobR[player]++;
				StartTweak( player, 0, 2, 0 );
				
				if( !CanMoveDirection( player, 0, halfway[player]? 1: 0 ) )
				{
					if( halfway[player] )
						halfway[player] = false;
					else
						blobY[player]--;

					if( ++blobSpin[player] >= 4 )
					{
						blobTime[player] = animTime[player] = GameTickCount( );
						role[player] = kLockdownBlobs;
						anim[player] = 0;
						PlayStereoFrequency( player, kPlace, player );
					}
				}
			}
		}
	}
	
	DrawSpriteBlobs( player, kNoSuction );
	
	PlayStereo( player, kRotate );
}

void DoDrop( int player )
{
	dropping[player] = true;
	
	if( role[player] != kJiggleBlobs &&
		role[player] != kFastJiggleBlobs &&
		role[player] != kLockdownBlobs      )
		blobTime[player] = GameTickCount( );
}

void StopDrop( int player )
{
	dropping[player] = false;
}
