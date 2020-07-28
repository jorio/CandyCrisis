// graymonitor.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "graymonitor.h"
#include "grays.h"
#include "score.h"
#include "gworld.h"
#include "graphics.h"
#include "level.h"


static SDL_Surface* smallGrayDrawSurface;


MRect grayMonitorZRect, grayMonitorRect[2];
MBoolean grayMonitorVisible[2] = {true, true};

void InitGrayMonitors( void )
{
	const double windowLoc[ ] = { 0.16, 0.84 };
	SDL_Rect     sdlRect;
			 
	grayMonitorZRect.top = grayMonitorZRect.left = 0;
	grayMonitorZRect.bottom = 32; grayMonitorZRect.right = 144;
	
	grayMonitorRect[0] = grayMonitorRect[1] = grayMonitorZRect;
	CenterRectOnScreen( &grayMonitorRect[0], windowLoc[0], 0.11 );
	CenterRectOnScreen( &grayMonitorRect[1], windowLoc[1], 0.11 );

	smallGrayDrawSurface = SDLU_InitSurface( SDLU_MRectToSDLRect( &grayMonitorZRect, &sdlRect ), 32 );
	DrawPICTInSurface( smallGrayDrawSurface, picBoard );
}

void ShowGrayMonitor( short player )
{
	SDL_Rect   sourceSDLRect, destSDLRect;
	short      monitor;
	MRect      myRect = { 4, 4, kBlobVertSize+4, 4 };
	MRect      srcRect;
	const int  smallGrayList[] = { 0, kSmallGray1, kSmallGray2, kSmallGray3, kSmallGray4, kSmallGray5 };
	
	if( !grayMonitorVisible[player] ) return;
	
	if( control[player] != kNobodyControl )
	{
		SDLU_AcquireSurface( smallGrayDrawSurface );
		
		SDLU_BlitSurface( boardSurface[player], &smallGrayDrawSurface->clip_rect,
						  smallGrayDrawSurface, &smallGrayDrawSurface->clip_rect  );
	 				
		monitor = unallocatedGrays[player];
		
		CalcBlobRect( kSobBlob, 3, &srcRect );
		while( monitor >= (6*4) )
		{
			myRect.right += kBlobHorizSize;
			SurfaceDrawSprite( &myRect, 4, kSobBlob );
			myRect.left = myRect.right;
			
			monitor -= (6*4);
		}
		
		CalcBlobRect( kNoSuction, kGray-1, &srcRect );
		while( monitor >= 6 )
		{
			myRect.right += kBlobHorizSize;
			SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
			myRect.left = myRect.right;
			
			monitor -= 6;
		}
		
		if( monitor > 0 )
		{
			myRect.right += kBlobHorizSize;
			SurfaceDrawAlpha( &myRect, kGray, kLight, smallGrayList[monitor] );
			myRect.left = myRect.right;
			myRect.right += kBlobHorizSize;
			SurfaceDrawAlpha( &myRect, kGray, kLight, smallGrayList[monitor]+1 );
		}
		
		SDLU_ReleaseSurface( smallGrayDrawSurface );

		SDLU_BlitFrontSurface( smallGrayDrawSurface, 
		                       SDLU_MRectToSDLRect( &grayMonitorZRect, &sourceSDLRect ),
		                       SDLU_MRectToSDLRect( &grayMonitorRect[player], &destSDLRect ) );
	}
}
