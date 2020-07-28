// zap.cpp

#include "stdafx.h"
#include "SDLU.h"

#include <stdio.h>
#include <algorithm>

#include "main.h"
#include "players.h"
#include "zap.h"
#include "grays.h"
#include "soundfx.h"
#include "gworld.h"
#include "graphics.h"
#include "gameticks.h"
#include "level.h"
#include "random.h"
#include "tweak.h"
#include "blitter.h"
#include "font.h"
#include "score.h"
#include "hiscore.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

signed char death[2][kGridAcross][kGridDown];
int zapIteration[2];
int grenadeFrame[2] = {kBlastFrames + 1, kBlastFrames + 1}, zapScoreFrame[2];
MPoint zapScorePt[2];
MRect grenadeRect[2];
SkittlesFontPtr zapFont, zapOutline;
char zapScore[2][20] = { "", "" };
int zapScoreWidth[2];
int zapScoreR[2], zapScoreG[2], zapScoreB[2];
int zapOffsetX[7][kZapFrames], zapOffsetY[7][kZapFrames];

using std::min;

void ZapScoreDisplay( int player, int amount, int multiplier, int x, int y, int c )
{
	char *scan;

	if( amount     > 0 && 
	    multiplier > 0 && 
	    x >= 0 && x < kGridAcross &&
	    y >= 0 && y < kGridDown   &&
	    c >= kFirstBlob && c <= (kLastBlob+1) )
	{
		zapScorePt[player].v = y * kBlobVertSize  + 6;
		zapScorePt[player].h = x * kBlobHorizSize + 6;

		zapScoreR[player] = glowColors[c][0];
		zapScoreG[player] = glowColors[c][1];
		zapScoreB[player] = glowColors[c][2];

	    sprintf( zapScore[player], (multiplier == 1)? "%d": "%d*%d", amount, multiplier );

		zapScoreWidth[player] = 0;
		scan = zapScore[player];
		while( *scan ) zapScoreWidth[player] += zapFont->width[*scan++];
		
		if( (zapScorePt[player].h + zapScoreWidth[player] + 8) > (kGridAcross * kBlobHorizSize) )
		{
			zapScorePt[player].h = (kGridAcross * kBlobHorizSize) - zapScoreWidth[player] - 8;
		}
	}
}

void ZapBlobs( int player )
{
	int x, y, cluster, clusterCount = 0, multiplier, amount = 0;
	int zapFocusX = -1, zapFocusY = -1, zapFocusC = 0;
	
	
	zapScorePt[player].v = 0;
	zapScoreFrame[player] = 0;
	
	switch( chain[player] )
	{
		case 1:  multiplier = 1;                  break;
		default: multiplier = 2 << chain[player]; break;
	}

	for( y=kGridDown-1; y>=0; y-- )
	{
		for( x=kGridAcross-1; x>=0; x-- )
		{
			if( grid[player][x][y] >= kFirstBlob &&
				grid[player][x][y] <= kLastBlob &&
				suction[player][x][y] != kInDeath )
			{
				cluster = SizeUp( grid[player], x, y, grid[player][x][y] );
				if( cluster >= kBlobClusterSize )
				{
					clusterCount++;
					zapFocusX = x;
					zapFocusY = y;
					zapFocusC = grid[player][x][y];
					
					amount += cluster * 10;
					
					multiplier += cluster - kBlobClusterSize;
					
					RemoveBlobs( player, x, y, grid[player][x][y], 0 );
				}
			}
		}
	}

	if( clusterCount > 0 )
	{
		switch( clusterCount )
		{
			case 1:                     break;
			case 2:   multiplier += 3;  break;
			case 3:   multiplier += 6;  break;
			case 4:   multiplier += 12; break;
			default:  multiplier += 24; break;
		}

		if( multiplier > 999 ) multiplier = 999;
		CalculateGrays( 1-player, amount * multiplier / difficulty[player] );
		potentialCombo[player].value += amount * multiplier;
		
		if( players == 1 ) amount *= ((level <= kLevels)? level: 1);
		score[player] += amount * multiplier;
		
		ZapScoreDisplay( player, amount, multiplier, zapFocusX, zapFocusY, zapFocusC );
	}
	
	blobTime[player] = GameTickCount( );
	
	if( clusterCount > 0 )
	{
		chain[player]++;
		role[player] = kKillBlobs;
		PlayStereoFrequency( player, kSquishy, zapIteration[player]++ );
	}
	else
	{
		if( control[player] == kPlayerControl )
		{
			SubmitCombo( &potentialCombo[player] );
		}
		
		SetupGrays( player );
		role[player] = kDropGrays;
		
		if( BusyDroppingGrays( player ) )
		{
			PlayStereoFrequency( player, kWhistle, player );
		}
	}
}

void RemoveBlobs( int player, int x, int y, int color, int generation )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) )
		return;
	
	if( grid[player][x][y] == kGray )
	{
		suction[player][x][y] = kGrayBlink1;
		death[player][x][y] = -8 - generation;
		return;
	}
	
	if( grid[player][x][y] != color || suction[player][x][y] == kInDeath )
		return;
	
	suction[player][x][y] = kInDeath;
	death[player][x][y] = -12 - generation;
	
	RemoveBlobs( player, x-1, y,   color, generation+3 );
	RemoveBlobs( player, x+1, y,   color, generation+3 );
	RemoveBlobs( player, x,   y-1, color, generation+3 );
	RemoveBlobs( player, x,   y+1, color, generation+3 );
}

void KillBlobs( int player )
{
	int x,y;
	const int   position[] = { 0, 15, 27, 39, 51, 63, 72, 81, 90, 99, 105,111,117,123,126,129,131,132,133,134,135,135,136,136,137,137,138,138,138,139,139,139,139,140,140,140 };
	const int   shading [] =
    {
        _5TO8(20),
        _5TO8(21),
        _5TO8(22),
        _5TO8(23),
        _5TO8(24),
        _5TO8(25),
        _5TO8(26),
        _5TO8(27),
        _5TO8(28),
        _5TO8(29),
        _5TO8(30),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(30),
        _5TO8(29),
        _5TO8(28),
        _5TO8(26),
        _5TO8(24),
        _5TO8(21),
        _5TO8(18),
        _5TO8(15),
        _5TO8(12),
        _5TO8(9),
        _5TO8(6),
        _5TO8(3),
        _5TO8(0)
    };
	const int   blobGraphic[kZapFrames] = { kDying,   kDying,   kDying,   kDying,   kSquish1,
								            kSquish1, kSquish1, kSquish1, kSquish2, kSquish2, 
			 					            kSquish2, kSquish2, kSquish3, kSquish3, kSquish3,
								            kSquish3, kSquish4, kSquish4, kSquish4, kSquish4 },
		  		grayGraphic[kZapFrames] = { kGrayBlink1, kGrayBlink1, kGrayBlink1,
		  						            kGrayBlink1, kGrayBlink1, kGrayBlink1, 
		  						            kGrayBlink2, kGrayBlink2, kGrayBlink2,
		  						            kGrayBlink2, kGrayBlink2, kGrayBlink2, 
		  						            kGrayBlink3, kGrayBlink3, kGrayBlink3,
		  					            	kGrayBlink3, kGrayBlink3, kGrayBlink3, 
		  						            kGrayBlink3, kGrayBlink3 };
	MRect myRect;
	MBoolean busy = false;
	MPoint dPoint, oPoint;
	char *scan;
	
	if( blobTime[player] > GameTickCount( ) )
		return;
	
	blobTime[player]++;

	SDLU_AcquireSurface( playerSurface[player] );
	
	// clear grenade sprite
	if( grenadeFrame[player] <= kBlastFrames )
	{
		CleanSpriteArea( player, &grenadeRect[player] );
		if( grenadeFrame[player] == kBlastFrames ) grenadeFrame[player]++;
	}
		
	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( grid[player][x][y] >= kFirstBlob &&  // if a blob is dying
				grid[player][x][y] <= kLastBlob &&
				suction[player][x][y] == kInDeath )
			{
				death[player][x][y]++;
				busy = true;
				
				CalcBlobRect( x, y, &myRect );
				
				if( death[player][x][y] >= 0 && death[player][x][y] <= kZapFrames ) // draw its death
				{					
					if( death[player][x][y] == kZapFrames )
					{
						grid[player][x][y] = kEmpty;
						suction[player][x][y] = kNoSuction;
						charred[player][x][y] = kNoCharring;
						SurfaceDrawBlob( player, &myRect, kEmpty, kNoSuction, kNoCharring );
						CleanSpriteArea( player, &myRect );
					}
					else
					{
						SurfaceDrawBlob( player, &myRect,
								         grid[player][x][y],
								         blobGraphic[ death[player][x][y] ],
								         kNoCharring );
						CleanSpriteArea( player, &myRect );
					}
					
					CleanChunks( player, x, y, death[player][x][y], character[player].zapStyle );
				}
				else
				{
					SurfaceDrawBlob( player, &myRect, grid[player][x][y],
								(blobTime[player] & 2)? kFlashDarkBlob: kNoSuction, kNoCharring );
					CleanSpriteArea( player, &myRect );
				}
			}
			else
			{
				if( grid[player][x][y] == kGray &&					// gray dying
					suction[player][x][y] == kGrayBlink1 )
				{
					CalcBlobRect( x, y, &myRect );
					
					if( death[player][x][y] >= 0 && death[player][x][y] <= kZapFrames )
					{
						if( death[player][x][y] == kZapFrames )
						{
							grid[player][x][y] = kEmpty;
							suction[player][x][y] = kNoSuction;
							SurfaceDrawBlob( player, &myRect, kEmpty, kNoSuction, kNoCharring );
						}
						else
						{
							SurfaceDrawBoard( player, &myRect );
							SurfaceDrawAlpha( &myRect, kGray, kLight, grayGraphic[ death[player][x][y] ] );
							busy = true;
						}
						CleanSpriteArea( player, &myRect );
					}
					
					death[player][x][y]++;
				}
			}
		}
	}
	
	// draw score info above blobs but below chunks and explosions
	
	if( zapScoreFrame[player] < arrsize(position) )
	{
		myRect.top    = zapScorePt[player].v - (position[zapScoreFrame[player]    ]);
		myRect.left   = zapScorePt[player].h;
		myRect.bottom = zapScorePt[player].v - (position[zapScoreFrame[player] - 1]) + 15;
		myRect.right  = myRect.left + zapScoreWidth[player];
		CleanSpriteArea( player, &myRect );

		if( zapScoreFrame[player] < arrsize(position)-1 )
		{		
			SDLU_AcquireSurface( playerSpriteSurface[player] );	
			
			dPoint.v = oPoint.v = myRect.top;
			dPoint.h = oPoint.h = myRect.left;
			scan = zapScore[player];
			while( *scan )
			{
				SurfaceBlitWeightedCharacter( zapFont,    *scan, &dPoint, zapScoreR[player], zapScoreG[player], zapScoreB[player], shading[zapScoreFrame[player]] );
				SurfaceBlitWeightedCharacter( zapOutline, *scan, &oPoint, 0,                 0,                 0,                 shading[zapScoreFrame[player]] );
				scan++;
			}
			
			SDLU_ReleaseSurface( playerSpriteSurface[player] );	

			zapScoreFrame[player]++;
			busy = true;
		}	
	}
		
	///////////////////////////////////////////////////////////////

	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( grid[player][x][y] >= kFirstBlob &&  // if a blob is dying
				grid[player][x][y] <= kLastBlob &&
				suction[player][x][y] == kInDeath &&
				death[player][x][y] >= 0 && death[player][x][y] < kZapFrames ) // draw chunks (after all that stuff)
			{
				DrawChunks( player, x, y, death[player][x][y], character[player].zapStyle );
			}
		}
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
	
	if( grenadeFrame[player] < kBlastFrames )
	{
		busy = true;
		
		SDLU_AcquireSurface( playerSpriteSurface[player] );
		
		myRect.top = grenadeFrame[player] * kBlastHeight;
		myRect.left = 0;
		myRect.bottom = myRect.top + kBlastHeight;
		myRect.right = kBlastWidth;
		
		SurfaceBlitAlpha(  playerSpriteSurface[player],  blastSurface,  blastMaskSurface,  playerSpriteSurface[player],
						  &grenadeRect[player],         &myRect,       &myRect,           &grenadeRect[player]          );

		grenadeFrame[player]++;

		SDLU_ReleaseSurface( playerSpriteSurface[player] );
	}
	
	if( !busy && role[player] == kKillBlobs )
	{
		blobTime[player] = GameTickCount( );
		halfway[player] = false;
		role[player] = kDropBlobs;
	}
}

int SizeUp( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{	
	int total;
	
	total = GetChainSize( myGrid, x, y, color );
	CleanSize( myGrid, x, y, color );
	
	return total;
}

int GetChainSize( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{
	int total;
	
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) ) return 0;
	if( myGrid[x][y] != color ) return 0;

	myGrid[x][y] = -color;
	
	total = 1 + GetChainSize( myGrid, x-1, y, color )
			  + GetChainSize( myGrid, x+1, y, color )
			  + GetChainSize( myGrid, x, y-1, color )
			  + GetChainSize( myGrid, x, y+1, color );
	
	return total;
}

void CleanWithPolish( signed char myGrid[kGridAcross][kGridDown], signed char polish[kGridAcross][kGridDown], int x, int y, int color )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) ) return;
	
	if( myGrid[x][y] == -color )
	{
		myGrid[x][y] = color;
		polish[x][y] = true;
		
		CleanWithPolish( myGrid, polish, x-1, y, color );
		CleanWithPolish( myGrid, polish, x+1, y, color );
		CleanWithPolish( myGrid, polish, x, y-1, color );
		CleanWithPolish( myGrid, polish, x, y+1, color );
	}
}

void CleanSize( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) ) return;
	
	if( myGrid[x][y] == -color )
	{
		myGrid[x][y] = color;
		
		CleanSize( myGrid, x-1, y, color );
		CleanSize( myGrid, x+1, y, color );
		CleanSize( myGrid, x, y-1, color );
		CleanSize( myGrid, x, y+1, color );
	}
}

void CleanChunks( int player, int x, int y, int level, int style )
{
	int count, color, type;
	MRect chunkRect;
	
    SDLU_AcquireSurface( playerSpriteSurface[player] );
		
    for( count=-3; count<=3; count++ )
    {
        if( count != 0 )
        {
            if( level > 0 )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level-1, style );
                CleanSpriteArea( player, &chunkRect );
            }
            
            if( level < kZapFrames )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level, style );
                CleanSpriteArea( player, &chunkRect );
            }
        }
    }
    
    SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void DrawChunks( int player, int x, int y, int level, int style )
{
	int count, color, type;
	MRect chunkRect;
	
    SDLU_AcquireSurface( playerSpriteSurface[player] );
    
    for( count=-3; count<=3; count++ )
    {
        if( count != 0 )
        {
            CalcBlobRect( x, y, &chunkRect );
            color = grid[player][x][y];
            GetZapStyle( player, &chunkRect, &color, &type, count, level, style );
            SurfaceDrawSprite( &chunkRect, color, type );
        }
    }
    
    SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void CleanSplat( int player, int x, int y, int level )
{
	int count, color, type;
	MRect chunkRect;

    SDLU_AcquireSurface( playerSpriteSurface[player] );
    
    for( count=-2; count<=2; count++ )
    {
        if( count != 0 )
        {
            if( level > 0 )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level-1, 4 );
                CleanSpriteArea( player, &chunkRect );
            }
            
            if( level < kZapFrames )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level, 4 );
                CleanSpriteArea( player, &chunkRect );
            }
        }
    }
    
    SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void DrawSplat( int player, int x, int y, int level )
{
	int count, color = kGray, type;
	MRect chunkRect;
	
    SDLU_AcquireSurface( playerSpriteSurface[player] );
    
    for( count=-2; count<=2; count++ )
    {
        if( level < kZapFrames && count != 0 )
        {
            CalcBlobRect( x, y, &chunkRect );
            GetZapStyle( player, &chunkRect, &color, &type, count, level, 4 );
            SurfaceDrawAlpha( &chunkRect, kGray, kLight, type );
        }
    }
    
    SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void InitZapStyle( void )
{
	int count, which;
	double x;
	const double position[kZapFrames] = {0, 10, 20, 28, 35, 42, 48, 54, 60, 64, 68, 70, 72, 73, 73, 74, 74, 75, 75, 75};
	const double offset[7]   = {-30, -50, -70, 0, -110, -130, -150};
	
	zapFont    = GetFont( picZapFont );
	zapOutline = GetFont( picZapOutlineFont );
	
	for( count=0; count<kZapFrames; count++ )
	{
		for( which=0; which<7; which++ )
		{
			x = d2r(offset[which]);
				
			zapOffsetX[which][count] = (int) floor( 0.5 + position[count] * cos( x ) );
			zapOffsetY[which][count] = (int) floor( 0.5 + position[count] * sin( x ) );
		}
	}
}


void GetZapStyle( int player, MRect *myRect, int *color, int *type, int which, int level, int style )
{
	const int chunkGraphic[] = { kSquish1, kSquish1, kSquish1, kSquish1, kSquish1, 
								 kSquish2, kSquish2, kSquish2, kSquish2, kSquish2,
								 kSquish3, kSquish3, kSquish3, kSquish3, kSquish3,
								 kSquish4, kSquish4, kSquish4, kSquish4, kSquish4 };
	
	(void) color; // later
	*type = chunkGraphic[level];
	
	switch(  style )
	{
		case 0:
		{
			const int direction[7][2] = { {0, -2}, {-2,-1}, {-2,1}, {0,0}, {2,-1}, {2,1}, {0, 2} };
			const int position[kZapFrames] = {0, 5, 9, 13, 17, 21, 24, 26, 30, 33, 35, 37, 39, 41, 42, 43, 43, 44, 44, 44 };
			
			OffsetMRect( myRect, direction[which+3][0] * position[level],
								direction[which+3][1] * position[level] );
			break;
		}

				
		case 1:
		{
			const int xVelocity = 3;
			const int yOffset[3][kZapFrames]   = {  { -4, -8, -12, -17, -22, -26, -30, -33, -36, -39, -41, -42, -43, -44, -44, -44, -43, -42, -41, -39 },
											        { -4, -7, -10, -14, -18, -21, -23, -25, -26, -27, -27, -27, -26, -25, -23, -21, -18, -14, -10, -7 },
											        { -2, -4, -5, -6, -7, -8, -9, -10, -10, -11, -11, -11, -10, -9, -8, -7, -6, -5, -3, -1 } };

			OffsetMRect( myRect, xVelocity * level * which, yOffset[abs(which)-1][level] );
			
			break;
		}
		
		case 2:
		{
			const int position[kZapFrames] = {0, 5, 9, 13, 17, 21, 24, 27, 30, 33, 35, 37, 39, 41, 42, 43, 43, 44, 44, 44 };
			
			OffsetMRect( myRect, 0, position[level] * which );
			break;
		}
		
		case 3:
		{
			double fLevel = ((double)level) / 2;
			OffsetMRect( myRect, (int)((player? -1.0: 1.0) * abs(which) * fLevel * (fLevel-1)), (int)((which-3) * fLevel) );
			break;
		}
		
		case 4:
		{
			OffsetMRect( myRect, zapOffsetX[which+3][level],
							 	 zapOffsetY[which+3][level] );
			
			break;
		}
	}
}
