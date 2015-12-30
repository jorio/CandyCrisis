// hiscore.c

#include "stdafx.h"
#include "SDLU.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "main.h"
#include "gworld.h"
#include "graphics.h"
#include "score.h"
#include "hiscore.h"
#include "keyselect.h"
#include "font.h"
#include "blitter.h"
#include "random.h"
#include "pause.h"
#include "level.h"
#include "tutorial.h"
#include "graymonitor.h"
#include "players.h"
#include "gameticks.h"
#include "music.h"
#include "soundfx.h"

using std::min;

Combo defaultBest = 
{
	/*bestGrid[kGridAcross][kGridDown] = */ 
	{ { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,                          1, 1, 1, 2, 2 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,           1, 1 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty } },

	/*bestA      = */ 2,
	/*bestB      = */ 2,
	/*bestM      = */ false, 
	/*bestG      = */ false, 
	/*bestLv     = */ kTutorialLevel,
	/*bestX      = */ 1,
	/*bestR      = */ upRotate,
	/*bestPlayer = */ 0,
	/*bestValue  = */ (40*1) + (50*9),
	/*bestName   = */ "Tutorial"
};

Combo best = 
{
	/*bestGrid[kGridAcross][kGridDown] = */ 
	{ { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,                          1, 1, 1, 2, 2 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,           1, 1 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty } },

	/*bestA      = */ 2,
	/*bestB      = */ 2,
	/*bestM      = */ false, 
	/*bestG      = */ false, 
	/*bestLv     = */ kTutorialLevel,
	/*bestX      = */ 1,
	/*bestR      = */ upRotate,
	/*bestPlayer = */ 0,
	/*bestValue  = */ (40*1) + (50*9),
	/*bestName   = */ "Tutorial"
};

Combo evenBetter = {0};
Combo potentialCombo[2];

AutoPattern hiScorePattern[] =
{
	{ kIdleTicks,          60, 0,   NULL },	
	{ kBlockUntilDrop,     0,  0,   NULL },	
	{ kBlockUntilComplete, 0,  0,   NULL },	
	{ kIdleTicks,          60, 0,   NULL },	
	{ kComplete,           0,  0,   NULL }
};

HighScore scores[10] = 
{	
	{"Leviathan", 40000},
	{"Dr. Crisis", 36000},
	{"Angel", 32000},
	{"Spike", 28000},
	{"Fox", 24000},
	{"Raguel", 20000},
	{"Kumo", 16000},
	{"Patty", 12000},
	{"Yuurei", 8000},
	{"Glurp", 4000}  
};

HighScore defaultScores[10] = 
{	
	{"Leviathan", 40000},
	{"Dr. Crisis", 36000},
	{"Angel", 32000},
	{"Spike", 28000},
	{"Fox", 24000},
	{"Raguel", 20000},
	{"Kumo", 16000},
	{"Patty", 12000},
	{"Yuurei", 8000},
	{"Glurp", 4000}  
};

char highScoreName[256];
const char *highScoreText;
const char *highScoreRank;

static void FadeScreen( SDL_Surface* hiScoreSurface, SDL_Surface* fadeSurface, int start, int end )
{
	int       skip, timer, frame, fade, color, direction, fadeStart, fadeEnd;
	SDL_Rect  destSDLRect;
	SDL_Rect  fullSDLRect = { 0, 0, 640, 480 };
	int       black;
	
	black = SDL_MapRGB( fadeSurface->format, 0, 0, 0 );
	
	if( start < end )
	{
		direction = 1;
		fadeStart = 0;
		fadeEnd = 32;
	}
	else
	{
		direction = -1;
		fadeStart = 32;
		fadeEnd = 0;
	}
	
	skip = 1;
	timer = MTickCount( ) + 1;
	while( timer > MTickCount() ) { }

	for( frame = start; (direction>0)? (frame <= end): (frame >= end); frame += direction )
	{
		MRect drawRect = {0, 0, 15, 640};
		timer += skip;
		
		for( fade = fadeStart; fade != fadeEnd; fade += direction )
		{
			color = frame + fade;
			if( color <  0 ) color = 0;
			if( color > 31 ) color = 31;
			
			SDLU_MRectToSDLRect( &drawRect, &destSDLRect );

			switch( color )
			{
				case 0:
					SDLU_BlitSurface( hiScoreSurface, &destSDLRect,
					                  fadeSurface,    &destSDLRect  ); 
					break;
			
				case 31:
					SDL_FillRect( fadeSurface, &destSDLRect, black );
					break;
				
				default:
					SurfaceBlitColorOver( hiScoreSurface,  fadeSurface,
					                      &drawRect,       &drawRect,
					                       0, 0, 0, _5TO8(color) );
					break;
			}
			
			OffsetMRect( &drawRect, 0, 15 );
		}

		SDLU_BlitFrontSurface( fadeSurface, &fullSDLRect, &fullSDLRect );
        SDLU_Present();

		if( timer <= MTickCount( ) )
		{
			skip = 2;
		}
		else
		{
			skip = 1;
			while( timer > MTickCount( ) )
			{
				SDLU_Yield();
			}
		}
	}
}

void ShowHiscore( void )
{
	short            count, length;
	char             myString[256];
	int              stringLength;
	SDL_Surface*     hiScoreSurface;
	SDL_Surface*     fadeSurface;
	SDL_Rect         fullSDLRect = { 0, 0, 640, 480 };
	SkittlesFontPtr  font;
	SDL_Color        anyColor;
	MPoint           dPoint;
	const char*      highScores = "HIGH SCORES";
	int              r, g, b;
	
	if (DeleteKeyIsPressed())
	{
		// If the user holds delete while opening the high scores,
		// clear the high score table.
		
		memcpy( &scores, &defaultScores, sizeof( scores ) );
		memcpy( &best,   &defaultBest,   sizeof( best   ) );
	}
	
	hiScoreSurface = LoadPICTAsSurface( picBackdrop + (100 * RandomBefore(kLevels)), 32 );
	fadeSurface    = SDLU_InitSurface( &fullSDLRect, 32 );

	font = GetFont( picHiScoreFont );

	SDLU_AcquireSurface( hiScoreSurface );
		
	SDLU_GetPixel( hiScoreSurface, RandomBefore( fullSDLRect.w ), RandomBefore( fullSDLRect.h ), &anyColor );

	anyColor.r = min( 255, anyColor.r + 112 );
	anyColor.g = min( 255, anyColor.g + 112 );
	anyColor.b = min( 255, anyColor.b + 112 );

	dPoint.v = 20;
	dPoint.h = 225;
	for( count=0; highScores[count]; count++ )
	{
		SurfaceBlitCharacter( font, highScores[count], &dPoint, 255, 255, 255, 1 );
	}
	
	for( count=0; count<=9; count++ )
	{
		r = ((255 * (10-count)) + (anyColor.r * count)) / 10;
		g = ((255 * (10-count)) + (anyColor.g * count)) / 10;
		b = ((255 * (10-count)) + (anyColor.b * count)) / 10;
		
		dPoint.v = 75 + count * 38;
		dPoint.h = 85;
		
		if( count<9 )
		{
			SurfaceBlitCharacter( font, count + '1', &dPoint, r, g, b, 1 );
		}
		else
		{
			SurfaceBlitCharacter( font, '1', &dPoint, r, g, b, 1 );
			SurfaceBlitCharacter( font, '0', &dPoint, r, g, b, 1 );
		}
		
		SurfaceBlitCharacter( font, '.', &dPoint, r, g, b, 1 );
		SurfaceBlitCharacter( font, ' ', &dPoint, r, g, b, 1 );

		length = 0;
		while( scores[count].name[length] && dPoint.h < 430 )
		{
			SurfaceBlitCharacter( font, scores[count].name[length++], &dPoint, r, g, b, 1 );
		}
		
		while( dPoint.h < 450 )
		{
			SurfaceBlitCharacter( font, '.', &dPoint, r, g, b, 1 );
		}
		
		dPoint.h = 470;

		stringLength = sprintf( myString, "%d", scores[count].score );
		for( length=0; length < stringLength; length++ )
		{
			SurfaceBlitCharacter( font, myString[length], &dPoint, r, g, b, 1 );
		}
	}
	
	SDLU_ReleaseSurface( hiScoreSurface );
	
	SDL_FillRect( g_frontSurface, &g_frontSurface->clip_rect, SDL_MapRGB( g_frontSurface->format, 0, 0, 0 ));
    SDLU_Present();
	
	SDLU_SetBrightness( 1.0f );	

	FadeScreen( hiScoreSurface, fadeSurface, 31, -32 );
	do
	{
	}
	while( !AnyKeyIsPressed( ) && !SDLU_Button() );
	FadeScreen( hiScoreSurface, fadeSurface, -31, 32 );
	
	SDLU_SetBrightness( 0.0f );	

	SDL_FreeSurface( hiScoreSurface );
	SDL_FreeSurface( fadeSurface );
}

void InitPotentialCombos( void )
{
	memset( &potentialCombo[0], 0, sizeof(Combo) );
	memset( &potentialCombo[1], 0, sizeof(Combo) );
	potentialCombo[0].player = 0;
	potentialCombo[1].player = 1;
}

void SubmitCombo( Combo *in )
{
	if( in->value > best.value && in->value > evenBetter.value )
	{
		PlayMono( kContinueSnd );
		memcpy( &evenBetter, in, sizeof( Combo ) );
	}	
}

// Please note: This function may well be the biggest kludge in Candy Crisis.
// It relies on tons of insider knowledge. But it works.
void ShowBestCombo( void )
{
	SkittlesFontPtr font;
	const char *bestCombo = "BEST COMBO";
    const char *scan;
    char bestInfo[256];
	MPoint dPoint;
	int levelCap;
	
	font = GetFont( picHiScoreFont );
	
	StopBalloon( );
	InitGame( kAutoControl, kNobodyControl );
	scoreWindowVisible[0] = false;
	grayMonitorVisible[0] = false;

	level = best.lv;
	levelCap = kLevels;
	if( (level < 1 || level > levelCap) && level != kTutorialLevel ) 
	{
		memcpy( &best, &defaultBest, sizeof(best) );
		showStartMenu = true;
		return;
	}

	BeginRound( false );
	character[0].dropSpeed = 12;
	
	SDLU_AcquireSurface( backdropSurface );
	
	dPoint.v = 40;
	dPoint.h = 225;
	for( scan = bestCombo; *scan; scan++ )
	{
		SurfaceBlitCharacter( font, *scan, &dPoint, 255, 255, 255, 1 );
	}
		
	sprintf( bestInfo, "%s (%d points)", best.name, best.value );
	dPoint.v = 410;
	dPoint.h = 320 - (GetTextWidth( font, bestInfo ) / 2);

	for( scan = bestInfo; *scan; scan++ )
	{
		SurfaceBlitCharacter( font, *scan, &dPoint, 255, 255, 255, 1 );
	}

	SDLU_ReleaseSurface( backdropSurface );
	
	memcpy( grid[0], best.grid, kGridAcross * kGridDown );
	ResolveSuction( 0 );
	RedrawBoardContents( 0 );
	RefreshAll( );

	nextA[0] = best.a;
	nextB[0] = best.b;
	nextG[0] = best.g;
	nextM[0] = best.m;
	RetrieveBlobs( 0 );

	EraseSpriteBlobs( 0 );
	blobX[0] = best.x;
	blobR[0] = best.r;
	DrawSpriteBlobs( 0, kNoSuction );
	
	QuickFadeIn( NULL );
	blobTime[0] = animTime[0] = GameTickCount( );

	autoPattern = hiScorePattern;	
	tutorialTime = 0;
}

void AddHiscore( int score )
{
	short count, item;
	char rank[50];
    char text[256];
    const char *playerName = "You";
    const char *twoPlayerNames[] = { "Player 1", "Player 2" };
	int highScoreLevel = -1;
	

	// Check for high score
	// (only do high scores if it's player vs CPU)
			
	if( players == 1 &&
	    control[0] == kPlayerControl &&
	    control[1] == kAIControl        )
	{		
		for( count=0; count<=9; count++ )
		{
			if( score >= scores[count].score )
			{				
				sprintf( rank, "%d points", score );
				highScoreLevel = count;
				break;
			}
		}
	}
	
	// Determine player's name for best combo

	if( players == 2 ) playerName = twoPlayerNames[evenBetter.player];


	// See if both best combo AND high score
		
	if( evenBetter.value > best.value && highScoreLevel != -1 )
	{
		
		sprintf( text, "You got a high score and the best combo!" );

		highScoreText = text;
		highScoreRank = rank;

		HandleDialog( kHiScoreDialog );

		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			memcpy( &best, &evenBetter, sizeof(Combo) );
			strcpy( best.name, highScoreName );

			for( item=8; item>=highScoreLevel; item-- )
			{
				memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
			}
			
			scores[highScoreLevel].score = score;
			strcpy( scores[highScoreLevel].name, highScoreName );				
		}
	}
	
	// See if best combo has been won
		
	else if( evenBetter.value > best.value )
	{
		sprintf( text, "Congratulations! %s got best combo!", playerName );
		
		highScoreText = text;
		highScoreRank = "";
				
		HandleDialog( kHiScoreDialog );
		
		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			memcpy( &best, &evenBetter, sizeof(Combo) );
			strcpy( best.name, highScoreName );
		}
	}

	// See if high score has been won
	
	else if( highScoreLevel != -1 )
	{
		highScoreText = "Congratulations! You got a high score!";
		highScoreRank = rank;

		HandleDialog( kHiScoreDialog );
		
		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			for( item=8; item>=highScoreLevel; item-- )
			{
				memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
			}
			
			scores[highScoreLevel].score = score;
			strcpy( scores[highScoreLevel].name, highScoreName );				
		}
	}
}
