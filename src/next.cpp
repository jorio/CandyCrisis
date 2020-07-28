// next.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "next.h"
#include "graphics.h"
#include "gworld.h"
#include "gameticks.h"
#include "random.h"
#include "blitter.h"
#include "level.h"

#define kJiggleFrames 8
#define kPulling 10
#define kPullingFrames 18

SDL_Surface* nextSurface;
SDL_Surface* nextDrawSurface;

MRect nextWindowZRect, nextWindowRect[2];
MBoolean nextWindowVisible[2] = {true, true};
int nextTime[2][2], nextStage[2][2], pullA[2], pullB[2];

void InitNext( void )
{
	const double windowLoc[] = {0.46, 0.54};
	SDL_Rect     sdlRect;

	nextWindowZRect.top = nextWindowZRect.left = 0;
	nextWindowZRect.bottom = 72; nextWindowZRect.right = 32;
	
	nextWindowRect[0] = nextWindowRect[1] = nextWindowZRect;
	CenterRectOnScreen( &nextWindowRect[0], windowLoc[0], 0.25 );
	CenterRectOnScreen( &nextWindowRect[1], windowLoc[1], 0.25 );	
	
	nextSurface = LoadPICTAsSurface( picNext, 32 );
	
	nextDrawSurface = SDLU_InitSurface( SDLU_MRectToSDLRect( &nextWindowZRect, &sdlRect ), 32 );
}

void RefreshNext( int player )
{
	nextStage[player][0] = 0;
    nextStage[player][1] = 0;

	nextTime[player][0] = GameTickCount( ) + RandomBefore( 60 );
	nextTime[player][1] = GameTickCount( ) + RandomBefore( 60 );

	ShowNext( player );
}

void PullNext( int player )
{
	pullA[player] = nextA[player];
	pullB[player] = nextB[player];
	nextStage[player][0] = kPulling;
	nextTime[player][0] = GameTickCount( );
}

#define kNoDraw 999
void ShowPull( int player )
{
	MRect    srcRect;
	int      yank[8] = { 20, 18, 15, 8, -6, -26, -46, -66 };
	int      slide[8] = { kNoDraw, 66, 48, 36, 29, 26, 24, 23 };
	int      drawA, drawB, offset, count;
	SDL_Rect sourceSDLRect, destSDLRect;
	
	if( !nextWindowVisible[player] ) return;
	
	SDLU_AcquireSurface( nextDrawSurface );
	
	SDLU_BlitSurface( nextSurface,     &nextSurface->clip_rect,
					  nextDrawSurface, &nextDrawSurface->clip_rect );
	
	for( count=0; count<2; count++ )
	{
		offset = nextStage[player][0] - kPulling;
		
		switch( count )
		{
			case 0: drawA = pullA[player]; drawB = pullB[player]; offset = yank[offset];  break;
			case 1: drawA = nextA[player]; drawB = nextB[player]; offset = slide[offset]; break;
		}
		
		if( offset != kNoDraw )
		{
			MRect blobRect = { 0, 4, 0 + kBlobVertSize, 4 + kBlobHorizSize };
			MRect shadowRect = { 4, 8, 4 + kBlobVertSize, 8 + kBlobHorizSize };

			OffsetMRect( &blobRect, 0, offset );
			OffsetMRect( &shadowRect, 0, offset );
					
			SurfaceDrawShadow( &shadowRect, drawB, kNoSuction );
			
			CalcBlobRect( kNoSuction, drawB-1, &srcRect );
			SurfaceBlitBlob( &srcRect, &blobRect );	  
					  
			OffsetMRect( &blobRect, 0, kBlobVertSize );
			OffsetMRect( &shadowRect, 0, kBlobVertSize );
			
			SurfaceDrawShadow( &shadowRect, drawA, nextM[player]? kFlashDarkBlob: kNoSuction );

			CalcBlobRect( nextM[player]? kFlashDarkBlob: kNoSuction, drawA-1, &srcRect );
			SurfaceBlitBlob( &srcRect, &blobRect );	  
		}
	}
	
	SDLU_ReleaseSurface( nextDrawSurface );
	
	SDLU_BlitFrontSurface( nextDrawSurface, 
	                       SDLU_MRectToSDLRect( &nextWindowZRect, &sourceSDLRect ),
	                       SDLU_MRectToSDLRect( &nextWindowRect[player], &destSDLRect ) );
}

void UpdateNext( int player )
{
	MBoolean changed = false;
	int blob;
	
	if( nextStage[player][0] >= kPulling )
	{
		if( GameTickCount() > nextTime[player][0] )
		{
			if( ++nextStage[player][0] >= kPullingFrames )
			{
				RefreshNext( player );
			}
			else
			{
				ShowPull( player );
				nextTime[player][0]++;
			}
		}
	} 
	else
	{
		for( blob=0; blob<2; blob++ )
		{	
			if( GameTickCount() > nextTime[player][blob] )
			{
				if( ++nextStage[player][blob] >= kJiggleFrames )
				{
					nextStage[player][blob] = 0;
					nextTime[player][blob] += 40 + RandomBefore( 80 );
				}
				else
				{
					nextTime[player][blob] += 2;
				}
					
				changed = true;
			}
		}
		
		if( changed ) ShowNext( player );
	}
}

void ShowNext( int player )
{
	int      jiggle[kJiggleFrames] = { kNoSuction,  kSquish,  kNoSuction,  kSquash,    
	                                   kNoSuction,  kSquish,  kNoSuction,  kSquash   };
	int      nextFrame = kNoSuction;
	MRect    blobRect = { 22, 4, 22 + kBlobVertSize, 4 + kBlobHorizSize };
	MRect    shadowRect = { 26, 8, 26 + kBlobVertSize, 8 + kBlobHorizSize };
	MRect    srcRect;
	SDL_Rect sourceSDLRect, destSDLRect;
	
	if( !nextWindowVisible[player] ) return;

	if( control[player] == kNobodyControl )
	{
	}
	else
	{
		SDLU_AcquireSurface( nextDrawSurface );

		SDLU_BlitSurface( nextSurface,     &nextSurface->clip_rect,
						  nextDrawSurface, &nextDrawSurface->clip_rect );
				
		nextFrame = nextG[player]? kNoSuction: jiggle[nextStage[player][0]];
		
		SurfaceDrawShadow( &shadowRect, nextB[player], nextFrame );
		
		CalcBlobRect( nextFrame, nextB[player]-1, &srcRect );
		SurfaceBlitBlob( &srcRect, &blobRect );	  
				  
		OffsetMRect( &blobRect, 0, kBlobVertSize );
		OffsetMRect( &shadowRect, 0, kBlobVertSize );

		nextFrame = nextG[player]? kNoSuction: 
						(nextM[player]? kFlashDarkBlob: jiggle[nextStage[player][1]]);
		
		SurfaceDrawShadow( &shadowRect, nextA[player], nextFrame );

		CalcBlobRect( nextFrame, nextA[player]-1, &srcRect );
		SurfaceBlitBlob( &srcRect, &blobRect );	  
		
		SDLU_ReleaseSurface( nextDrawSurface );

		SDLU_BlitFrontSurface( nextDrawSurface, 
		                       SDLU_MRectToSDLRect( &nextWindowZRect, &sourceSDLRect ),
		                       SDLU_MRectToSDLRect( &nextWindowRect[player], &destSDLRect ) );
	}
}
