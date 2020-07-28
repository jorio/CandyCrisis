// gworld.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "gworld.h"
#include "blitter.h"
#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


SDL_Surface* blobSurface;
SDL_Surface* maskSurface;
SDL_Surface* charMaskSurface;
SDL_Surface* boardSurface[2];
SDL_Surface* blastSurface;
SDL_Surface* blastMaskSurface;
SDL_Surface* playerSurface[2];
SDL_Surface* playerSpriteSurface[2];


void GetBlobGraphics()
{
	MRect myRect;
	
	// Get board
	
	myRect.top = myRect.left = 0;
	myRect.right = kBlobHorizSize * kGridAcross;
	myRect.bottom = kBlobVertSize * (kGridDown-1);
	
	boardSurface[0] = LoadPICTAsSurface( picBoard, 32 );
	
	boardSurface[1] = LoadPICTAsSurface( picBoardRight, 32 );
	if( boardSurface[1] == NULL )
		boardSurface[1] = LoadPICTAsSurface( picBoard, 32 );
	
	// Get blob worlds

	blobSurface = LoadPICTAsSurface( picBlob, 32 );
	maskSurface = LoadPICTAsSurface( picBlobMask, 1 );
	charMaskSurface = LoadPICTAsSurface( picCharMask, 1 );

	// Get blast worlds
	
	blastSurface = LoadPICTAsSurface( picBlast, 32 );
	blastMaskSurface = LoadPICTAsSurface( picBlastMask, 32 );
}


void InitPlayerWorlds()
{
	MRect     myRect;
	SDL_Rect  sdlRect;
	int       count;
	
	myRect.top = myRect.left = 0;
	myRect.right = kGridAcross * kBlobHorizSize;
	myRect.bottom = kGridDown * kBlobVertSize;
	
	SDLU_MRectToSDLRect( &myRect, &sdlRect );
	
	for( count=0; count<=1; count++ )
	{
		playerSurface[count]       = SDLU_InitSurface( &sdlRect, 32 );
		playerSpriteSurface[count] = SDLU_InitSurface( &sdlRect, 32 );
	}
}


void SurfaceDrawBoard( int player, const MRect *myRect )
{
	MRect    srcRect, offsetRect;
	SDL_Rect srcSDLRect, offsetSDLRect;
	
	srcRect = *myRect;
	if( srcRect.bottom <= kBlobVertSize ) return;
	if( srcRect.top < kBlobVertSize ) srcRect.top = kBlobVertSize;

	offsetRect = srcRect;
	OffsetMRect( &offsetRect, 0, -kBlobVertSize );

	SDLU_BlitSurface( boardSurface[player],     SDLU_MRectToSDLRect( &offsetRect, &offsetSDLRect ),
	                  SDLU_GetCurrentSurface(), SDLU_MRectToSDLRect( &srcRect, &srcSDLRect )         );
}


void SurfaceDrawBlob( int player, const MRect *myRect, int blob, int state, int charred )
{
	SurfaceDrawBoard( player, myRect );
	SurfaceDrawSprite( myRect, blob, state );

	if( charred & 0x0F )
	{
		MRect blobRect, charRect, alphaRect;
		
		CalcBlobRect( (charred & 0x0F), kBombTop-1, &charRect );
		CalcBlobRect( (charred & 0x0F), kBombBottom-1, &alphaRect );
		CalcBlobRect( state, blob-1, &blobRect );

		SurfaceBlitWeightedDualAlpha( SDLU_GetCurrentSurface(),  blobSurface,  charMaskSurface,  blobSurface,  SDLU_GetCurrentSurface(),
                                       myRect,                   &charRect,    &blobRect,        &alphaRect,    myRect, 
                                      (charred & 0xF0) );
	}
}

void SurfaceDrawShadow( const MRect *myRect, int blob, int state )
{
	int x;
	const MPoint offset[4] = { {-2, 0}, {0, -2}, {2, 0}, {0, 2} };
	
	if( blob > kEmpty )
	{
		MRect blobRect, destRect;

		for( x=0; x<4; x++ )
		{
			destRect = *myRect;
			OffsetMRect( &destRect, offset[x].h, offset[x].v );
			
			CalcBlobRect( state, blob-1, &blobRect );
			SurfaceBlitColor( maskSurface,  SDLU_GetCurrentSurface(),
			                  &blobRect,    &destRect, 
			                   0, 0, 0, _5TO8(3) );
		}
	}
}

void SurfaceDrawColor( const MRect *myRect, int blob, int state, int r, int g, int b, int w )
{
	MRect blobRect;
	if( blob > kEmpty )
	{
		CalcBlobRect( state, blob-1, &blobRect );
		SurfaceBlitColor( charMaskSurface,  SDLU_GetCurrentSurface(),
						  &blobRect,         myRect, 
						   r, g, b, w );
	}
}

void SurfaceDrawAlpha( const MRect *myRect, int blob, int mask, int state )
{
	if( blob > kEmpty )
	{
		MRect blobRect, alphaRect;

		CalcBlobRect( state, blob-1, &blobRect );
		CalcBlobRect( state, mask-1, &alphaRect );
		
		SurfaceBlitAlpha( SDLU_GetCurrentSurface(),  blobSurface,  blobSurface, SDLU_GetCurrentSurface(),
		                  myRect,                   &blobRect,    &alphaRect,   myRect );
	}
}

void SurfaceDrawSprite( const MRect *myRect, int blob, int state )
{
	MRect blobRect;
	if( blob > kEmpty )
	{
		CalcBlobRect( state, blob-1, &blobRect );
		SurfaceBlitBlob( &blobRect, myRect );
	}
}


MBoolean PICTExists( int pictID )
{
	if( FileExists( QuickResourceName( "PICT", pictID, ".jpg" ) ) )
		return true;

	if( FileExists( QuickResourceName( "PICT", pictID, ".png" ) ) )
		return true;
	
	return false;
}


SDL_Surface* LoadPICTAsSurface( int pictID, int depth )
{
	const char*  filename;
	SDL_Surface* surface;
    
	filename = QuickResourceName( "PICT", pictID, ".jpg" );
	if( FileExists( filename ) )
	{
		surface = IMG_Load( filename );
	}
	else
	{
		filename = QuickResourceName( "PICT", pictID, ".png" );
		if( FileExists( filename ) )
		{
			surface = IMG_Load( filename );
		}
		else
		{
			// Fail
			return NULL;
		}
	}
	
	if( depth != 0 )
	{
		SDLU_ChangeSurfaceDepth( &surface, depth );
	}
	
	return surface;
}

void DrawPICTInSurface( SDL_Surface* surface, int pictID )
{
	SDL_Surface* image;
	
	image = LoadPICTAsSurface( pictID, 0 );
	if( image != NULL )
	{
		SDLU_BlitSurface( image,    &image->clip_rect,
		                  surface,  &surface->clip_rect );
		                 
		SDL_FreeSurface( image );
	}
}
