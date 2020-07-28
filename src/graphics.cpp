// graphics.c

#include "stdafx.h"

#include <stdlib.h>
#include "SDLU.h"
#include "main.h"
#include "players.h"
#include "graphics.h"
#include "gworld.h"
#include "moving.h"
#include "tweak.h"
#include "gameticks.h"
#include "blitter.h"
#include "victory.h"
#include "grays.h"
#include "level.h"
#include "keyselect.h"


SDL_Surface* backdropSurface = NULL;


void DrawSpriteBlobs( int player, int type )
{
	MRect firstRect, secondRect, thirdRect;
	const int repeat = 0xFF, forever = 0xFE;
	
	static const unsigned char blobAnimation[6][2][25] = 
	{ 
	    { { kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
	        kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
	        kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,       
		    kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob,
		    kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob,
		    kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, repeat }, 
		  { kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction, repeat } },
		{ { kNoSuction,       kSquish,          kNoSuction,       kSquash,        
		    kNoSuction,       kSquish,          kNoSuction,       kSquash,
		    kNoSuction,       forever },
		  { kNoSuction,       kSquish,          kNoSuction,       kSquash,        
		    kNoSuction,       kSquish,          kNoSuction,       kSquash,
		    kNoSuction,       forever } },
		{ { kSobBlob,         kSobBlob,         kSobBlob,         kSobBlob,         
			kSob2Blob,        kSob2Blob,        kSob2Blob,        kSob2Blob,        
			repeat },
		  { kSobBlob,         kSobBlob,         kSobBlob,         kSobBlob,         
			kSob2Blob,        kSob2Blob,        kSob2Blob,        kSob2Blob,        
			repeat } },
		{ { kBombFuse1,       kBombFuse2,       kBombFuse3,       repeat }, 
		  { kBombFuse1,       kBombFuse2,       kBombFuse3,       repeat } }, 
		{ { kBlinkBomb1,      kBombFuse2,       kBlinkBomb3,      kBombFuse1,        
		    kBlinkBomb2,      kBombFuse3,       repeat },
		  { kBlinkBomb1,      kBombFuse2,       kBlinkBomb3,      kBombFuse1,        
		    kBlinkBomb2,      kBombFuse3,       repeat } }
	};
	
	if( grenade[player] ) type += 3;
	
	SDLU_AcquireSurface( playerSpriteSurface[player] );
	
	if( blobAnimation[type][0][anim[player]] == forever ) anim[player]--;
	if( blobAnimation[type][0][anim[player]] == repeat  ) anim[player] = 0;
	
	CalcBlobRect( blobX[player], blobY[player], &firstRect );
	if( halfway[player] ) OffsetMRect( &firstRect, 0, kBlobVertSize / 2 );
	
	TweakFirstBlob ( player, &firstRect );
	secondRect = firstRect;
	TweakSecondBlob( player, &secondRect );
		
	thirdRect = firstRect;
	thirdRect.top    -= kBlobShadowError;
	thirdRect.left   -= kBlobShadowError;
	thirdRect.right  += kBlobShadowDepth + kBlobShadowError;
	thirdRect.bottom += kBlobShadowDepth + kBlobShadowError;
	CleanSpriteArea( player, &thirdRect );
							
	thirdRect = secondRect;
	thirdRect.top    -= kBlobShadowError;
	thirdRect.left   -= kBlobShadowError;
	thirdRect.right  += kBlobShadowDepth + kBlobShadowError;
	thirdRect.bottom += kBlobShadowDepth + kBlobShadowError;
	CleanSpriteArea( player, &thirdRect );
	
	thirdRect = firstRect;
	OffsetMRect( &thirdRect, shadowDepth[player], shadowDepth[player] );
	SurfaceDrawShadow( &thirdRect,  colorA[player], blobAnimation[type][0][anim[player]] );

	thirdRect = secondRect;
	OffsetMRect( &thirdRect, shadowDepth[player], shadowDepth[player] );
	SurfaceDrawShadow( &thirdRect, colorB[player], blobAnimation[type][1][anim[player]] );

	SurfaceDrawSprite( &firstRect,  colorA[player], blobAnimation[type][0][anim[player]] );
	
	SurfaceDrawSprite( &secondRect, colorB[player], blobAnimation[type][1][anim[player]] );
	
	SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void CleanSpriteArea( int player, MRect *myRect )
{
	SDL_Rect sdlRect;

	SDLU_MRectToSDLRect( myRect, &sdlRect );
	
	SDLU_BlitSurface( playerSurface[player],       &sdlRect,
	                  playerSpriteSurface[player], &sdlRect  );
		
	SetUpdateRect( player, myRect );
}

void EraseSpriteBlobs( int player )
{
	MRect myRect, secondRect;
	
	CalcBlobRect( blobX[player], blobY[player], &myRect );
	if( halfway[player] ) OffsetMRect( &myRect, 0, kBlobVertSize / 2 );

	TweakFirstBlob( player, &myRect );
	secondRect = myRect;
	secondRect.top    -= kBlobShadowError;
	secondRect.left   -= kBlobShadowError;
	secondRect.right  += kBlobShadowDepth + kBlobShadowError;
	secondRect.bottom += kBlobShadowDepth + kBlobShadowError;
	CleanSpriteArea( player, &secondRect );

	TweakSecondBlob( player, &myRect );
	myRect.top    -= kBlobShadowError;
	myRect.left   -= kBlobShadowError;
	myRect.right  += kBlobShadowDepth + kBlobShadowError;
	myRect.bottom += kBlobShadowDepth + kBlobShadowError;
	CleanSpriteArea( player, &myRect );
}

void CalcBlobRect( int x, int y, MRect *myRect )
{
	myRect->top = y * kBlobVertSize;
	myRect->left = x * kBlobHorizSize;
	myRect->bottom = myRect->top + kBlobVertSize;
	myRect->right = myRect->left + kBlobHorizSize;
}

void InitBackdrop( void )
{
	backdropSurface = LoadPICTAsSurface( picBackdrop, 32 );
}

void DrawBackdrop( void )
{
	SDL_Rect backdropRect = { 0, 0, 640, 480 };
	
	SDLU_BlitFrontSurface( backdropSurface, &backdropRect, &backdropRect );
}

void ShowTitle( void )
{
    SDL_FillRect( g_frontSurface, &g_frontSurface->clip_rect, SDL_MapRGB( g_frontSurface->format, 0, 0, 0 ) );
    SDLU_Present();

    RetrieveResources( );

    int time = MTickCount() + 120;

	while( time > MTickCount() && !SDLU_Button() )
	{
        DrawPICTInSurface( g_frontSurface, picTitle );
        SDLU_Present();
		SDLU_Yield();
	}
	
	WaitForRelease();
		
	QuickFadeOut( NULL );
	
	SDL_FillRect( g_frontSurface, &g_frontSurface->clip_rect, SDL_MapRGB( g_frontSurface->format, 0, 0, 0 ) );
    SDLU_Present();
}

