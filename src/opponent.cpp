// opponent.c

#include "stdafx.h"
#include "SDLU.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "level.h"
#include "opponent.h"
#include "gworld.h"
#include "graphics.h"
#include "random.h"
#include "control.h"
#include "players.h"
#include "gameticks.h"
#include "blitter.h"

SDL_Surface* opponentSurface;
SDL_Surface* opponentMaskSurface;
SDL_Surface* opponentDrawSurface;

MRect opponentWindowZRect, opponentWindowRect;
int opponentMood, opponentFrame;
int opponentTime, glowTime[kGlows], glowFrame[kGlows], panicTime, panicFrame;
int heavyGlowArray[kGlowArraySize], glowArray[kGlowArraySize], lightGlowArray[kGlowArraySize];

void InitOpponent( void )
{
	MRect    littleRect = {0, 0, 64, 64}, bigRect = {0, 0, 64, 64*(kOppFrames*3) };
	SDL_Rect sdlRect;
	double   index, value;
	
	opponentDrawSurface = SDLU_InitSurface( SDLU_MRectToSDLRect( &littleRect, &sdlRect ), 32 );
	opponentSurface     = SDLU_InitSurface( SDLU_MRectToSDLRect( &bigRect, &sdlRect ), 32 );

	bigRect.bottom *= kGlows + 1;
	opponentMaskSurface = SDLU_InitSurface( SDLU_MRectToSDLRect( &bigRect, &sdlRect ), 1 );
	
	opponentWindowZRect.top = opponentWindowZRect.left = 0;
	opponentWindowZRect.bottom = opponentWindowZRect.right = 64;
	opponentWindowRect = opponentWindowZRect;
	CenterRectOnScreen( &opponentWindowRect, 0.5, 0.5 );
		
	opponentMood = 0;
	
	for( index=0; index<kGlowArraySize; index++ )
	{
		value = sin( index*pi/kGlowArraySize );
		value *= value;
		
		heavyGlowArray[(int)index] = (int)(value * 0.75  * 256);
		glowArray     [(int)index] = (int)(value * 0.50  * 256);
		lightGlowArray[(int)index] = (int)(value * 0.375 * 256);
	}
}

void BeginOpponent( int which )
{
	int count;

	DrawPICTInSurface( opponentSurface,     5000 + which );
	DrawPICTInSurface( opponentMaskSurface, 5100 + which );

	opponentTime = panicTime = GameTickCount( );
	for( count=0; count<kGlows; count++ )
	{
		glowTime[count] = panicTime;
		glowFrame[count] = 0;
	}
	
	opponentMood = 0;
	emotions[0] = emotions[1] = kEmotionNeutral;
}

void DrawFrozenOpponent( void )
{
	SDL_Rect   sourceSDLRect, destSDLRect;
	MRect      myRect = {0, 0, 64, 64};

	OffsetMRect( &myRect, opponentFrame * 64, 0 );

	SDLU_BlitFrontSurface( opponentSurface, 
	                       SDLU_MRectToSDLRect( &myRect, &sourceSDLRect ),
	                       SDLU_MRectToSDLRect( &opponentWindowRect, &destSDLRect ) );
}

void OpponentPissed( void )
{
	opponentMood = 7;
	opponentTime = GameTickCount();
}

void OpponentChatter( MBoolean on )
{
	switch( on )
	{
		case true:
			opponentMood = 5;
			opponentTime = GameTickCount();
			break;
			
		case false:
			opponentMood = 0;
			opponentTime = GameTickCount();
			break;
	}
}

void UpdateOpponent( void )
{
	MRect    myRect = {0,0,64,64}, dstRect = {0,0,64,64}, maskRect;
	int      emotiMap[] = {0, 1, 2, 1}, draw = false, count;
	SDL_Rect srcSDLRect, dstSDLRect;
	
	if( GameTickCount( ) > opponentTime )
	{
		switch( opponentMood )
		{
			case 0: 				// Idle
				opponentTime += 60 + RandomBefore(180);
				opponentMood = RandomBefore(2) + 1;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames);
				break;
			
			case 1:					// Shifty Eyes
				opponentTime += 40 + RandomBefore(60);
				opponentMood = 0;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + RandomBefore(2) + 1;
				break;

			case 2:					// Blinks
				opponentTime += 3;
				opponentMood = 3;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + 3;
				break;
			
			case 3:					// Blinks (more)
				opponentTime += 3;
				opponentMood = 4;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + 4;
				break;
			
			case 4: 				// Blinks (more)
				opponentTime += 3;
				opponentMood = 0;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + 3;
				break;
			
			case 5:                 // Chatter (only good for tutorial)
				opponentTime += 8;
				opponentMood = 6;
				opponentFrame = 5;
				break;

			case 6:					// Chatter 2 (only good for tutorial)
				opponentTime += 8;
				opponentMood = 5;
				opponentFrame = 6;
				break;
			
			case 7:					// Pissed (when hit with punishments)
				opponentTime += 60;
				opponentFrame = 7;
				opponentMood = 0;
				break;
		}
		
		draw = true;
	}
	
	if( GameTickCount( ) > panicTime )
	{
		panicTime += 2;
		
		if( emotions[1] == kEmotionPanic )
		{
			if( ++panicFrame >= kGlowArraySize ) panicFrame = 0;
			draw = true;
		}
		else
		{
			panicFrame = 0;
		}
	}
	
	for( count=0; count<kGlows; count++ )
	{
		if( GameTickCount( ) > glowTime[count] )
		{
			glowTime[count] += character[1].glow[count].time;
			
			if( character[1].glow[count].colorR || character[1].glow[count].colorG || character[1].glow[count].colorB )
			{
				if( ++glowFrame[count] >= kGlowArraySize ) glowFrame[count] = 0;
				draw = true;
			}
			else
			{
				glowFrame[count] = 0;
			}
		}
	}
	
	if( draw )
	{
		OffsetMRect( &myRect, 64*opponentFrame, 0 );
		
		SDLU_AcquireSurface( opponentDrawSurface );
		
		SDLU_BlitSurface( opponentSurface,     SDLU_MRectToSDLRect( &myRect, &srcSDLRect ),
		                  opponentDrawSurface, SDLU_MRectToSDLRect( &dstRect, &dstSDLRect )  );
		
		maskRect = myRect;
		for( count=0; count<kGlows; count++ )
		{
			OffsetMRect( &maskRect, 0, 64 );

			if( glowFrame[count] )
			{
				if( character[1].glow[count].isHeavy )
				{
					SurfaceBlitColor( opponentMaskSurface,  opponentDrawSurface,
					                  &maskRect,            &dstRect, 
					                   character[1].glow[count].colorR,
									   character[1].glow[count].colorG,
									   character[1].glow[count].colorB,
									   heavyGlowArray[glowFrame[count]] );
				}
				else
				{
					SurfaceBlitColor( opponentMaskSurface,  opponentDrawSurface,
					                  &maskRect,            &dstRect, 
					                   character[1].glow[count].colorR,
									   character[1].glow[count].colorG,
									   character[1].glow[count].colorB,
									   lightGlowArray[glowFrame[count]] );
				}
			}
		}
		
		if( panicFrame )
		{
			SurfaceBlitColor(  opponentMaskSurface,  opponentDrawSurface,
			                  &myRect,              &dstRect, 
			                   _5TO8(31), _5TO8(31), _5TO8(22), glowArray[panicFrame] );
		}
		
		SDLU_ReleaseSurface( opponentDrawSurface );
		
		SDLU_BlitFrontSurface( opponentDrawSurface, 
		                       SDLU_MRectToSDLRect( &opponentWindowZRect, &srcSDLRect ),
		                       SDLU_MRectToSDLRect( &opponentWindowRect,  &dstSDLRect )  );
	}
}
