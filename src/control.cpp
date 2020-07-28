// control.c

#include "stdafx.h"
#include "main.h"
#include "control.h"
#include "moving.h"
#include "players.h"
#include "random.h"
#include "grays.h"
#include "zap.h"
#include "gameticks.h"
#include "level.h"
#include "tutorial.h"
#include <stdlib.h>

int destinationX[2], destinationR[2];
signed char tempGrid[kGridAcross][kGridDown];
int timeAI[2], timeMove[2];
MBoolean moveQuick[2];
AutoPatternPtr autoPattern = NULL;

void AutoControl( int player )
{
	if( autoPattern ) 
	{				
		switch( autoPattern->command )
		{
			case kMessage:
				StartBalloon( autoPattern->message );
				autoPattern++;
				break;
				
			case kIdleTicks:
				if( !tutorialTime )
				{
					tutorialTime = GameTickCount() + autoPattern->d1;
				}
				else
				{
					if( GameTickCount() >= tutorialTime )
					{
						tutorialTime = 0;
						autoPattern++;
					}
				}
				break;
			
			case kRetrieve:
				if( role[player] == kWaitForRetrieval )
				{
					nextA[player] = abs( autoPattern->d1 );
					nextB[player] = abs( autoPattern->d2 );
					nextM[player] = (autoPattern->d1 < 0);
					nextG[player] = (autoPattern->d1 == kBombBottom) && (autoPattern->d2 == kBombTop);
					
					if( !nextG[player] )
					{
						nextA[player] = pieceMap[ nextA[player] ];
						nextB[player] = pieceMap[ nextB[player] ];
					}
										
					role[player]  = kRetrieveBlobs;
					autoPattern++;
				}
				break;
			
			case kPosition:
				if( (role[player] != kFalling) || (blobX[player] == autoPattern->d1) )
				{
					autoPattern++;
				}
				else if( GameTickCount() >= timeMove[player] )
				{
					timeMove[player] = GameTickCount() + 12;
					
					if( blobX[player] > autoPattern->d1 )
					{
						if( CanGoLeft( player ) )
							GoLeft( player );
					}
					else
					{
						if( CanGoRight( player ) )
							GoRight( player );
					}
				}
				break;
			
			case kSpin:
				if( role[player] != kFalling )
				{
					autoPattern++;
				}
				else if( CanRotate( player ) )
				{
					DoRotate( player );
					autoPattern++;
				}
				break;

			case kBlockUntilLand:
				if( role[player] != kFalling ) 
				{
					autoPattern++;
				}			
				break;
			
			case kBlockUntilDrop:
				if( !dropping[player] ) DoDrop( player );

				if( role[player] != kFalling ) 
				{
					autoPattern++;
				}			
				break;

			case kPunish:
				if( role[player] == kWaitForRetrieval )
				{
					lockGrays[player] = autoPattern->d1;
					SetupGrays( player );
					blobTime[player] = GameTickCount( );
					role[player] = kDropGrays;

					autoPattern++;
				}
				break;
		
			case kComplete:
				EndTutorial( );
				break;
			
			case kBlockUntilComplete:
				if( role[player] == kWaitForRetrieval ) 
				{
					autoPattern++;
				}			
				break;
		}
	}
}

void PlayerControl( int player )
{
	int a = player, b = player;
	MBoolean moved = false;
	
	if( players == 1 )
	{
		a = 0;
		b = 1;
	}

	if( hitKey[a].left || hitKey[b].left )
	{
		if( GameTickCount() >= timeMove[player] )
		{
			timeMove[player] += 12;
			if( CanGoLeft( player ) )
				GoLeft( player );
		}
		
		moved = true;
	}
	
	if( hitKey[a].right || hitKey[b].right )
	{	
		if( GameTickCount() >= timeMove[player] )
		{
			timeMove[player] += 12;
			if( CanGoRight( player ) )
				GoRight( player );
		}
		
		moved = true;
	}
	
	if( !moved ) timeMove[player] = GameTickCount( );
	
	if( hitKey[a].rotate == 1 || hitKey[b].rotate == 1 )
	{
		if( CanRotate( player ) )
			DoRotate( player );
	}
	
	if( hitKey[a].drop == 1 || hitKey[b].drop == 1 )
	{
		DoDrop( player );
	}
	
	if( hitKey[a].drop == 0 && hitKey[b].drop == 0 )
	{
		StopDrop( player );
	}
}

void AIControl( int player )
{
	if( timeAI[player] > GameTickCount() )
		return;
	
	timeAI[player] += moveQuick[player]? character[player].speedRush:
									     character[player].speedNormal;
	
	switch( RandomBefore( 2 ) )
	{
		case 0:
			if( destinationR[player] != blobR[player] )
			{
				if( CanRotate(player) )
				{
					DoRotate( player );
				}
			}
			break;
		
		case 1:
			if( destinationX[player] != blobX[player] ) 
			{
				if( destinationX[player] > blobX[player] )
				{
					if( CanGoRight( player ) )
						GoRight( player );
				}
				else
				{
					if( CanGoLeft( player ) )
						GoLeft( player );
				}
			}
			break;		
	}
	
	if( destinationX[player] == blobX[player] &&
		destinationR[player] == blobR[player] &&
		RandomBefore( 100 ) < character[player].intellect )
	{
		DoDrop( player );
	}
}

void ChooseAIDestination( int player )
{
	int testX, testR, testX2, testR2, value, bestValue = -9999999;
	int x, y;
	int bestX[kGridAcross*4], bestR[kGridAcross*4], currentBest = -1;
	int rowDifference, totalTries, temp;
	MBoolean shouldTry[kGridAcross][4];
	
	timeAI[player] = GameTickCount( ) + 1;
	moveQuick[player] = true;
	
	if( grenade[player] )
	{
		bestValue = 0;
		currentBest = 2;
		
		for( testX = 0; testX < kGridAcross; testX++ )
		{
			rowDifference = GetRowHeight( player, testX );
			
			if( (rowDifference < kGridDown - 1) &&
					(grid[player][testX][rowDifference+1] >= kFirstBlob) &&
					(grid[player][testX][rowDifference+1] <= kLastBlob)     )
			{
				value = 0;

				for( x=0; x<kGridAcross; x++ )
				{
					for( y=0; y<kGridDown; y++ )
					{
						if( grid[player][x][y] == grid[player][testX][rowDifference+1] ) value++;
					}
				}
			
				if( value > bestValue )
				{
					bestValue = value;
					currentBest = testX;
				}
			}
		}
		
		destinationR[player] = upRotate;
		destinationX[player] = currentBest;
		return;
	}
	
	if( (GameTickCount() - startTime) <= 3600 )
	{
		for( testX = 0; testX < kGridAcross; testX++ )
		{
			rowDifference =  GetRowHeight( player, testX ) - character[player].autoSetup[testX];
			
			if( rowDifference >= 2 )
			{
				destinationR[player] = downRotate;
				destinationX[player] = testX;
				return;
			}
			
			if( rowDifference == 1 )
			{
				destinationX[player] = testX;
				
				if( testX > 0 )
				{
					if( GetRowHeight( player, testX-1 ) > character[player].autoSetup[testX-1] )
					{
						destinationR[player] = leftRotate;
						return;
					}
				}
				
				if( testX < (kGridAcross-1) )
				{
					if( GetRowHeight( player, testX+1 ) > character[player].autoSetup[testX+1] )
					{
						destinationR[player] = rightRotate;
						return;
					}
				}
				
				destinationR[player] = upRotate;
				return;
			}
		}
	}
	
	moveQuick[player] = (emotions[player] == kEmotionSad) || (emotions[player] == kEmotionPanic);
	
	totalTries = character[player].intellect;
	for( testX = 0; testX < kGridAcross; testX++ )
	{
		for( testR = 0; testR < 4; testR++ )
		{
			shouldTry[testX][testR] = --totalTries >= 0;
		}
	}
	
	for( testX = 0; testX < kGridAcross; testX++ )
	{
		for( testR = 0; testR < 4; testR++ )
		{
			testX2 = RandomBefore( kGridAcross );
			testR2 = RandomBefore( 4 );
		
			temp = shouldTry[testX][testR];
			shouldTry[testX][testR] = shouldTry[testX2][testR2];
			shouldTry[testX2][testR2] = temp;
		}
	}
	
	shouldTry[0][leftRotate]			  = false;
	shouldTry[kGridAcross-1][rightRotate] = false;
		
	for( testX = 0; testX < kGridAcross; testX++ )
	{
		for( testR = 0; testR<=3; testR++ )
		{
			if( shouldTry[testX][testR] )
			{
				value = TestAIDestination( player, testX, testR );
				
				if( value > bestValue )
				{
					bestValue = value;
					currentBest = -1;
				}
				
				if( value == bestValue )
				{
					currentBest++;
					bestX[currentBest] = testX;
					bestR[currentBest] = testR;
				}
			}
		}
	}
	
	currentBest = RandomBefore( currentBest + 1 );
	destinationX[player] = bestX[currentBest];
	destinationR[player] = bestR[currentBest];
}

int TestAIDestination( int player, int testX, int testR )
{
	int x, y, height, chains, rensa = 50, result = 0;

	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			tempGrid[x][y] = grid[player][x][y];
		}
	}
		
	height = GetRowHeight(player, testX);
	switch( testR ) 
	{
		case upRotate:
			tempGrid[testX][height--] = colorA[player];
			if( height >= 0 ) tempGrid[testX][height]   = colorB[player];
			break;
		
		case downRotate:
			tempGrid[testX][height--] = colorB[player];
			if( height >= 0 ) tempGrid[testX][height]   = colorA[player];
			break;
		
		case leftRotate:
			tempGrid[testX][height]                         = colorA[player];
			tempGrid[testX-1][GetRowHeight(player,testX-1)] = colorB[player];
			break;
		
		case rightRotate:
			tempGrid[testX][height]                         = colorA[player];
			tempGrid[testX+1][GetRowHeight(player,testX+1)] = colorB[player];
			break;		
	}
	
	chains = TestTemporaryGrid( );
	
	result = ScoreTemporaryGrid( );
		
	if( (chains < 2) && (character[player].intellect > (24 * 2/3)) )
	{
		rensa = 0;
	}
	else
	{
		while( chains-- ) rensa *= 10;
	}
	
	result += rensa;
	
	return result;
}

int ScoreTemporaryGrid( void )
{
	int x, y, change, result = 0;
	int deductions[kGridAcross][kGridDown] = 
		{ { 400, 350, 350, 200, 120, 60, 20, 5, 0, 0, 0, 0 },
		  { 600, 500, 300, 150, 100, 50, 20, 0, 0, 0, 0, 0 },
		  { 9999, 800, 200, 100,  50, 40, 20, 0, 0, 0, 0, 0 },
		  { 9999, 800, 200, 100,  50, 40, 20, 0, 0, 0, 0, 0 },
		  { 9999, 500, 300, 150, 100, 50, 20, 0, 0, 0, 0, 0 },
		  { 400, 350, 350, 200, 120, 60, 20, 5, 0, 0, 0, 0 } };

	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( tempGrid[x][y] == kEmpty )
			{
				result += deductions[x][y];
			}
			else if( tempGrid[x][y] == kGray )
			{
				result -= deductions[x][y];
			}
			else
			{
				change = 0;

				if( y < (kGridDown-1)   && (tempGrid[x][y] == tempGrid[x][y+1])   ) change += 50;
				if( y > 0               && (tempGrid[x][y] == tempGrid[x][y-1])   ) change += 50;
				if( x < (kGridAcross-1) && (tempGrid[x][y] == tempGrid[x+1][y])   ) change += 40;
				if( x > 0               && (tempGrid[x][y] == tempGrid[x-1][y])   ) change += 40;
				if( x > 0               &&  
				    y > 0               && (tempGrid[x][y] == tempGrid[x-1][y-1]) ) change += 20;
				if( x < (kGridAcross-1) &&  
				    y > 0               && (tempGrid[x][y] == tempGrid[x+1][y-1]) ) change += 20;
				if( x > 0               &&  
				    y < (kGridDown-1)   && (tempGrid[x][y] == tempGrid[x-1][y+1]) ) change += 10;
				if( x < (kGridAcross-1) &&  
				    y < (kGridDown-1)   && (tempGrid[x][y] == tempGrid[x+1][y+1]) ) change += 10;
				if( y < (kGridDown-2)   && (tempGrid[x][y] == tempGrid[x][y+2])   ) change += 10;
				if( y > 1               && (tempGrid[x][y] == tempGrid[x][y-2])   ) change += 10;
				
				if( (x > 0               && tempGrid[x-1][y] == kEmpty) ||
				    (x < (kGridAcross-1) && tempGrid[x+1][y] == kEmpty) ||
				    (y < 4               || tempGrid[x][y-4] == kEmpty)    ) change *= 4;
				    
				result += change / 4;
			}
		}
	}
	
	return result;
}

int TestTemporaryGrid( void )
{
	MBoolean busy;
	int x, y, stackSize, chains = 0;
	
	do
	{
		busy = false;
		
		for( x=0; x<kGridAcross; x++ )
		{
			for( y=0; y<kGridDown; y++ )
			{
				if( tempGrid[x][y] >= kFirstBlob && tempGrid[x][y] <= kLastBlob )
				{
					if( SizeUp( tempGrid, x, y, tempGrid[x][y] ) >= kBlobClusterSize )
					{
						QuickRemove( tempGrid, x, y, tempGrid[x][y] );
						busy = true;
					}
				}
			}
		}
		
		if( busy )
		{
			chains++;
			
			for( x=0; x<kGridAcross; x++ )
			{
				stackSize = kGridDown-1;
				
				for( y=kGridDown-1; y>=0; y-- )
				{
					if( tempGrid[x][y] != kEmpty )
					{
						tempGrid[x][stackSize] = tempGrid[x][y];
						if( y < stackSize ) tempGrid[x][y] = kEmpty;
						stackSize--;
					}
				}
			}
		}
	}
	while( busy );
	
	return chains;
}

void QuickRemove( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) )
		return;
	
	if( myGrid[x][y] == kGray ) myGrid[x][y] = kEmpty;	
	if( myGrid[x][y] == color )
	{
		myGrid[x][y] = kEmpty;
		QuickRemove( myGrid, x-1, y,   color );
		QuickRemove( myGrid, x+1, y,   color );
		QuickRemove( myGrid, x,   y-1, color );
		QuickRemove( myGrid, x,   y+1, color );
	}
}

int BestColor( int player, int blobX, int blobY )
{
	int x, y, color, bestColor = kFirstBlob, bestResult = -9999999, rensa, chains, result;
//	char C[] = {' ', '*', '@', '.', '=', '+', 'o', 'J'};	


	for( color = kFirstBlob; color <= kLastBlob; color++ )
	{		
		for( y=0; y<kGridDown; y++ )
		{
			for( x=0; x<kGridAcross; x++ )
			{
				tempGrid[x][y] = grid[player][x][y];
			}
		}
	
		tempGrid[blobX][blobY] = color;
		
		chains = TestTemporaryGrid( );
		result = ScoreTemporaryGrid( );

		rensa = 1000;
		while( chains-- ) rensa *= 10;
		
		result += rensa;
		
		if( result > bestResult )
		{
			bestColor = color;
			bestResult = result;
		}
	}
		
	return bestColor;
}

// Returns the first empty row.
int GetRowHeight( int player, int row )
{
	int height;
	
	for( height = (kGridDown-1); height > 0; height-- )
	{
		if( grid[player][row][height] == kEmpty ) break;
	}
	
	return height;
}

int DetermineEmotion( int player )
{
	int us = 0, them = 0, aboveWater = 1;
	int x,y;
	
	if( role[player] == kLosing )  return kEmotionPanic;
	if( role[player] == kWinning ) return kEmotionHappy;
	
	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( aboveWater && (y<3) && (grid[player][x][y] != kEmpty) ) aboveWater = 0;
			if( grid[player][x][y]   != kEmpty ) us++;
			if( grid[1-player][x][y] != kEmpty ) them++;
		}
	}
	
	if( us > 48 && !aboveWater ) return kEmotionPanic;
	else if( abs(us-them) < 12 ) return kEmotionNeutral;
	else if( us > them ) return kEmotionSad;
	else return kEmotionHappy;
}
