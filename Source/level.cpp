// level.c

#include <stdlib.h>
#include <math.h>

#include "SDL.h"
#include "SDLU.h"

#include "main.h"
#include "level.h"
#include "score.h"
#include "random.h"
#include "grays.h"
#include "gameticks.h"
#include "players.h"
#include "graymonitor.h"
#include "opponent.h"
#include "gworld.h"
#include "graphics.h"
#include "music.h"
#include "control.h"
#include "tweak.h"
#include "soundfx.h"
#include "next.h"
#include "hiscore.h"
#include "victory.h"
#include "blitter.h"
#include "zap.h"
#include "keyselect.h"
#include "tutorial.h"
#include "pause.h"

MRect stageWindowZRect, stageWindowRect;
Character character[2];
int level, players, credits, difficulty[2] = {kHardLevel, kHardLevel};
int difficultyTicks, backdropTicks, backdropFrame;

#define kNumSplats 16
#define kIdleSplat -2
#define kFallingSplat -1
#define kTitleItems 7
#define kIncrementPerFrame 2
#define kSplatType 4

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))


const int startSkip = 1;
static MBoolean shouldFullRepaint = false;
static int startMenuTime = 0;
static int splatState[kNumSplats], splatColor[kNumSplats], splatSide[kNumSplats];
static MRect splatBlob[kNumSplats];
static int glowUpdate = 0, titleGlow[kTitleItems] = {24, 24, 24, 24, 24, 24, 24};
static MRect titleRect[kTitleItems] = {
		{ 155, 203, 207, 426 }, // tutorial
		{ 225, 179, 281, 451 }, // 1p
		{ 297, 182, 352, 454 }, // 2p
		{ 358, 183, 428, 458 }, // solitaire
		{ 429, 280, 478, 390 }, // high scores
		{ 433, 390, 477, 446 }, // quit
		{ 430, 187, 479, 280 }, // controls
		                };

const int kCursorWidth  = 32;
const int kCursorHeight = 32;

static void InsertCursor( MPoint mouseHere, SDL_Surface* scratch, SDL_Surface* surface )
{
	SkittlesFontPtr cursorFont = GetFont( picFont );
	SDL_Rect        cursorBackSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
	SDL_Rect        cursorFrontSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
	MPoint          mouseHereToo = mouseHere;
	
	cursorFrontSDLRect.x = mouseHere.h;
	cursorFrontSDLRect.y = mouseHere.v;
	
	SDLU_BlitSurface( surface, &cursorFrontSDLRect,
	                  scratch, &cursorBackSDLRect   );
	
	SDLU_AcquireSurface( surface );
	SurfaceBlitCharacter( cursorFont, '°', &mouseHere,    0,  0,  0, 0 );			
	SurfaceBlitCharacter( cursorFont, '¢', &mouseHereToo, 31, 31, 31, 0 );			
	SDLU_ReleaseSurface( surface );
}

static void RemoveCursor( MPoint mouseHere, SDL_Surface* scratch, SDL_Surface* surface )
{
	SDL_Rect      cursorBackSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
	SDL_Rect      cursorFrontSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
	MPoint        mouseHereToo = mouseHere;
	
	cursorFrontSDLRect.x = mouseHere.h;
	cursorFrontSDLRect.y = mouseHere.v;
	
	SDLU_BlitSurface( scratch, &cursorBackSDLRect,
	                  surface, &cursorFrontSDLRect );
}

static void GameStartMenuRepaint()
{
	shouldFullRepaint = true;
}

void GameStartMenu( void )
{
	// NOTE: be wary of initializing variables here! This function can run top-to-bottom
	// multiple times in a row, thanks to "redo". Put initializations after redo.
    SDL_Surface*    gameStartSurface;
    SDL_Surface*    gameStartDrawSurface;
    SDL_Surface*    cursorBackSurface;
	MRect           backdropPortZRect = { 0, 0, 480, 640 };
	SDL_Rect        backdropSDLRect = { 0, 0, 640, 480 };
	SDL_Rect        cursorBackSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
	SDL_Rect        cursorFrontSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
	SDL_Rect        destSDLRect;
	MRect           drawRect[4], chunkRect, tempRect;
	int             blob, count, oldGlow, splat, chunkType, selected;
	int             skip;
	MPoint          mouse;
	MPoint          dPoint;
	MPoint          sPoint[4];
	unsigned long   black;
	int             currentID;
	char            registeredString[256];
	char*           scan;
	int             combo[2], comboBright[2], missBright[2];
	SkittlesFontPtr smallFont = GetFont( picFont );
	SkittlesFontPtr tinyFont = GetFont( picTinyFont );
	SDL_Rect        meterRect[2] = { { 30, 360, 110, 20 }, { 530, 360, 110, 20 } };
 
	const int       kLeftSide = 0, kRightSide = 1, kGlow = 2, kCursor = 3;
	
redo:

	combo[0] = combo[1] = 0;
	comboBright[0] = comboBright[1] = 0;
	missBright[0] = missBright[1] = 0;
		
	skip = 1;
	selected = -1;
	mouse.h = mouse.v = 0;
	
	if( finished ) return;
	
	if( musicSelection != 13 ) ChooseMusic( 13 );
	
	for( count=0; count<kTitleItems; count++ )
	{
		titleGlow[count] = 24;
	}
	
	for( count=0; count<kNumSplats; count++ )
	{
		splatState[count] = kIdleSplat;
	}
	
	// make background surface
	gameStartSurface     = LoadPICTAsSurface( picGameStart, 16 );
	black = SDL_MapRGB( gameStartSurface->format, 0, 0, 0 );

	// make cursor backing store
	cursorBackSurface    = SDLU_InitSurface( &cursorBackSDLRect, 16 );
	SDL_FillRect( cursorBackSurface, &cursorBackSDLRect, black );
	
	// draw user name with a cool blue halo
	if( IsRegistered() )
	{
		sprintf( registeredString, "Registered to %s", registeredName );
	}
	else
	{
		sprintf( registeredString, "U N R E G I S T E R E D" );
	}
	
	dPoint.v = 131;
	dPoint.h = 320 - (GetTextWidth( smallFont, registeredString ) / 2);
	sPoint[0].v = dPoint.v + 1;	sPoint[0].h = dPoint.h + 1;
	sPoint[1].v = dPoint.v - 1;	sPoint[1].h = dPoint.h - 1;
	sPoint[2].v = dPoint.v - 1;	sPoint[2].h = dPoint.h + 1;
	sPoint[3].v = dPoint.v + 1;	sPoint[3].h = dPoint.h - 1;
 
	SDLU_AcquireSurface( gameStartSurface );
	for( scan = registeredString; *scan; scan++ )
	{		
		SurfaceBlitCharacter( smallFont, *scan, &sPoint[0], 0, 0, 25, 0 );			
		SurfaceBlitCharacter( smallFont, *scan, &sPoint[1], 0, 0, 25, 0 );			
		SurfaceBlitCharacter( smallFont, *scan, &sPoint[2], 0, 0, 25, 0 );			
		SurfaceBlitCharacter( smallFont, *scan, &sPoint[3], 0, 0, 25, 0 );			
		SurfaceBlitCharacter( smallFont, *scan, &dPoint, 30, 30, 30, 0 );			
	}
	SDLU_ReleaseSurface( gameStartSurface );
	
	// make drawing surface
	gameStartDrawSurface = SDLU_InitSurface( &backdropSDLRect, 16 );
	SDLU_BlitSurface( gameStartSurface,     &gameStartSurface->clip_rect,
					  gameStartDrawSurface, &gameStartDrawSurface->clip_rect );
	
	// darken menu items
	for( count=0; count<kTitleItems; count++ )
	{
		SurfaceBlitColorOver(  gameStartSurface,  gameStartDrawSurface, 
		                      &titleRect[count], &titleRect[count], 
		                       0, 0, 0, titleGlow[count] );
	}
	
	SDLU_BlitFrontSurface( gameStartDrawSurface, &backdropSDLRect, &backdropSDLRect );

	WaitForRelease();

	QuickFadeIn( NULL );
	
	DoFullRepaint = GameStartMenuRepaint;

	startMenuTime = MTickCount( );
	while( ( selected == -1 || !SDLU_Button() ) && !finished )
	{	
		startMenuTime += skip;

		// Add a new falling blob 
		if( glowUpdate == 0 ) 
		{
			for( blob=0; blob<kNumSplats; blob++ )
			{
				if( splatState[blob] == kIdleSplat )
				{
					splatSide[blob] = RandomBefore(2);
					splatBlob[blob].top = -24 - RandomBefore(15);
					splatBlob[blob].left = (splatSide[blob] == 0)? RandomBefore( 110 ): 640 - kBlobHorizSize - RandomBefore( 110 );
					splatBlob[blob].bottom = splatBlob[blob].top + kBlobVertSize;
					splatBlob[blob].right = splatBlob[blob].left + kBlobHorizSize;
					splatColor[blob] = ((startMenuTime >> 2) % kBlobTypes) + 1;
					splatState[blob] = kFallingSplat;
					
					break;
				}
			}
		}
	
		// Erase and redraw falling blobs and chunks

		SDLU_AcquireSurface( gameStartDrawSurface );
	
		// Take the cursor out of the scene
		RemoveCursor( mouse, cursorBackSurface, gameStartDrawSurface );
		drawRect[kCursor].top    = mouse.v;
		drawRect[kCursor].left   = mouse.h;
		drawRect[kCursor].bottom = mouse.v + kCursorHeight;
		drawRect[kCursor].right  = mouse.h + kCursorWidth;
		
		// is this a hack? maybe. but it works!
		drawRect[kLeftSide].top    = drawRect[kRightSide].top    = drawRect[kGlow].top    = 
		drawRect[kLeftSide].left   = drawRect[kRightSide].left   = drawRect[kGlow].left   = 9999;
		drawRect[kLeftSide].bottom = drawRect[kRightSide].bottom = drawRect[kGlow].bottom =
		drawRect[kLeftSide].right  = drawRect[kRightSide].right  = drawRect[kGlow].right  = -9999;
	
		// Get cursor position		
		SDLU_GetMouse( &mouse );
        if( mouse.v > 460 ) mouse.v = 460;
        
		// Erase falling blobs
		for( blob=0; blob<kNumSplats; blob++ )
		{
			if( splatState[blob] == kFallingSplat )
			{
				SDL_FillRect( gameStartDrawSurface, SDLU_MRectToSDLRect( &splatBlob[blob], &destSDLRect ), black );
				UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
				
				OffsetMRect( &splatBlob[blob], 0, startSkip * (6 + (splatBlob[blob].bottom / 20)) );
			}
			else if( splatState[blob] >= kIncrementPerFrame )
			{
				for( splat=-3; splat<=3; splat++ )
				{
					if( splat )
					{
						chunkRect = splatBlob[blob];
						GetZapStyle( 0, &chunkRect, &splatColor[blob], &chunkType, splat, splatState[blob]-kIncrementPerFrame, kSplatType );
						SDL_FillRect( gameStartDrawSurface, SDLU_MRectToSDLRect( &chunkRect, &destSDLRect ), black );
						UnionMRect( &drawRect[splatSide[blob]], &chunkRect, &drawRect[splatSide[blob]] );
					}
				}
				
				SDL_FillRect( gameStartDrawSurface, SDLU_MRectToSDLRect( &splatBlob[blob], &destSDLRect ), black );
				UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
			}
		}
	
		// Draw combo meters (GEEK!!!!!)
        
		for( count=0; count<2; count++ )
		{
			int bright = comboBright[count];
			int mBright = missBright[count];
            if( bright || mBright )
			{
                SDL_FillRect( gameStartDrawSurface, &meterRect[count], black );
                UnionMRect( &drawRect[count], SDLU_SDLRectToMRect( &meterRect[count], &tempRect ), &drawRect[count] );
                
                if( mBright > 1 )
                {
                  dPoint.v = meterRect[count].y;
                  dPoint.h = meterRect[count].x + 10;
                  SurfaceBlitCharacter( smallFont, 'M', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                  SurfaceBlitCharacter( smallFont, 'I', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                  SurfaceBlitCharacter( smallFont, 'S', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                  SurfaceBlitCharacter( smallFont, 'S', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                  missBright[count]--;
                }
                else if( (combo[count] >= 10) && (bright > 1) )
                {
				  char  number[16] = { 0 };
				  char* scan;
				  sprintf( number, "%d", combo[count] );

                  dPoint.v = meterRect[count].y + 3;
                  dPoint.h = meterRect[count].x;
     
				  SurfaceBlitCharacter( tinyFont, 'C', &dPoint, bright, bright, bright, 1 );
				  SurfaceBlitCharacter( tinyFont, 'O', &dPoint, bright, bright, bright, 1 );
				  SurfaceBlitCharacter( tinyFont, 'M', &dPoint, bright, bright, bright, 1 );
				  SurfaceBlitCharacter( tinyFont, 'B', &dPoint, bright, bright, bright, 1 );
				  SurfaceBlitCharacter( tinyFont, 'O', &dPoint, bright, bright, bright, 1 );
				  SurfaceBlitCharacter( tinyFont, ' ', &dPoint, bright, bright, bright, 1 );
				  dPoint.v -= 3;
				
				  for( scan = number; *scan; scan++ )
				  {
				 	SurfaceBlitCharacter( smallFont, *scan, &dPoint, bright>>2, bright>>2, bright, 1 );
			      }
                  
                  comboBright[count] -= 2;
				}
                else 
                {
                  comboBright[count] = 0;
                }
   			}
		}

		// Redraw falling blobs
		for( blob=0; blob<kNumSplats; blob++ )
		{
			if( splatState[blob] == kFallingSplat )
			{
				if( splatBlob[blob].bottom >= 480 ) 
				{
					splatBlob[blob].top = 480 - kBlobVertSize;
					splatBlob[blob].bottom = 480;
					splatState[blob] = 1;
					
                    // Process combos
					if( mouse.v > 420 &&
					    mouse.h >= (splatBlob[blob].left - 30) &&
					    mouse.h <= (splatBlob[blob].right + 10)    )
					{
						combo[splatSide[blob]]++;
						comboBright[splatSide[blob]] = 31;
					}
					else
					{
                        if( combo[splatSide[blob]] >= 10 ) missBright[splatSide[blob]] = 31;
						combo[splatSide[blob]] = 0;
                        comboBright[splatSide[blob]] = 0;                        
					}
				}
				else
				{
					SurfaceDrawSprite( &splatBlob[blob], splatColor[blob], kNoSuction );
					UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
				}
			}
			
			if( splatState[blob] >= 0 && splatState[blob] <= kZapFrames )
			{
				if( splatState[blob] <= (kZapFrames - kIncrementPerFrame) ) 
				{
					for( splat=-3; splat<=3; splat++ )
					{
						if( splat )
						{
							chunkRect = splatBlob[blob];
							GetZapStyle( 0, &chunkRect, &splatColor[blob], &chunkType, splat, splatState[blob], kSplatType );
							SurfaceDrawSprite( &chunkRect, splatColor[blob], chunkType );
							UnionMRect( &drawRect[splatSide[blob]], &chunkRect, &drawRect[splatSide[blob]] );
						}
					}
					
					SurfaceDrawSprite( &splatBlob[blob], splatColor[blob], chunkType );
					UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
				}
				
				splatState[blob] += kIncrementPerFrame;
				if( splatState[blob] > kZapFrames ) splatState[blob] = kIdleSplat;
			}
		}
				
		SDLU_ReleaseSurface( gameStartDrawSurface );		
		
		// Find mouse coords
		
		selected = -1;			
		for( count=0; count<kTitleItems; count++ )
		{
			if( MPointInMRect( mouse, &titleRect[count] ) )
			{
				selected = count;
				break;
			}
		}

		// update glows
		do
		{
            glowUpdate = (glowUpdate + 1) % 7;

			oldGlow = titleGlow[glowUpdate];
			
			if( selected == glowUpdate )
			{
				titleGlow[glowUpdate] -= (4 * startSkip);
				if( titleGlow[glowUpdate] < 0 ) titleGlow[glowUpdate] = 0;
			}
			else 
			{
				titleGlow[glowUpdate] += (4 * startSkip);
				if( titleGlow[glowUpdate] > 24 ) titleGlow[glowUpdate] = 24;
			}
			
			if( titleGlow[glowUpdate] != oldGlow )
			{
				SurfaceBlitColorOver(  gameStartSurface,       gameStartDrawSurface,
				                      &titleRect[glowUpdate], &titleRect[glowUpdate],
				                       0, 0, 0, titleGlow[glowUpdate] );

				drawRect[kGlow] = titleRect[glowUpdate];
			}

			// do 5, 6, and 7 all at once because they're small
		    if( glowUpdate == 4 || glowUpdate == 5 ) continue;
		}
		while( 0 );

		// Reinsert the cursor into the scene
		InsertCursor( mouse, cursorBackSurface, gameStartDrawSurface );
		drawRect[kCursor].top    = min( drawRect[kCursor].top,    mouse.v );
		drawRect[kCursor].left   = min( drawRect[kCursor].left,   mouse.h );
		drawRect[kCursor].bottom = max( drawRect[kCursor].bottom, mouse.v + kCursorHeight );
		drawRect[kCursor].right  = max( drawRect[kCursor].right,  mouse.h + kCursorWidth );

		// Copy down everything		
		if( shouldFullRepaint )
		{
			SDLU_BlitFrontSurface( gameStartDrawSurface, &gameStartDrawSurface->clip_rect, &gameStartDrawSurface->clip_rect );
			shouldFullRepaint = false;
		}
		else
		{
			for( count=0; count<4; count++ )
			{		
				if( drawRect[count].left < drawRect[count].right )
				{
					SDLU_MRectToSDLRect( &drawRect[count], &destSDLRect );			
					SDLU_BlitFrontSurface( gameStartDrawSurface, &destSDLRect, &destSDLRect );
				}
			}
		}
				
		// Skip frames? Or delay?
		if( startMenuTime <= MTickCount( ) )
		{
			startMenuTime = MTickCount( );
			skip = 2;
		}
		else
		{
			skip = 1;
			while( startMenuTime > MTickCount( ) ) 
			{   
                SDLU_Yield();
			}
		}
	}

	DoFullRepaint = NoPaint;

	if( finished ) 
	{
		selected = 5; // quit
	}
	
	switch( selected )
	{
		case 0: 
		case 1: 
		case 2:
		case 3:
			PlayMono( kChime ); 
			break;
	}

	SDL_FreeSurface( gameStartSurface );
	SDL_FreeSurface( gameStartDrawSurface );
	SDL_FreeSurface( cursorBackSurface );
	
	QuickFadeOut( NULL );

	switch( selected )
	{
		case 0: 
			InitGame( kAutoControl, kNobodyControl );
			level = kTutorialLevel;
			BeginRound( true );
			InitTutorial( );
			QuickFadeIn( NULL );
			break;				

		case 1:
		case 2:
		case 3:
			{
				int player2[] = { 0, kAIControl, kPlayerControl, kNobodyControl };
				
				InitGame( kPlayerControl, player2[selected] );
				BeginRound( true );
				QuickFadeIn( NULL );
				break;
			}
		
		case 4: 
			ShowHiscore();
			ShowBestCombo();
			break;
			
		case 5:
			finished = true;
			break;
		
		case 6:
			currentID = RandomBefore( IsRegistered()? kLevels: kSharewareLevels ) * 100;
	
			DrawPICTInSurface( boardSurface[0], picBoard + currentID );	
			DrawPICTInSurface( frontSurface, picBackdrop + currentID );
			SDL_Flip( frontSurface );		
	
			QuickFadeIn( NULL );
			HandleDialog( kControlsDialog );
			QuickFadeOut( NULL );
			goto redo;
	}
}

void ShowGameOverScreen( void )
{
	unsigned long timer = MTickCount() + (60*3);
	
	QuickFadeOut(NULL);

	DrawPICTInSurface( frontSurface, picGameOver );
	SDL_Flip( frontSurface );

	QuickFadeIn( NULL );
	do
	{
		if( MTickCount() > timer ) break;
		SDLU_Yield();
	}
	while( !AnyKeyIsPressed( ) && !SDLU_Button() );
	QuickFadeOut( NULL );
}

void InitStage( void )
{
	stageWindowZRect.top = stageWindowZRect.left = 0;
	stageWindowZRect.bottom = 32; stageWindowZRect.right = 64; 
	
	stageWindowRect = stageWindowZRect;
	CenterRectOnScreen( &stageWindowRect, 0.5, 0.65 );
}

void DrawStage( void )
{
	SDL_Surface* levelSurface;
	SDL_Rect     sourceSDLRect, destSDLRect;
	MRect        numberRect = { 0, kNumberHorizSize/8, kNumberVertSize, kNumberHorizSize*9/8 };
			
	switch( players )
	{
		case 0:
		case 2:
			break;
			
		case 1:
			SDLU_MRectToSDLRect( &stageWindowZRect, &sourceSDLRect );
			SDLU_MRectToSDLRect( &stageWindowRect,  &destSDLRect );
			
			levelSurface = SDLU_InitSurface( &sourceSDLRect, 16 );

			SDLU_AcquireSurface( levelSurface );
			
			SDLU_BlitSurface( boardSurface[0], &sourceSDLRect,
							  levelSurface,    &sourceSDLRect   );
			
			if( level < 10 )
			{
				OffsetMRect( &numberRect, kNumberHorizSize*3/8, 0 );
			}
			
			DrawCharacter( kCharacterStage,   &numberRect );
			OffsetMRect( &numberRect, kNumberHorizSize, 0 );
			DrawCharacter( kCharacterStage+1, &numberRect );

			if( level < 10 )
			{
				OffsetMRect( &numberRect, kNumberHorizSize, 0 );
				DrawCharacter( level + '0', &numberRect );
			}
			else
			{
				OffsetMRect( &numberRect, kNumberHorizSize*3/4, 0 );
				DrawCharacter( (level / 10) + '0', &numberRect );
				OffsetMRect( &numberRect, kNumberHorizSize, 0 );
				DrawCharacter( (level % 10) + '0', &numberRect );
			}

			SDLU_BlitFrontSurface( levelSurface, &sourceSDLRect, &destSDLRect );
			
			SDL_FreeSurface( levelSurface );
						
			break;
	}
}

void InitGame( int player1, int player2 )
{
	playerWindowVisible[0] = true;
	nextWindowVisible[0] = true;
	scoreWindowVisible[0] = true;
	grayMonitorVisible[0] = true;
	
	if( player2 == kNobodyControl )
	{
		playerWindowVisible[1] = false;
		nextWindowVisible[1] = false;
		scoreWindowVisible[1] = false;
		grayMonitorVisible[1] = false;

		CenterRectOnScreen( &playerWindowRect[0], 0.5, 0.5  );
		CenterRectOnScreen( &scoreWindowRect[0],  0.5, 0.89 );
		CenterRectOnScreen( &grayMonitorRect[0],  0.5, 0.11 );
		CenterRectOnScreen( &nextWindowRect[0],   0.3, 0.25 );
		
		CenterRectOnScreen( &stageWindowRect,	  0.3, 0.65 );		
		CenterRectOnScreen( &opponentWindowRect,  0.3, 0.5 );		
	}
	else
	{
		playerWindowVisible[1] = true;
		nextWindowVisible[1] = true;
		scoreWindowVisible[1] = true;
		grayMonitorVisible[1] = true;

		CenterRectOnScreen( &playerWindowRect[0], kLeftPlayerWindowCenter, 0.5  );
		CenterRectOnScreen( &scoreWindowRect[0],  kLeftPlayerWindowCenter, 0.89 );
		CenterRectOnScreen( &grayMonitorRect[0],  kLeftPlayerWindowCenter, 0.11 );
		CenterRectOnScreen( &nextWindowRect[0],   0.46, 0.25 );

		CenterRectOnScreen( &playerWindowRect[1], kRightPlayerWindowCenter, 0.5  );
		CenterRectOnScreen( &scoreWindowRect[1],  kRightPlayerWindowCenter, 0.89 );
		CenterRectOnScreen( &grayMonitorRect[1],  kRightPlayerWindowCenter, 0.11 );
		CenterRectOnScreen( &nextWindowRect[1],   0.54, 0.25 );

		CenterRectOnScreen( &stageWindowRect,    0.5, 0.65 );		
		CenterRectOnScreen( &opponentWindowRect, 0.5, 0.5 );		
	}
	
	nextWindowVisible[0] = ( player1 == kAutoControl )? false: true;
	
	players = (player1 == kPlayerControl) + (player2 == kPlayerControl);
	
	if( players < 2 )
	{
		difficulty[0] = difficulty[1] = kHardLevel;
	}
	
	control[0] = player1;
	control[1] = player2;
	
	score[0] = score[1] = displayedScore[0] = displayedScore[1] = 0;	
	roundStartScore[0] = roundStartScore[1] = 0;
	
	level = 1;
	credits = (player2 == kNobodyControl)? 1: 5;
}

MBoolean InitCharacter( int player, int level )
{
	const Character characterList[] = {
		{ -1 }, // no zero'th character
		{ 0, 3, 1, { 8, 8, 8, 8, 8, 8 }, 13, 9, 0, 25, { 0, 0, 0, 0 }, true },
		{ 1, 6, 2, { 10, 9, 8, 8, 9, 10 }, 12, 7, 1, 20, { 223, 7, 0, 0 }, true },
		{ 2, 9, 3, { 7, 7, 7, 11, 7, 7 }, 10, 6, 2, 17, { 0, 0, 0, 0 }, false },
		{ 3, 12, 4, { 11, 10, 9, 8, 7, 6 }, 8, 5, 3, 13, { 32767, 4, 16912, 4 }, false },
		{ 4, 15, 0, { 5, 9, 10, 10, 9, 5 }, 7, 4, 4, 10, { 32767, 1, 0, 0 }, false },
		{ 5, 17, 1, { 4, 7, 11, 11, 6, 3 }, 7, 2, 5, 8, { 14835, 8, 0, 0 }, false },
		{ 6, 18, 2, { 7, 9, 10, 10, 9, 7 }, 6, 4, 6, 7, { 0, 0, 0, 0 }, false },
		{ 7, 20, 3, { 5, 10, 10, 10, 10, 5 }, 5, 3, 7, 5, { 9696, 2, 21151, 3 }, false },
		{ 8, 21, 4, { 11, 11, 10, 10, 9, 9 }, 4, 3, 8, 5, { 32738, 5, 0, 0 }, false },
		{ 9, 22, 0, { 11, 7, 11, 7, 11, 7 }, 3, 1, 9, 4, { 32356, 5, 17392, 3 }, false },
		{ 10, 23, 1, { 11, 11, 11, 11, 11, 11 }, 2, 1, 10, 2, { 6337, 1, 0, 0 }, false },
		{ 11, 24, 2, { 11, 11, 11, 11, 11, 11 }, 2, 1, 11, 2, { -1, 7, 0, 0 }, false },
		{ -1 }, // skip
		{ 13, 24, 1, { 11, 11, 11, 11, 11, 11 }, 10, 5, 0, 30, { 0, 0, 0, 0 }, true }
	};
		
	if( !IsRegistered() )
	{
		int levelCap = kSharewareLevels;
		if( control[1] == kNobodyControl ) levelCap = kSharewareSolitaireLevels;
		
		if( level > levelCap && level != kTutorialLevel ) return false;
	}

	character[player] = characterList[level];
	return (character[player].picture != -1);
}

void PrepareStageGraphics( int type )
{
	int player;
	                            
	MRect blobBoard = { 0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
	
	backgroundID = type * 100;
	
	DrawPICTInSurface( boardSurface[0], picBoard + backgroundID );	
	
	// NOTE: Many levels have no right-side board, so we copy the left
	// side over to the right side. This way, if DrawPICTInSurface flunks,
	// we still have a valid picture.
	
	SDLU_BlitSurface( boardSurface[0], &boardSurface[0]->clip_rect,
	                  boardSurface[1], &boardSurface[1]->clip_rect  );
	
	DrawPICTInSurface( boardSurface[1], picBoardRight + backgroundID );	

	DrawPICTInSurface( backdropSurface, picBackdrop + backgroundID );

	DrawPICTInSurface( nextSurface, picNext + backgroundID );
	
	for( player=0; player<=1; player++ )
	{
		SDLU_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &blobBoard );
		SDLU_ReleaseSurface( playerSurface[player] );
		
		CleanSpriteArea( player, &blobBoard );
	}
	
	BeginOpponent( type );

	RedrawBoardContents( 0 );
	RedrawBoardContents( 1 );
	
	RefreshAll( );

	backdropTicks = MTickCount( );
	backdropFrame = 0;
}

void BeginRound( MBoolean changeMusic )
{
	int player, count, count2;
	
	InitGrays( );
	InitPotentialCombos( );
	
	switch( players )
	{
		case 0:
		case 1:
			if( InitCharacter( 1, level ) )
			{
				score[1] = roundStartScore[1] = displayedScore[1] = 0;
				character[0] = character[1];
				character[0].zapStyle = RandomBefore(5);
			}
			else
			{
				TotalVictory( );
				return;
			}
			
			if( control[1] == kNobodyControl )
			{
				InitRandom( 3 );
			}
			else
			{
				InitRandom( 5 );
			}
			break;
			
		case 2:
			score[0] = score[1] = roundStartScore[0] = roundStartScore[1] = displayedScore[0] = displayedScore[1] = 0;	

			InitRandom( 5 );

			SelectRandomLevel( );
			InitCharacter( 0, level );
			
			SelectRandomLevel( );
			InitCharacter( 1, level );
			
			character[0].hints = (difficulty[0] == kEasyLevel) || (difficulty[0] == kMediumLevel);
			character[1].hints = (difficulty[1] == kEasyLevel) || (difficulty[1] == kMediumLevel);
			break;
	}
	
	for( player=0; player<=1; player++ )
	{
		for( count=0; count<kGridAcross; count++ )
		{
			grays[player][count] = 0;
			
			for( count2=0; count2<kGridDown; count2++ )
			{
				grid[player][count][count2] = kEmpty;
				suction[player][count][count2] = kNoSuction;
				charred[player][count][count2] = kNoCharring;
				glow[player][count][count2] = false;
			}
		}
		
		nextA[player] = GetPiece( player );
		nextB[player] = GetPiece( player );
		nextM[player] = false;
		nextG[player] = false;
		
		halfway[player] = false;
		
		unallocatedGrays[player] = 0;
		anim[player] = 0;
		lockGrays[player] = 0;
		roundStartScore[player] = score[player];
		
		RedrawBoardContents(player);
		
		if( control[player] != kNobodyControl )
		{
			role[player] = kWaitForRetrieval;
		}
		else
		{
			role[player] = kIdlePlayer;
		}
	}
	
	PrepareStageGraphics( character[1].picture );
	if( changeMusic ) ChooseMusic( character[1].music );
	
	blobTime[0]     = blobTime[1]     = 
	boredTime[0]    = boredTime[1]    = 
	hintTime[0]     = hintTime[1]     =
	timeAI[0]       = timeAI[1]       =
	fadeCharTime[0] = fadeCharTime[1] = 
	messageTime     = startTime       =
	blinkTime[0]    = blinkTime[1]    = GameTickCount( );
	
	blinkTime[1] += 60;
	
	if( players == 2 )
		InitDifficulty( );
}

void IncrementLevel( void )
{
	level++;
}

void SelectRandomLevel( void )
{
	level = RandomBefore( IsRegistered()? kLevels: kSharewareLevels ) + 1;
}

void InitDifficulty( )
{
	MRect blobBoard = { 0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
	int player;
	const int selectionRow = 5;
	int count;
	MRect blobRect;
	
	for( player=0; player<=1; player++ )
	{
		// Set up variables
		role[player] = kChooseDifficulty;
		colorA[player] = RandomBefore(kBlobTypes)+1;
		colorB[player] = kEmpty;
		switch( difficulty[player] )
		{
			case kEasyLevel:      blobX[player] = 1; break;
			case kMediumLevel:    blobX[player] = 2; break;
			case kHardLevel:      blobX[player] = 3; break;
			case kUltraLevel:     blobX[player] = 4; break;
		}
		
		blobY[player] = selectionRow;
		blobR[player] = upRotate;
		blobTime[player] = GameTickCount( ) + (60*8);
		animTime[player] = GameTickCount( );
		shadowDepth[player] = kBlobShadowDepth;
		magic[player] = false;
		grenade[player] = false;
		
		DrawPICTInSurface( boardSurface[player], picSelectDifficulty + backgroundID );
		
		SDLU_AcquireSurface( playerSurface[player] );

		SurfaceDrawBoard( player, &blobBoard );
		
		grid[player][0][selectionRow] = kGray;
		suction[player][0][selectionRow] = kEasyGray;
		charred[player][0][selectionRow] = kNoCharring;
		CalcBlobRect( 0, selectionRow, &blobRect );
		SurfaceDrawBlob( player, &blobRect, kGray, kEasyGray, kNoCharring );
		
		grid[player][kGridAcross-1][selectionRow] = kGray;	
		suction[player][kGridAcross-1][selectionRow] = kHardGray;
		charred[player][kGridAcross-1][selectionRow] = kNoCharring;
		CalcBlobRect( kGridAcross-1, selectionRow, &blobRect );
		SurfaceDrawBlob( player, &blobRect, kGray, kHardGray, kNoCharring );

		CalcBlobRect( 1, selectionRow, &blobRect );
		blobRect.top -= 4; blobRect.bottom += 4;
		blobRect.left += 4; blobRect.right -= 4;
		for( count=1; count<=4; count++ )
		{
			DrawCharacter( count + '0', &blobRect );
			OffsetMRect( &blobRect, kBlobHorizSize, 0 );
		}
		
		SDLU_ReleaseSurface( playerSurface[player] );
		
		DrawSpriteBlobs( player, kNoSuction );
		CleanSpriteArea( player, &blobBoard );
	}
}

void ChooseDifficulty( int player )
{
	MRect blobBoard = { 0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
	const int selectionRow = 5;
	const int difficultyMap[kGridAcross] = {kEasyLevel, kEasyLevel, kMediumLevel, kHardLevel, kUltraLevel, kUltraLevel};
	const int fallingSpeed[kGridAcross] = {0, 15, 9, 7, 4, 0};
	const int startGrays[kGridAcross] = {0, 0,  0, 10, 20, 0};
	const int difficultyFrame[] = { kNoSuction, blobBlinkAnimation, blobBlinkAnimation,
									  blobJiggleAnimation, blobCryAnimation, kNoSuction };
	int oldX = blobX[player];
	
	if( !IsRegistered( ) )
	{
		if( player == 0 )
		{
			HandleDialog( kRegisterDialog );
			QuickFadeOut( NULL );
			showStartMenu = true;
		}	

		return;	
	}
	
	PlayerControl( player );
	if( blobX[player] != oldX ) anim[player] = 0;
	
	UpdateTweak( player, difficultyFrame[blobX[player]] );

	if( GameTickCount( ) >= blobTime[player] )
	{
		if( player == 1 && PICTExists( picBoardRight + backgroundID ) )
		{
			DrawPICTInSurface( boardSurface[player], picBoardRight + backgroundID );
		}
		else
		{
			DrawPICTInSurface( boardSurface[player], picBoard + backgroundID );
		}
		
		SDLU_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &blobBoard );
		SDLU_ReleaseSurface( playerSurface[player] );
		
		CleanSpriteArea( player, &blobBoard );
	
		grid[player][0][selectionRow] = kEmpty;
		grid[player][5][selectionRow] = kEmpty;
												
		suction[player][0][selectionRow] = kNoSuction;
		suction[player][5][selectionRow] = kNoSuction;
		
		difficulty[player]          = difficultyMap[ blobX[player] ];
		character[player].dropSpeed = fallingSpeed[ blobX[player] ];
		unallocatedGrays[player] = lockGrays[player] = startGrays[blobX[player]];
		character[player].hints     = (startGrays[blobX[player]] == 0);
		role[player] = kWaitingToStart;
		
		PlayStereoFrequency( player, kPause, player );
	}
}

const char *gameCredits[][6] = 
{
	{ "Programming", "John Stiles", "", "", "", "" },
	{ "Artwork", "Kate Davis", "Leanne Stiles", "Arnauld de la Grandiere", "Bob Frasure", "Ryan Bliss" },
	{ "Music", "Leanne Stiles", "fmod", "Lizardking", "Armadon, Explizit", "Leviathan, Nemesis" },
	{ "Music", "Jester, Pygmy", "Siren", "Sirrus", "Scaven, FC", "Spring" }, 		  
	{ "Music", "Timewalker", "Jason, Silents", "Chromatic Dragon", "Ng Pei Sin", "" },
	{ "Open Source", "gcc, mingw", "SDL", "libpng", "IJG", "zlib" },
	{ "Special Thanks", "Sam Lantinga", "Carey Lening", "modarchive.com", "digitalblasphemy.com", "" },	  
	{ "Please Register!", "The full version of", "Candy Crisis features", "twelve stages and also", "includes two player", "mode." } 		  
};


void SharewareVictory( void )
{
	SkittlesFontPtr textFont, titleFont;
	SDL_Surface*    backBuffer;
	SDL_Surface*    frontBuffer;
	SDL_Rect        creditSrcSDLRect, bufferSrcSDLRect, bufferDstSDLRect;
	MRect           creditSrcRect = { 0, 0, 369, 150 };
	MRect           bufferSrcRect = { 0, 50, 480, 300 };
	MRect           bufferDstRect = { 0, 0, 480, 250 };
	MPoint          dPoint = { 450, 50 }, lPoint, cPoint;
	int             scroll, ticks, x, y;
	int             delay = 2;
	const char*     text;
	int             thisFade;
	const char fade[120] =   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0
				               1, 2, 3, 4, 5, 6, 7, 8, 9,10, //1
				              11,12,13,14,15,16,17,18,19,20, //2
				              20,20,20,20,20,20,20,20,20,20, //3
				              20,20,20,20,20,20,20,20,20,20, //4
				              20,20,20,20,20,20,20,20,20,20, //5
				              20,20,20,20,20,20,20,20,20,20, //6
				              20,20,20,20,20,20,20,20,20,20, //7
				              20,20,20,20,20,20,20,20,20,20, //8
							  20,19,18,17,16,15,14,13,12,11, //9
							  10, 9, 8, 7, 6, 5, 4, 3, 2, 1, //10
							   0, 0, 0, 0, 0, 0, 0, 0, 0, 0  //11
						     };
	              
	titleFont = GetFont( picFont );
	textFont = GetFont( picTinyFont );
	
	SDLU_MRectToSDLRect( &creditSrcRect, &creditSrcSDLRect );
	SDLU_MRectToSDLRect( &bufferSrcRect, &bufferSrcSDLRect );
	SDLU_MRectToSDLRect( &bufferDstRect, &bufferDstSDLRect );
	
	DrawPICTInSurface( frontSurface, picSharewareVictory );
	SDL_Flip( frontSurface );
	
	backBuffer = SDLU_InitSurface( &bufferDstSDLRect, 16 );
	
	SDLU_BlitSurface( frontSurface, &bufferSrcSDLRect,
	                  backBuffer,   &bufferDstSDLRect   );

	frontBuffer = SDLU_InitSurface( &bufferDstSDLRect, 16 );
	
	QuickFadeIn( NULL );	

	ChooseMusic( 12 ); 
	
	ticks = MTickCount();
	for( scroll=0; scroll<1500; scroll++ )
	{
		SDLU_AcquireSurface( frontBuffer );	
		
		SDLU_BlitSurface( backBuffer,   &bufferDstSDLRect,
		                  frontBuffer,  &bufferDstSDLRect  );
		
		ticks += 3;
		lPoint = dPoint;
		for( y=0; y<8; y++ )
		{
			if( y != 3 && y != 4 )
			{
				cPoint.v = lPoint.v + 25;
				cPoint.h = lPoint.h;
				if( cPoint.v > 480 ) break;
				
				if( cPoint.v > 0 )
				{
					text = gameCredits[y][0];
					thisFade = fade[cPoint.v >> 2];
					
					while( *text )
					{
						SurfaceBlitWeightedCharacter( titleFont, *text++, &cPoint, 31, 31, 0, thisFade );
					}
				}
				
				lPoint.v += 50;
			}
			
			for( x=1; x<6; x++ )
			{
				if( gameCredits[y][x][0] )
				{
					cPoint.v = lPoint.v;
					cPoint.h = lPoint.h + 20;
					if( cPoint.v > 480 ) break;

					if( cPoint.v > 0 )
					{
						text = gameCredits[y][x];
						thisFade = fade[cPoint.v >> 2];
						
						while( *text )
						{
							SurfaceBlitWeightedCharacter( textFont, *text++, &cPoint, 31, 31, 0, thisFade );
						}
					}

					lPoint.v += 20;
				}
			}
		}
		
		SDLU_ReleaseSurface( frontBuffer );

		dPoint.v--;

		SDLU_BlitFrontSurface( frontBuffer, &bufferDstSDLRect, &bufferSrcSDLRect );

		do
		{
			if( SDLU_Button() ) goto out;	
            SDLU_Yield();	
		}
		while( ticks >= MTickCount() );
	}
	
	do
	{
        SDLU_Yield();
	}
	while( !AnyKeyIsPressed( ) && !SDLU_Button() );

out:
	QuickFadeOut( NULL );	
	
	SDL_FreeSurface( backBuffer );
	SDL_FreeSurface( frontBuffer );
}

void RegisteredVictory( void )
{
	SkittlesFontPtr textFont, titleFont, bubbleFont;
	SDL_Surface*    backBuffer;
	SDL_Surface*    frontBuffer;
	MPoint          dPoint[] = { { 230, 340 }, { 230, 30 }, { 230, 30 }, { 30, 30 }, { 30, 340 }, { 230, 340 }, { 230, 30 } }; 
	MPoint          bubblePoint, textPoint, shadowPoint;
	MPoint          setPoint[7][6];
	MPoint          msgSetPoint[7][2];
	long            ticks;
	int             vertScroll, picture, weight, line, minimum;
	int             scrollDir[] = {1, -1, 1, -1, 1, -1, -1};
	int             spacing[] = {40, 19, 19, 19, 23, 19, 23 };
	const char*     text;
	SDL_Rect        fullSDLRect = { 0, 0, 640, 480 };
	SDL_Rect        highSDLRect = { 0, 0, 640, 480 };
	SDL_Rect        lowSDLRect = { 0, 250, 640, 480 };
	SDL_Rect        backBufferSDLRect = { 0, 0, 640, 730 };
	SDL_Rect        scrollSDLRect;

	const char *messages[7][2] =
	{
		{ "Congratulations!", "" },
		{ "You've managed to vaporize all", "of the rampaging candies!" },
		{ "Your quick thinking and sharp", "reflexes have saved the day." },
		{ "", "" },
		{ "", "" },
		{ "", "" },
		{ "Thanks for playing Candy Crisis!", "" },
	};
	
	textFont = GetFont( picFont );
	titleFont = GetFont( picHiScoreFont );
	bubbleFont = GetFont( picBubbleFont );
	
	ChooseMusic( 14 ); 

	for( picture=0; picture<7; picture++ )
	{
		for( line=0; line<2; line++ )
		{
			msgSetPoint[picture][line].v = ((dPoint[picture].v == 230)? 100: 400) + (line * 30);
			msgSetPoint[picture][line].h = 320 - (GetTextWidth( titleFont, messages[picture][line] ) / 2);
		}
		
		for( line=0; line<6; line++ )
		{
			SkittlesFontPtr font;

			if( line == 0 )
			{
				font = titleFont;
				textPoint.v = 45;				
			}
			else
			{
				font = textFont;
				textPoint.v = 65 + (spacing[picture] * line);				
			}

			textPoint.h = (bubbleFont->width['*'] - GetTextWidth( font, gameCredits[picture][line] )) / 2;

			setPoint[picture][line].v = dPoint[picture].v + textPoint.v;
			setPoint[picture][line].h = dPoint[picture].h + textPoint.h;
		}
		
		minimum = 640;
		for( line=1; line<6; line++ )
		{
			if( setPoint[picture][line].h < minimum ) minimum = setPoint[picture][line].h;
		}
		
		for( line=1; line<6; line++ )
		{
			setPoint[picture][line].h = minimum;
		}		
	}
	
	backBuffer  = SDLU_InitSurface( &backBufferSDLRect,  16 );
	frontBuffer = SDLU_InitSurface( &fullSDLRect, 16 );
	
	for( picture = 0; picture<7; picture++ )
	{
		scrollSDLRect = ( scrollDir[picture] > 0 )? highSDLRect: lowSDLRect;
		
		DrawPICTInSurface( backBuffer, picture + picVictory1 );

		SDLU_BlitFrontSurface( backBuffer, &scrollSDLRect, &fullSDLRect );
		
		QuickFadeIn( NULL );
		
		ticks = MTickCount();
		for( vertScroll = 0; vertScroll < 250; vertScroll++ )
		{
			SDLU_AcquireSurface( frontBuffer );
					
			SDLU_BlitSurface( backBuffer,  &scrollSDLRect,
			                  frontBuffer, &fullSDLRect );

			weight = vertScroll - 20;
			for( line=0; line<2; line++ )
			{
				textPoint = msgSetPoint[picture][line];
				shadowPoint.v = textPoint.v + 1;
				shadowPoint.h = textPoint.h + 1;
				
				text = messages[picture][line];
				
				while( *text && weight > 0 )
				{
					int fixedWeight = (weight > 31)? 31: weight;
					
					SurfaceBlitWeightedCharacter( titleFont, *text, &shadowPoint, 0,  0,  0,  fixedWeight );
					SurfaceBlitWeightedCharacter( titleFont, *text, &textPoint,   31, 31, 31, fixedWeight );
					weight--;
					text++;
				}
			}
			
			bubblePoint = dPoint[picture];
			weight = ( vertScroll <= 210 )? vertScroll - 10: 241 - vertScroll;
			if( weight < 0  ) weight = 0;
			if( weight > 31 ) weight = 31;
			
			if( weight > 0 )
			{
				SurfaceBlitWeightedCharacter( bubbleFont, '*', &bubblePoint, 31, 31, 31, (weight+1)>>1 );
				
				for( line=0; line<6; line++ )
				{
					SkittlesFontPtr font = (line == 0)? titleFont: textFont;
					
					textPoint = setPoint[picture][line];
					text = gameCredits[picture][line];
					
					while( *text )
					{
						SurfaceBlitWeightedCharacter( font, *text++, &textPoint, 0, 0, 0, weight );
					}
				}
			}
			
			SDLU_ReleaseSurface( frontBuffer );
			
			SDLU_BlitFrontSurface( frontBuffer, &fullSDLRect, &fullSDLRect );
			                       
			scrollSDLRect.y += scrollDir[picture];
			
			ticks += 4;
			do
			{ 
				if( SDLU_Button() ) vertScroll = 250;
			    SDLU_Yield();
			}			
			while( ticks >= MTickCount() );
		}

		QuickFadeOut( NULL );
	}
	
	SDL_FreeSurface( backBuffer  );
	SDL_FreeSurface( frontBuffer );
}

void TotalVictory( void )
{
	
	AddHiscore( score[0] );
	QuickFadeOut( NULL );	

	DoFullRepaint = NoPaint;

	if( !IsRegistered() )
	{
		SharewareVictory( );
	}
	else
	{
		RegisteredVictory( );
	}

	showStartMenu = true;
}
