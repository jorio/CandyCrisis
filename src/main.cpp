// main.c

//
//                    CANDY CRISIS 32-BIT - OS X and Windows
//
// FEATURES:
// • Runs natively on SDL 2.0, for both Intel OS X and Windows. Targeting
//   popular Windows installs in 2015 (i.e. Win7/8/10).
// • Now runs in 32-bit color.
//
// ETC:
// • Codebase should once again be almost entirely platform-neutral.
//

//
//                    CANDY CRISIS SDL 2.0
//
// FEATURES:
// • Runs natively on SDL 2.0 and Intel OS X. Targeting modern OS X,
//   circa 2015 (e.g. OS X 10.10).
//
// ETC:
// • Removed registration.
// • Resources moved into app bundle.
// • Migrated to SDL 2 and SDL_image 2 (for graphics--also removes
//   subdependencies on libjpeg/libpng)
//

//
//                      CANDY CRISIS SDL
//
// FEATURES:
// • Runs natively on any platform with support for SDL. Developed
//   primarily around the lame Mac Carbon SDL. Can compile and run
//   with gcc/mingw32 on Windows and gcc on Linux.
//
// ETC:
// • Game functionality unchanged.
// • Removed all Mac OS-specific concepts from the game, i.e.:
//   - GWorlds replaced with SDL_Surfaces.
//   - Rects and Points replaced with MRects and MPoints. (same contents)
// • All resources moved into external files in a folder called
//   "CandyCrisisResources." Graphics are all JPG and PNG, opened
//   with SDL_image. Sounds are WAV. Music is still MOD type.
// • Using the following Open Source projects:
//   SDL, SDL_image (for graphics)
//   libjpeg (used by SDL_image)
//   libpng (used by SDL_image)
//   zlib (used by libpng)
// • Using fmod for sound; replaces MikMod
// • Wrote a utility library, SDLU, to pick up slack in the SDL
//   implementation and to help SDL mesh with a Mac-centric universe.
// • New registration code algorithm based on a fast string hash.
//

//
//                      CANDY CRISIS X
//
// FEATURES:
// • Runs natively on Mac OS X. Developed around Mac OS X
// Public Beta 1H39 and 2E14.
// Updated 3/25/2001 for Mac OS X 4K78--OS X 10.0.
// Updated 8/25/2001 for Mac OS X 10.0.4.
//
// ETC:
// • Game functionality unchanged.
// • Zerius Sound System scrapped, replaced with LibMikMod. I'm
// very unhappy with performance relative to ZSS, but on G3s and up,
// performance should not be a major concern. If only I could get
// the source to ZSS so it could be Carbonized! 
// • Everything runs in one window now, instead of having
// one window per interface element. This was necessary 
// because OS X wanted to put drop shadows around everything
// and it looked pretty weird. This was also a personal pet
// peeve that I never had the motivation to fix until now.
// • Controls dialog is super Aqua savvy, using Theme Text 
// and Theme Buttons.
// • A couple of kludges added, to work around OS X bugs. Ugh.
// • Found bug which was causing blitter to draw larger dirty rects
// than necessary (top/left of dirty rect was always 0/0). Not sure 
// if it ever shipped like that or if this is something I changed 
// post-Candy Crisis 1.0.
// • Changed cursor management since OS X cursors don't know how to
// hide and show themselves properly.
//
// UNRESOLVED:
// • Stopped getting Out of Memory reports. I wonder if any of the
// Candy Crisis cleanups affected this...?
// • OS X displays a line of garbage when you try to put up a totally
// blank cursor. I'm not going to spend too long analyzing this; it's
// not my bug.
// 


//                    CANDY CRISIS 1.0 UPDATE
//
// FEATURES:
// • Rebranded "Candy Crisis" at the request of Mars Candy Co.
// Many, many graphical changes as a result. (New logo thanks 
// to Bob Frasure.)
// • "Controls" button in main menu per many user requests.
// • Slightly improved error reporting. 
// • Option-key at startup to turn on "allow background tasks" 
// or "don't change resolutions." (Don't change resolutions
// requires DrawSprocket 1.7.)
//
// NONCRITICAL:
// • Fixed bug in 2P mode where game would say "Player 1 got
//  best combo!" when Player 2 really got it, and vice versa. 
// • Changed Magic Skittle ratio to 1/19 instead of 1/17, after
// watching Brett get tons of Magic Skittles at work. Hmm.
// • Replaced RandomBefore with less hacked-up code, because
// Magic Skittles STILL seemed to be coming up more often than
// expected. That seemed to take care of it.
// • Fixed rare bug where, after losing, the game would sometimes
// get stuck until you explicitly chose "end game." (Would manifest
// more often on a slow computer and/or when Background Tasks were
// activated.)
//
// ETC:
// • Antipiracy measures.
// • Game fonts all loaded at startup time instead of dynamically,
// in an attempt to reduce the number of GWorlds which are created,
// then torn down, during the game (which could have been potentially
// fragmenting the heap, though I doubt this was a real problem).
//
// UNRESOLVED:
// • Still a handful of people who get Out of Memory when they 
// try to pause a game. Damn. Hopefully now I'll at least know
// where they're dying (though I highly suspect it's InitGWorld,
// which without a stack crawl is pretty much useless info...)
// One guy says this is fixed by deleting prefs. Huh??
//

//
//                     2.0.2 UPDATE
//
// FEATURES:
// • When you continue, your score is now rolled back to what
// it was when you first started the round. This prevents people
// from racking up high scores by continuing over and over again
// on the highest board.
// • You can now clear the high score tables to their default
// values by holding delete while clicking "high scores" on the
// main screen.
//
// ETC:
// • Added small picture to the controls dialog so people know
// which color is Player 1, and which is Player 2.
// • Holding option while warping causes a CPU/CPU match to
// occur.
// • Changed in-game registration URL to:
// http://emulation.net/s2.com/register.html
//
// CRITICAL:
// • Fixed minor memory corruption when a bomb hits floor or
// gray Skittle. Could potentially have corrupted 3 tiles of
// opponent's board.
// • Fixed bug where potential combo data would not be cleared
// when choosing "End Game" and then starting a new game, which
// led to really weird corruptions of potential combo data.
// • Fixed bug where holding down button after end-credits rolled
// would cause the pause dialog to pop up on a zero-gamma screen
// (whoops).
// 
// NONCRITICAL:
// • Fixed bug where dropping bomb would not display associated
// points.
// • Fixed bug where dropping bomb on floor/gray Skittle would
// reward the player for "killing" empty squares, making it
// score 100x(9-number of grays in 3x3 area) as opposed to
// the correct 100x(number of blobs in 3x3 area).
// • Occasionally, when Best Combo got corrupted, it would show
// the ending credits instead of displaying the Best Combo. Now
// there is code to ensure that the level # of the Best Combo 
// structure is in bounds. (If it isn't, the best combo is
// assumed to be corrupt, and it is deleted. Not the most 
// optimal solution, but what can I do?)
// • If you started the tutorial, ended it, then viewed the best
// combo, you'd see a speech balloon appear for one frame.
// • Fixed bug where bringing up InputSprocket dialog would 
// unload ics8's used to draw key caps (damn InputSprocket bugs).
// Does this only affect ISp < 1.7?
// • Fixed bug where bringing up InputSprocket dialog would not
// update game windows behind it after it got closed.
//
// UNRESOLVED:
// • Slow loading time issue seems to only be affecting an
// incredible minority of people. It's being caused by QuickTime
// decompressing JPEGs. I think it's not a Skittles 2 issue.
// • One guy says if he quits the game, reopens it, starts a game,
// then pauses it, he gets an Out of Memory condition. He's running
// 8.6-D clean. Hmm.
//

//
//                     2.0.1 UPDATE
//
// FEATURES:
// • Best Combo
// • New bg for level 8
// • New sfx for continue sound (requested by Nathan Lamont)
//
// ETC:
// • High score dialog enhanced to support Best Combo stuff
// • Tutorial suggests pressing esc to set up keys now
// • More aggressive AI for intellect>18 (does not percieve
// 1-level zap as advantageous)
//
// CRITICAL:
// • Fix for out-of-bounds array read (->crash) inside 
// ZapScoreDisplay.
// • Fixed bug where falling Skittles (in DropBlobs) would
// occasionally have their bottom half lopped off. Tough to
// see while in motion but totally obvious in screenshots.
// • Fixed InputSprocket icons in ISpConfigure dialog
// • Workaround for System 7 bug, where setting the 
// cursor while gamma is faded causes solid black cursor.
//
// NONCRITICAL:
// • Fixed bug where char fading on blobs that were
// in motion would cause crap to appear for one frame.
// Only apparent on slow Macs or in screenshots. Otherwise
// appeared as flicker and easily dismissed.
// • Cmd-tab inside High Scores or Game Over screen
// no longer leaves a white box (empty window) open
// and will not switch out with 0 gamma
// • Fixed DrawSprocket bug on Macs that can't do 640x480
// (i.e. PowerBook G3 is stuck at 1024x768). The window
// would be drawn in lower-right hand corner instead of
// centered.
// • Fixed bug in multipliers for getting multiple 
// colors at once. (Should have been *3/*6/*12/*24, was
// actually *3/*9/*21/*45! Ouch!)
// • Hitting cmd-Q at high score dialog would allow
// empty high score name to be added to high score table.
//
// UNRESOLVED:
// • A few users report very slow loading times between
// levels and during loading sequence. Problem tends to
// be alleviated by turning on VM. Can't reproduce here.
// 

//
//                 2.0.0 INITIAL RELEASE
//

#include "stdafx.h"

#if _WIN32
#include <windows.h>
#include <io.h> // for _chdir
#endif
#if __APPLE__
#include <Cocoa/Cocoa.h>
#endif

#include "SDLU.h"

#include "main.h"

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "hiscore.h"
#include "control.h"
#include "players.h"
#include "gworld.h"
#include "graphics.h"
#include "grays.h"
#include "soundfx.h"
#include "next.h"
#include "random.h"
#include "victory.h"
#include "score.h"
#include "graymonitor.h"
#include "music.h"
#include "gameticks.h"
#include "level.h"
#include "opponent.h"
#include "keyselect.h"
#include "blitter.h"
#include "prefs.h"
#include "tweak.h"
#include "zap.h"
#include "pause.h"
#include "tutorial.h"


SDL_Renderer* g_renderer;
SDL_Window*   g_window;
SDL_Texture*  g_windowTexture;
SDL_Surface*  g_frontSurface;
signed char  nextA[2], nextB[2], nextM[2], nextG[2], colorA[2], colorB[2],
	         blobX[2], blobY[2], blobR[2], blobSpin[2], speed[2], role[2], halfway[2],
	         control[2], dropping[2], magic[2], grenade[2], anim[2];
int          chain[2];
int          blobTime[2], startTime, endTime;
MBoolean     finished = false, pauseKey = false, showStartMenu = true;
signed char  grid[2][kGridAcross][kGridDown], suction[2][kGridAcross][kGridDown], charred[2][kGridAcross][kGridDown], glow[2][kGridAcross][kGridDown];
MRect        playerWindowZRect, playerWindowRect[2];
MBoolean     playerWindowVisible[2] = { true, true };
KeyList      hitKey[2];
int          backgroundID = -1;
MPoint       blobWindow[8][2];
void         (*DoFullRepaint)() = NoPaint;
MBoolean     needsRefresh = false;

static char  candyCrisisResources[512];

#if _WIN32
int WinMain(
	_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPSTR,
	_In_ int)
#else
int main(int, char *[])
#endif
{
	Initialize( );	

	LoadPrefs( );
	
	ReserveMonitor( );	
	ShowTitle( );

	ChooseMusic( 13 );
	
	while (!finished)
	{
		if (showStartMenu)
		{
			GameStartMenu();
			showStartMenu = false;
		}
		
		if (finished)
            break;
        
        DoFullRepaint = NeedRefresh;
        CheckKeys( );
        HandlePlayers( );
        UpdateOpponent( );
        UpdateBalloon( );
        UpdateSound( );
        DoFullRepaint = NoPaint;
        
        if (needsRefresh)
        {
            RefreshAll();
            needsRefresh = false;
        }
        
        SDLU_Present();

        if (!showStartMenu && pauseKey)
        {
            FreezeGameTickCount( );
            PauseMusic( );
            MaskRect( &playerWindowRect[0] );
            MaskRect( &playerWindowRect[1] );
            WaitForRelease( );
            
            HandleDialog( kPauseDialog );
                            
            WaitForRelease( );
            RefreshPlayerWindow( 0 );
            RefreshPlayerWindow( 1 );
            ResumeMusic( );
            UnfreezeGameTickCount( );
		}
	}
	
	SavePrefs( );
	ReleaseMonitor( );
	
	return 0;
}

void NoPaint( void )
{
}

void MaskRect( MRect *r )
{
	SDL_Rect sdlRect;
	SDLU_MRectToSDLRect( r, &sdlRect );
	SDLU_BlitFrontSurface( backdropSurface, &sdlRect, &sdlRect );
}

void RefreshPlayerWindow( short player )
{
	MRect fullUpdate = {0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
	
	if( control[player] == kNobodyControl )
	{
		MaskRect( &playerWindowRect[player] );
	}
	else
	{
		SetUpdateRect( player, &fullUpdate );
		UpdatePlayerWindow( player );
	}
}

void NeedRefresh()
{
	needsRefresh = true;
}

void RefreshAll( void )
{	
	DrawBackdrop( );

	ShowGrayMonitor( 0 );
	ShowGrayMonitor( 1 );

	RefreshNext( 0 );
	RefreshNext( 1 );

	RefreshPlayerWindow( 0 );
	RefreshPlayerWindow( 1 );

	DrawFrozenOpponent( );
	DrawStage( );

	ShowScore( 0 );
	ShowScore( 1 );
}

void Error( const char* extra )
{
#if _WIN32
	char error[1024];
	sprintf_s(error, "Sorry, a critical error has occurred. Please report the following error message:\n    %s", extra );
	MessageBox(NULL, error, "Candy Crisis", MB_OK | MB_ICONERROR);
 #else
 	fprintf(stderr, "Sorry, a critical error has occurred. Please report the following error message:\n    %s", extra );
#endif
    abort();
}

void WaitForRelease( void )
{	
	do
	{
		SDLU_Yield();
	}
	while( AnyKeyIsPressed( ) || SDLU_Button() );
}

MBoolean AnyKeyIsPressed( void )
{
	int arraySize;
    const Uint8* pressedKeys;
    
    SDLU_PumpEvents();
    pressedKeys = SDL_GetKeyboardState( &arraySize );
    
	for (int index = 0; index < arraySize; index++)
	{
		if (pressedKeys[index] &&
		    index != SDL_SCANCODE_CAPSLOCK &&
            index != SDL_SCANCODE_NUMLOCKCLEAR &&
            index != SDL_SCANCODE_SCROLLLOCK)
        {
            return true;
        }
	}

    return false;
}

MBoolean ControlKeyIsPressed( void )
{
    int arraySize;
    const Uint8* pressedKeys;
    
    SDLU_PumpEvents();
    pressedKeys = SDL_GetKeyboardState( &arraySize );
    
	return pressedKeys[SDL_SCANCODE_LCTRL] || pressedKeys[SDL_SCANCODE_RCTRL];
}

MBoolean OptionKeyIsPressed( void )
{
    int arraySize;
    const Uint8* pressedKeys;
    
    SDLU_PumpEvents();
    pressedKeys = SDL_GetKeyboardState( &arraySize );
    
    return pressedKeys[SDL_SCANCODE_LALT] || pressedKeys[SDL_SCANCODE_RALT];
}

MBoolean DeleteKeyIsPressed( void )
{
    int arraySize;
    const Uint8* pressedKeys;
    
    SDLU_PumpEvents();
    pressedKeys = SDL_GetKeyboardState( &arraySize );
    
    return pressedKeys[SDL_SCANCODE_BACKSPACE];
}

void RetrieveResources( void )
{
	InitSound( );				

	InitBackdrop( );			

	GetBlobGraphics( );			
	
	InitNext( );				
	
	InitScore( );				

	InitGrayMonitors( );		
	
	InitOpponent( );

	InitStage( );   // must run after backdrop window is open
	InitGameTickCount( );

	InitPlayers( ); // must run after backdrop window is open
	InitFont( ); 
	InitZapStyle( );// must run after fonts are inited
	
	InitBlitter( ); // must run after player windows are open
	InitPlayerWorlds( );
	
	InitVictory( );	// must run after fonts are inited			
	InitTweak( );
}


void CenterRectOnScreen( MRect *rect, double locationX, double locationY )
{
	MPoint dest = {0,0};
	
	dest.h = (short)(locationX * (640 - (rect->right - rect->left)));
	dest.h &= ~3;
	dest.v = (short)(locationY * (480 - (rect->bottom - rect->top)));

	OffsetMRect( rect, -rect->left, -rect->top );
	OffsetMRect( rect, dest.h, dest.v );
}

void ReserveMonitor( void )
{
	SDL_ShowCursor( SDL_DISABLE );
	
//    SDL_CreateWindowAndRenderer(640, 480, 0, &g_window, &g_renderer);
    SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &g_window, &g_renderer);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    SDL_RenderSetLogicalSize(g_renderer, 640, 480);

    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderPresent(g_renderer);
    
    g_frontSurface = SDL_CreateRGBSurface(0, 640, 480, 32,
                                          RED_MASK, GREEN_MASK, BLUE_MASK, 0);
    
    g_windowTexture = SDL_CreateTexture(g_renderer,
                                        SDL_PIXELFORMAT_RGB888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        640, 480);
}

void ReleaseMonitor( void )
{
}

int Warp( void )
{
	return 8;
}

const char* QuickResourceName( const char* prefix, int id, const char* extension )
{
	static char name[1024];
	if (id)
	{
		sprintf( name, "%s%s_%d%s", candyCrisisResources, prefix, id, extension );
	}
	else
	{
		sprintf( name, "%s%s%s", candyCrisisResources, prefix, extension );
	}
	
	return name;
}

void Initialize(void)
{
#if _WIN32
    HMODULE module;
	char    name[MAX_PATH + 1], *lastBackslash;

	module = GetModuleHandle(NULL);
	GetModuleFileName(module, name, MAX_PATH);
	lastBackslash = strrchr(name, '\\');
	if (lastBackslash != NULL)
    {
        *lastBackslash = '\0';
		strcpy(candyCrisisResources, name);
		strcat(candyCrisisResources, "\\CandyCrisisResources\\");
    }
#endif
#if __APPLE__
    sprintf(candyCrisisResources, "%s/", [[[NSBundle mainBundle] resourcePath] UTF8String]);
#endif
#ifdef linux
	strcpy(candyCrisisResources, "CandyCrisisResources/");
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		Error("SDL_Init failed");
	}

	int desiredFormats = (IMG_INIT_JPG | IMG_INIT_PNG);
	int haveFormats = IMG_Init(desiredFormats);
	if ((haveFormats & desiredFormats) != desiredFormats)
	{
		Error("IMG_Init failed");
	}

	atexit(IMG_Quit);
	atexit(SDL_Quit);

    SDLU_Init();
}

void QuickFadeIn( MRGBColor *color )
{
/*	(void) color; // is unused

	int   c;
	float percent;

	for( percent=0.0f; percent<1.0f; percent += 0.04f )
	{
		c = MTickCount( ); 
		SDLU_SetBrightness( percent );
		while( c == MTickCount( ) ) 
		{  
			SDLU_Yield(); 
		}
	}
	
	SDLU_SetBrightness( percent );*/
    SDL_Delay(200);
}

void QuickFadeOut( MRGBColor *color )
{/*
	(void) color; // is unused

	int    c;
	float  percent;

	for( percent=1.0f; percent>0.0f; percent -= 0.04f )
	{
		c = MTickCount( ); 
		SDLU_SetBrightness( percent );
 		while( c == MTickCount( ) )
 		{
 			SDLU_Yield(); 
 		}
	}
	
	SDLU_SetBrightness( percent );*/
    SDL_Delay(200);
}

MBoolean FileExists( const char* name )
{
	FILE* f = fopen( name, "rb" );
	if( f == NULL )
	{
		return false;
	}
	
	fclose( f );
	return true;
}


void WaitForRegainFocus()
{
    do
    {  
        SDLU_PumpEvents();
        SDL_Delay(50);
    }
    while( !SDLU_IsForeground() );    

	DoFullRepaint();
}


