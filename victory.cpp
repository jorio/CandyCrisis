// victory.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "victory.h"
#include "players.h"
#include "gworld.h"
#include "grays.h"
#include "graphics.h"
#include "soundfx.h"
#include "score.h"
#include "control.h"
#include "random.h"
#include "tweak.h"
#include "gameticks.h"
#include "level.h"
#include "blitter.h"
#include "music.h"
#include "hiscore.h"
#include "keyselect.h"
#include "zap.h"
#include "pause.h"
#include "font.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int  winTime, loseTime;
int winStage, loseStage;
float drop[kGridAcross], last[kGridAcross];
SkittlesFontPtr victoryFont;

void InitVictory( void )
{
	victoryFont = GetFont( picVictoryFont );
}

void EndRound( int player )
{
	int count;

	loseTime = GameTickCount( );
	loseStage = 0;

	role[player] = kLosing;
	emotions[player] = kEmotionPanic;

	for( count=0; count<kGridAcross; count++ )
	{
		last[count] = 0.1f;
		drop[count] = 0.4f + RandomBefore(1000)/20000;
		rowBounce[player][count] = 99;
	}

	if( player == 0 )
	{
		ChooseMusic( -1 );
		PlayMono( kLoss );
	}
}

void BeginVictory( int player )
{
	int count;
	
	endTime = GameTickCount( );
	winTime = GameTickCount( );
	winStage = 0;
	
	role[player] = kWinning;	
	emotions[player] = kEmotionHappy;
	
	EraseSpriteBlobs( player );
	
	for( count=0; count<kGridAcross; count++ )
	{
		rowBounce[player][count] = 99;
	}

	if( player == 0 )
	{
		ChooseMusic( -1 );
		PlayMono( kVictory );
	}
}

void Lose( int player )
{
	int       gameTime = GameTickCount();
	int       skip = 1;
	MRect     boardRect;
	
	if( gameTime < loseTime )
		return;
	
	if( gameTime > loseTime )
	{
		skip = 2;
	}
		
	loseTime  += skip;
	loseStage += skip;

	if( loseStage < 120 )
	{
		DropLosers( player, skip );
	}
	else if( loseStage == 120 || loseStage == 121 )
	{
		loseStage = 122;

		boardRect.top    = boardRect.left = 0;
		boardRect.bottom = playerSurface[player]->h;
		boardRect.bottom = playerSurface[player]->w;

		SDLU_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &boardRect );
		SDLU_ReleaseSurface( playerSurface[player] );

		CleanSpriteArea( player, &boardRect );
	}
	else if( loseStage == 240 || loseStage == 241 )
	{
		loseStage = 242;
		if( players == 1 && control[player] == kPlayerControl )
		{
			if( --credits > 0 )
			{
				HandleDialog( kContinueDialog );
			}
			else
			{
				AddHiscore( score[player] );
				ShowGameOverScreen( );
				
				showStartMenu = true;
			}
		}
		else if( players == 2 )
		{
			AddHiscore( score[player] );
		}
	}
}

void DropLosers( int player, int skip )
{
	int x, y, suck;
	int beginDrop[] = { 28, 14, 0, 7, 21, 35 };
	float thisDrop;
	MRect myRect;
	
	SDLU_AcquireSurface( playerSpriteSurface[player] );

	for( x=0; x<kGridAcross; x++ )
	{
		if( loseStage >= beginDrop[x] )
		{
			thisDrop = last[x] + ( (float)(skip) * ( 0.7f + last[x] / 12.0f ) );
			
			CalcBlobRect( x, 0, &myRect );
			myRect.top = (int) last[x];
			myRect.bottom = kGridDown * kBlobVertSize;
			SurfaceDrawBoard( player, &myRect );
			SetUpdateRect( player, &myRect );

			if( thisDrop <  (kGridDown*kBlobVertSize) )
			{
				myRect.top = (int) thisDrop;
				myRect.bottom = myRect.top + kBlobVertSize;
				
				y=0;
				while( myRect.top < (kGridDown*kBlobVertSize) )
				{
					if( grid[player][x][y] >= kFirstBlob && 
						grid[player][x][y] <= kLastBlob )
					{
						suck = suction[player][x][y] & (kUpDown);
						if( suck == kNoSuction ) suck = kDying;
						SurfaceDrawBlob( player, &myRect, grid[player][x][y], suck, charred[player][x][y] );
					}
					else if( grid[player][x][y] == kGray )
					{
						SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
					} 
					
					OffsetMRect( &myRect, 0, kBlobVertSize );
					y++;
				}
				
				last[x] = thisDrop;
			}
		}
	}

	SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void Win( int player )
{
	int x, y;
	
	if( GameTickCount() >= winTime )
	{			
		if( winStage < (kGridDown * kGridAcross) )
		{
			y = (kGridDown-1) - (winStage / kGridAcross);
			x = (winStage % kGridAcross);
			if( y & 2 ) x = (kGridAcross-1) - x;
						
			if( grid[player][x][y] == kGray )
			{
				suction[player][x][y] = kGrayBlink1;
				death[player][x][y] = 0;
				score[player] += 20;
			}
			else if( grid[player][x][y] >= kFirstBlob && grid[player][x][y] <= kLastBlob )
			{
				suction[player][x][y] = kInDeath;
				death[player][x][y] = 0;
				score[player] += 100;
			}
		}
		else if( winStage == 140 && control[player] == kPlayerControl )
		{
			DrawTimerCount( player );
		}
		else if( winStage == 200 && control[player] == kPlayerControl )
		{
			DrawTimerBonus( player );
		}
		
		winTime++;
		winStage++;
	}
	
	if( winStage < 140 )
	{	
		KillBlobs( player );
	}

	if( winStage >= 280 )
	{
		if( control[player] == kPlayerControl )
		{
			IncrementLevel( );
			BeginRound( true );
		}
	}
}

void DrawTimerCount( int player )
{
	MRect playerRect;
	
	SDLU_AcquireSurface( playerSurface[player] );

	{
		MPoint dPoint  = { (kBlobVertSize * 3), 15 };		

		SurfaceBlitCharacter( victoryFont, 'A', &dPoint,  255, 255, 0, 1  );
	}	

	{
		MPoint dPoint  = { (kBlobVertSize * 4), kBlobHorizSize };
		char seconds[20];
		char *scan = seconds;
		
		sprintf( seconds, "%d", (endTime - startTime) / 60 );
		while( *scan )
		{
			SurfaceBlitCharacter( zapFont, *scan++, &dPoint, 255, 255, 255, 1  );
			dPoint.h--;
		}
		
		dPoint.h += 6;
		SurfaceBlitCharacter( zapFont, 'S', &dPoint,  255, 255, 255, 1  );
	}

	playerRect.top    = playerRect.left = 0;
	playerRect.bottom = playerSurface[player]->h;
	playerRect.right  = playerSurface[player]->w;
	
	CleanSpriteArea( player, &playerRect );
	PlayStereo( player, kSplop );
	
	SDLU_ReleaseSurface( playerSurface[player] );
}

void DrawTimerBonus( int player )
{
	MRect playerRect;
	int timer, bonus;
	
	SDLU_AcquireSurface( playerSurface[player] );
	
	{
		MPoint dPoint  = { (kBlobVertSize * 6),     15     };

		SurfaceBlitCharacter( victoryFont, 'B', &dPoint,  255, 255, 0, 1  );
	}
	
	timer = (endTime - startTime) / 60;
	     if( timer <= 10 ) bonus = 30000;
	else if( timer <= 20 ) bonus = 10000;
	else if( timer <= 30 ) bonus =  5000;
	else if( timer <= 45 ) bonus =  4000;
	else if( timer <= 60 ) bonus =  3000;
	else if( timer <= 80 ) bonus =  2000;
	else if( timer <=100 ) bonus =  1000;
	else if( timer <=120 ) bonus =   500;
	else                   bonus =     0;
	
	if( players == 1 ) bonus *= level;
	
	score[player] += bonus;
	
	{
		MPoint dPoint  = { (kBlobVertSize * 7), kBlobHorizSize };
		char points[20];
		char *scan = points;
		
		sprintf( points, "%d", bonus );
		while( *scan )
		{
			SurfaceBlitCharacter( zapFont, *scan++, &dPoint, 255, 255, 255, 1  );
			dPoint.h--;
		}
		
		dPoint.h += 6;
		SurfaceBlitCharacter( zapFont, 'P', &dPoint,  255, 255, 255, 1  );
	}
	
	playerRect.top    = playerRect.left = 0;
	playerRect.bottom = playerSurface[player]->h;
	playerRect.right  = playerSurface[player]->w;

	CleanSpriteArea( player, &playerRect );
	PlayStereo( player, kSplop );
	
	SDLU_ReleaseSurface( playerSurface[player] );
}
