// score.c

#include "stdafx.h"
#include "SDLU.h"

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "score.h"
#include "gworld.h"
#include "graphics.h"
#include "blitter.h"
#include "hiscore.h"
#include "gameticks.h"
#include "level.h"

SDL_Surface* scoreSurface;
SDL_Surface* numberSurface;
SDL_Surface* numberMaskSurface;


MRect scoreWindowZRect, scoreWindowRect[2];
MBoolean scoreWindowVisible[2] = {true, true};
int roundStartScore[2], score[2], displayedScore[2], scoreTime[2];
const char characterList[] = 
{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', '.', '0', '1', '2', 
  '3', '4', '5', '6', '7', '8', '9', '!', '"', '#',
  '$' };


void InitScore( void )
{
	const double windowLoc[ ] = { 0.16, 0.84 };
	SDL_Rect     sdlRect;
	
	scoreWindowZRect.top = scoreWindowZRect.left = 0;
	scoreWindowZRect.bottom = 32; scoreWindowZRect.right = 144;
	
	scoreWindowRect[0] = scoreWindowRect[1] = scoreWindowZRect;
	CenterRectOnScreen( &scoreWindowRect[0], windowLoc[0], 0.89 );
	CenterRectOnScreen( &scoreWindowRect[1], windowLoc[1], 0.89 );
	
	scoreSurface = SDLU_InitSurface( SDLU_MRectToSDLRect( &scoreWindowZRect, &sdlRect ), 32 );
	DrawPICTInSurface( scoreSurface, picBoard );
	
	numberSurface = LoadPICTAsSurface( picNumber, 32 );

	numberMaskSurface = LoadPICTAsSurface( picNumberMask, 1 );
	
	displayedScore[0] = displayedScore[1] = 0;
	score[0]          = score[1]          = 0;
	scoreTime[0]      = scoreTime[1]      = 0;
}

void UpdateScore( int player )
{
	if( GameTickCount( ) >= scoreTime[player] )
	{		
		scoreTime[player] = GameTickCount() + 1;
		
		if( displayedScore[player] < score[player] )
		{
			if( (score[player] - displayedScore[player]) > 5000 )
			{
				displayedScore[player] += 1525;
			}
			else if( (score[player] - displayedScore[player]) > 1000 )
			{
				displayedScore[player] += 175;
			}
			else
			{
				displayedScore[player] += 25;
			}
			
			if( displayedScore[player] > score[player] )
				displayedScore[player] = score[player];
			
			ShowScore( player );
		}
	}
}

void ShowScore( int player )
{
	SDL_Rect   sourceSDLRect, destSDLRect;
	MRect      myRect;
	char       myString[256];
	int        count;
	
	if( !scoreWindowVisible[player] ) return;
	
	if( control[player] != kNobodyControl )
	{
		sprintf( myString, "%d", displayedScore[player] );
				
		SDLU_AcquireSurface( scoreSurface );
		
		SDLU_BlitSurface( boardSurface[player], &scoreSurface->clip_rect,
				 		  scoreSurface,         &scoreSurface->clip_rect   );
		
		myRect.top = 0;
		myRect.left = 2;
		myRect.bottom = kNumberVertSize;
		myRect.right = myRect.left + kNumberHorizSize;
		DrawCharacter( kCharacterScore,   &myRect );
		OffsetMRect( &myRect, kNumberHorizSize, 0 );
		DrawCharacter( kCharacterScore+1, &myRect );

		myRect = scoreWindowZRect;
		myRect.right -= 2;
		myRect.left = myRect.right - kNumberHorizSize;
		for( count = int(strlen(myString)) - 1; count >= 0; count-- )
		{
			DrawCharacter( myString[count], &myRect );
			OffsetMRect( &myRect, -kNumberHorizSize - 1, 0 );
		}
		
		SDLU_ReleaseSurface( scoreSurface );
		
		SDLU_BlitFrontSurface( scoreSurface, 
		                       SDLU_MRectToSDLRect( &scoreWindowZRect, &sourceSDLRect ),
		                       SDLU_MRectToSDLRect( &scoreWindowRect[player], &destSDLRect ) );
	}
}

void DrawCharacter( char which, const MRect *myRect )
{
	MRect   srcRect;
	char    count, result;
	
	result = -1;
	for( count = 0; count < arrsize(characterList); count++ )
	{
		if( characterList[count] == which ) 
		{
			result = count;
			break;
		}
	}
	
	if( result == -1 ) return;
	
	srcRect.top    = 0;
	srcRect.left   = result * kNumberHorizSize;
	srcRect.bottom = kNumberVertSize;
	srcRect.right  = srcRect.left + kNumberHorizSize;
	
	SurfaceBlitMask(  numberSurface,  numberMaskSurface,  SDLU_GetCurrentSurface(),
			         &srcRect,       &srcRect,            myRect );
}
