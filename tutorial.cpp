// tutorial.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "tutorial.h"
#include "level.h"
#include "font.h"
#include "pause.h"
#include "gworld.h"
#include "graphics.h"
#include "control.h"
#include "blitter.h"
#include "gameticks.h"
#include "soundfx.h"
#include "opponent.h"
#include "keyselect.h"

#include <string.h>
#include <stdio.h>

AutoPattern tutorialPattern[] =
{
	{ kMessage,        0,   0,   "Welcome to the\nCandy Crisis\ntutorial!" },
	{ kIdleTicks,      180, 0,   NULL },
	{ kMessage,        0,   0,   "I'll be your guide\nthroughout the\ntutorial. Let's\nget started!" },
	{ kRetrieve,       1,   1,   NULL },
	{ kIdleTicks,      240, 0,   NULL },	
	{ kMessage,        0,   0,   "When you start the\ngame, you'll find a\npair of candies\nfalling from the sky." },
	{ kIdleTicks,      240, 0,   NULL },	
	{ kMessage,        0,   0,   "Your goal is to\nkeep these candies\nfrom overflowing the\nboard!" },
	{ kBlockUntilLand, 0,   0,   NULL },
	{ kRetrieve,       2,   2,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kMessage,        0,   0,   "You control the\ncandy by moving\nthem left and right." },
	{ kIdleTicks,      120, 0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kIdleTicks,      120, 0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kMessage,        0,   0,   "Press '~~'\nto make the pieces\nmove left." },
	{ kRetrieve,       3,   3,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kPosition,       5,   0,   NULL },
	{ kIdleTicks,      90,  0,   NULL },
	{ kMessage,        0,   0,  "Use '||' to\nmake the pieces\nmove right." },
	{ kIdleTicks,      180, 0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kMessage,        0,   0,   "You can also\nmake the pieces\nrotate around each\nother." },
	{ kRetrieve,       4,   3,   NULL },
	{ kIdleTicks,      180, 0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kMessage,        0,   0,   "Press '{{'\nto make the pieces\nrotate." },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       5,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kMessage,        0,   0,   "Also, '``'\ncauses the candy\nto drop faster." },
	{ kIdleTicks,      180, 0,   NULL },	
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "To pause the game\nor adjust settings,\npress 'esc.'" },
	{ kIdleTicks,      280, 0,   NULL },	
	{ kMessage,        0,   0,   "The candy in\nthis game is made\nfrom a highly\nunstable substance!" },
	{ kRetrieve,       2,   2,   NULL },
	{ kIdleTicks,      200, 0,   NULL },
	{ kMessage,        0,   0,   "So when four\npieces of the same\ncolor come into\ncontact..." },
	{ kPosition,       1,   0,   NULL },
	{ kIdleTicks,      180, 0,   NULL },
	{ kMessage,        0,   0,   "... they vaporize!" },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kIdleTicks,      60,  0,   NULL },
	{ kMessage,        0,   0,   "Let's see that\nonce again." },
	{ kRetrieve,       1,   1,   NULL },
	{ kIdleTicks,      120, 0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "Pop!" },
	{ kIdleTicks,      120, 0,   NULL },	
	{ kMessage,        0,   0,   "You can even get\nfive or more\npieces to pop\nall at the same\ntime!" },
	{ kRetrieve,       4,   4,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kPosition,       4,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       4,   4,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kPosition,       3,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       4,   4,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       4,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "Pop!\n\nTechniques like this\ncan earn lots of\nbonus points." },
	{ kIdleTicks,      180, 0,   NULL },
	{ kMessage,        0,   0,   "You can also pop\nmore than one color\nat once." },
	{ kRetrieve,       5,   5,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       4,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       6,   5,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       3,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       3,   5,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       5,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kMessage,        0,   0,   "All right!" },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       0,   6,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kMessage,        0,   0,   "You can even set\nup devastating\nchain reactions..." },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       0,   6,   NULL },
	{ kIdleTicks,      90,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       6,   6,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       1,   0,   NULL },
	{ kMessage,        0,   0,   "... like this!" },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kMessage,        0,   0,   "Tricky, isn't it?" },
	{ kRetrieve,       1,   2,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       1,   2,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       3,   1,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       3,   2,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       3,   0,   NULL },
	{ kMessage,        0,   0,  "Let's see one more\nexample of a chain\nreaction." },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kRetrieve,       1,   2,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       5,   5,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       1,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       5,   3,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kPosition,       1,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       3,   5,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       3,   3,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kIdleTicks,      20,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kIdleTicks,      90,  0,   NULL }, 
	{ kMessage,        0,   0,   "There's one more\nthing you need to\nknow about..." },
	{ kIdleTicks,      180, 0,   NULL },
	{ kMessage,        0,   0,   "Watch out for the\nsee-through candy!" },
	{ kIdleTicks,      60,  0,   NULL },
	{ kPunish,         18,  0,   NULL },
	{ kRetrieve,       1,   1,   NULL },
	{ kIdleTicks,      120, 0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kIdleTicks,      20,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "When your opponent\nvaporizes a group of\ncandies, they also\nsend some transparent\npieces to you!" },
	{ kRetrieve,       1,   1,   NULL },
	{ kIdleTicks,      90,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kPosition,       4,   0,   NULL },
	{ kIdleTicks,      30,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       1,   1,   NULL },
	{ kIdleTicks,      60,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      90,  0,   NULL },
	{ kMessage,        0,   0,   "You can get rid of\nthese pieces by\nvaporizing something\nnext to them." },
	{ kIdleTicks,      150, 0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kIdleTicks,      60,  0,   NULL },
	{ kRetrieve,       1,   2,   NULL },
	{ kMessage,        0,   0,   "There are also\nsome bonus items\nwhich can come\nin handy." },
	{ kIdleTicks,      20,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kIdleTicks,      20,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       3,   1,   NULL },
	{ kIdleTicks,      20,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kIdleTicks,      20,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       1,   3,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kPosition,       1,   0,   NULL },
	{ kIdleTicks,      15,  0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "One of these bonus\nitems is the Crazy\nCandy!" },
	{ kRetrieve,       -1,  2,   NULL },
	{ kIdleTicks,      15,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      10,  0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      100, 0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "The Crazy Candy\ntakes on the\ncolor of one of\nits neighbors." },
	{ kIdleTicks,      60,  0,   NULL },
	{ kRetrieve,       3,   4,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kPosition,       4,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       1,   5,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kPosition,       1,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kMessage,        0,   0,   "Another useful bonus\nitem is the bomb!" },
	{ kRetrieve,       2,   3,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kPosition,       0,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       0,   3,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       3,   2,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kPosition,       5,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       4,   3,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kPosition,       3,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve,       2,   3,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kSpin,           0,   0,   NULL },
	{ kIdleTicks,      5,   0,   NULL },
	{ kPosition,       5,   0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },	
	{ kRetrieve, kBombBottom, kBombTop, NULL },
	{ kMessage,        0,   0,   "When the bomb lands\non one color\nof candy, all\npieces of that\ncolor will\nvaporize!" },
	{ kIdleTicks,      100, 0,   NULL },
	{ kPosition,       4,   0,   NULL },
	{ kIdleTicks,      120, 0,   NULL },
	{ kBlockUntilDrop, 0,   0,   NULL },
	{ kIdleTicks,      200, 0,   NULL },
	{ kMessage,        0,   0,   "Now you're ready\nto play Candy Crisis.\nGood luck!" },
	{ kIdleTicks,      270, 0,   NULL },

	{ kComplete,       0,   0,   NULL }
};

MRect           balloonRect = {0, 0, 190, 210};
SkittlesFontPtr balloonFont;
MPoint          balloonPt;
char*           balloonChar;
char            balloonMsg[256];
int             balloonTime, tutorialTime;
SDL_Surface*    balloonSurface = NULL;

void InitTutorial( void )
{
	// Balloon font
	balloonFont = GetFont( picBalloonFont );
	
	// Balloon backbuffer
	if( balloonSurface == NULL )
	{
		SDL_Rect surfaceRect = { 0, 0, backdropSurface->w, backdropSurface->h }; 
		balloonSurface = SDLU_InitSurface( &surfaceRect, 32 );
	}
	
	// Set up auto pattern
	autoPattern = tutorialPattern;	
	tutorialTime = 0;
}

void EndTutorial( void )
{
	QuickFadeOut( NULL );
	
	showStartMenu = true;
}

static int CalculateBalloonWidth( char *message )
{
	int maxWidth = 40;
	int currentWidth = 0;
	
	for( ;; )
	{
		char in = *message++;
		
		switch(in)
		{
			case 0:
				return (currentWidth > maxWidth)? currentWidth: maxWidth;
				
			case '\n':
				maxWidth = (currentWidth > maxWidth)? currentWidth: maxWidth;
				currentWidth = 0;
				break;
			
			default:
				currentWidth += balloonFont->width[in];
				break;
		}
	}
}

static int CalculateBalloonHeight( char *message )
{
	int lines = 2;
	char *scan = message;
	
	while( *scan ) lines += (*scan++ == '\n');

	return lines * 20;
}

void StopBalloon( void )
{
	balloonTime = 0x7FFFFFFF;
}

void StartBalloon( const char *message )
{
	MPoint      balloonTip, balloonFill;
	int         replace;
	const char* match[] = { "~~", "||", "``", "{{" };
	char*       search;
	SDL_Rect    balloonSDLRect, balloonContentsSDLRect;
	MRect       balloonContentsRect;
	
	strcpy( balloonMsg, message );
	for( replace=0; replace<4; replace++ )
	{
		search = strstr( balloonMsg, match[replace] );
		if( search )
		{
			char temp[256];
			
			search[0] = '%';
			search[1] = 's';
			sprintf( temp, balloonMsg, SDL_GetKeyName( playerKeys[1][replace] ) );
			strcpy( balloonMsg, temp );
		}
	}
	
	// Erase previous balloons
	SDLU_MRectToSDLRect( &balloonRect, &balloonSDLRect );
	SDLU_BlitFrontSurface( backdropSurface, &balloonSDLRect, &balloonSDLRect );

	// Draw empty balloon outline
	SDLU_AcquireSurface( balloonSurface );

	balloonRect.left = balloonRect.right - 25 - CalculateBalloonWidth ( balloonMsg );
	balloonRect.top = balloonRect.bottom - 25 - CalculateBalloonHeight( balloonMsg );

	SDLU_MRectToSDLRect( &balloonRect, &balloonSDLRect );
	SDLU_BlitSurface( backdropSurface, &balloonSDLRect,
	                  balloonSurface,  &balloonSDLRect  );
	
	balloonContentsRect = balloonRect;
	balloonContentsRect.bottom -= 25;
		
	SurfaceGetEdges( balloonSurface, &balloonContentsRect );
	SDL_FillRect( balloonSurface, 
				  SDLU_MRectToSDLRect( &balloonContentsRect, &balloonContentsSDLRect ), 
				  SDL_MapRGB( balloonSurface->format, 0xFF, 0xFF, 0xFF ) );
	SurfaceCurveEdges( balloonSurface, &balloonContentsRect );
	
	balloonTip.v = balloonContentsRect.bottom - 2;
	balloonTip.h = balloonContentsRect.right - 40;
	balloonFill = balloonTip;

	SurfaceBlitCharacter( balloonFont, '\x01', &balloonFill,  0,   0,   0,  0 );
	SurfaceBlitCharacter( balloonFont, '\x02', &balloonTip, 255, 255, 255,  0 );
	
	SDLU_ReleaseSurface( balloonSurface );

	// Blit empty balloon to screen
	SDLU_MRectToSDLRect( &balloonRect, &balloonSDLRect );
	SDLU_BlitFrontSurface( balloonSurface, &balloonSDLRect, &balloonSDLRect );
	
	balloonPt.h = balloonRect.left + 10;
	balloonPt.v = balloonRect.top + 10;
	balloonChar = balloonMsg;
	balloonTime = GameTickCount( );
	
	OpponentChatter( true );
}

void UpdateBalloon( void )
{
	SDL_Rect balloonSDLRect;
	
	if( control[0] != kAutoControl ) return;
	if( GameTickCount() < balloonTime ) return;
	
	if( balloonChar )
	{
		char in = *balloonChar++;
				
		switch( in )
		{
			case 0:
				OpponentChatter( false );
				balloonChar = NULL;
				balloonTime += 120;
				break;
				
			case '\n':
				balloonPt.h = balloonRect.left + 10;
				balloonPt.v += 20;
				break;
				
			default:
				if( balloonFont->width[in] > 0 )
				{
					SDLU_AcquireSurface( balloonSurface );
					SurfaceBlitCharacter( balloonFont, in, &balloonPt, 0, 0, 0, 0 );
					SDLU_ReleaseSurface( balloonSurface );
					
					SDLU_MRectToSDLRect( &balloonRect, &balloonSDLRect );
					SDLU_BlitFrontSurface( balloonSurface, &balloonSDLRect, &balloonSDLRect );

					balloonTime += 2;
				}
				break;			
		}	
	}
	else
	{
		SDLU_MRectToSDLRect( &balloonRect, &balloonSDLRect );
		SDLU_BlitFrontSurface( backdropSurface, &balloonSDLRect, &balloonSDLRect );
		                    
		StopBalloon();
	}
}
