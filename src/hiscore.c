// hiscore.c

#include "SDLU.h"

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
	MTicks    timer;
	int       skip, frame, fade, color, direction, fadeStart, fadeEnd;
	SDL_Rect  destSDLRect;
	SDL_Rect  fullSDLRect = { 0, 0, 640, 480 };
	int       black;
	
	black = SDL_MapSurfaceRGB( fadeSurface, 0, 0, 0 );
	
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
					SDL_FillSurfaceRect( fadeSurface, &destSDLRect, black );
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
		
		SDL_memcpy( &scores, &defaultScores, sizeof( scores ) );
		SDL_memcpy( &best,   &defaultBest,   sizeof( best   ) );
	}
	
	hiScoreSurface = LoadPICTAsSurface( picBackdrop + (100 * RandomBefore(kLevels)), 32 );
	fadeSurface    = SDLU_InitSurface( &fullSDLRect, 32 );


	SDLU_AcquireSurface( hiScoreSurface );
		
	SDLU_GetPixel( hiScoreSurface, RandomBefore( fullSDLRect.w ), RandomBefore( fullSDLRect.h ), &anyColor );

	anyColor.r = MinInt( 255, anyColor.r + 112 );
	anyColor.g = MinInt( 255, anyColor.g + 112 );
	anyColor.b = MinInt( 255, anyColor.b + 112 );

	dPoint.v = widescreen? 100: 20;
	dPoint.h = 225;
	font = GetFont( picHiScoreFont );
	for( count=0; highScores[count]; count++ )
	{
		SurfaceBlitCharacter( font, highScores[count], &dPoint, 255, 255, 255, 1 );
	}
	
    font = GetFont(widescreen ? picFont : picHiScoreFont);
	for( count=0; count<=9; count++ )
	{
		r = ((255 * (10-count)) + (anyColor.r * count)) / 10;
		g = ((255 * (10-count)) + (anyColor.g * count)) / 10;
		b = ((255 * (10-count)) + (anyColor.b * count)) / 10;
		
		if (widescreen)
			dPoint.v = 150 + count * 24;
		else
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

		stringLength = SDL_snprintf( myString, sizeof(myString), "%d", scores[count].score );
		for( length=0; length < stringLength; length++ )
		{
			SurfaceBlitCharacter( font, myString[length], &dPoint, r, g, b, 1 );
		}
	}
	
	SDLU_ReleaseSurface( hiScoreSurface );
	
	SDL_FillSurfaceRect( g_frontSurface, &g_frontSurfaceClipRect, SDL_MapSurfaceRGB(g_frontSurface, 0, 0, 0) );
    SDLU_Present();
	
	SDLU_SetBrightness( 1.0f );	

	FadeScreen( hiScoreSurface, fadeSurface, 31, -32 );
	do
	{
	}
	while( !AnyKeyIsPressed( ) && !SDLU_Button() );
	FadeScreen( hiScoreSurface, fadeSurface, -31, 32 );
	
	SDLU_SetBrightness( 0.0f );	

	SDL_DestroySurface( hiScoreSurface );
	SDL_DestroySurface( fadeSurface );
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
		SDL_memcpy( &evenBetter, in, sizeof( Combo ) );
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
	
	StopBalloon( );
	InitGame( kAutoControl, kNobodyControl );
	scoreWindowVisible[0] = false;
	grayMonitorVisible[0] = false;

	level = best.lv;
	levelCap = kLevels;
	if( (level < 1 || level > levelCap) && level != kTutorialLevel ) 
	{
		SDL_memcpy( &best, &defaultBest, sizeof(best) );
		showStartMenu = true;
		return;
	}

	BeginRound( false );
	character[0].dropSpeed = 12;
	
	SDLU_AcquireSurface( backdropSurface );
	
	font = GetFont(picHiScoreFont);
	dPoint.v = widescreen? 70: 40;
	dPoint.h = 225;
	for( scan = bestCombo; *scan; scan++ )
	{
		SurfaceBlitCharacter( font, *scan, &dPoint, 255, 255, 255, 1 );
	}

	SDL_snprintf( bestInfo, sizeof(bestInfo), "%s (%d points)", best.name, best.value );

    font = GetFont(widescreen ? picFont : picHiScoreFont);
	dPoint.v = widescreen? 388: 410;
	dPoint.h = 320 - (GetTextWidth( font, bestInfo ) / 2);

	for( scan = bestInfo; *scan; scan++ )
	{
		SurfaceBlitCharacter( font, *scan, &dPoint, 255, 255, 255, 1 );
	}

	SDLU_ReleaseSurface( backdropSurface );
	
	SDL_memcpy( grid[0], best.grid, kGridAcross * kGridDown );
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
				SDL_snprintf( rank, sizeof(rank), "%d points", score );
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
		
		SDL_snprintf( text, sizeof(text), "You got a high score and the best combo!" );

		highScoreText = text;
		highScoreRank = rank;

		HandleDialog( kHiScoreDialog );

		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			SDL_memcpy( &best, &evenBetter, sizeof(Combo) );
			SDL_strlcpy( best.name, highScoreName, sizeof(best.name) );

			for( item=8; item>=highScoreLevel; item-- )
			{
				memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
			}
			
			scores[highScoreLevel].score = score;
			SDL_strlcpy( scores[highScoreLevel].name, highScoreName, sizeof(scores[highScoreLevel].name) );
		}
	}
	
	// See if best combo has been won
		
	else if( evenBetter.value > best.value )
	{
		SDL_snprintf( text, sizeof(text), "Congratulations! %s got best combo!", playerName );
		
		highScoreText = text;
		highScoreRank = "";
				
		HandleDialog( kHiScoreDialog );
		
		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			SDL_memcpy( &best, &evenBetter, sizeof(Combo) );
			SDL_strlcpy( best.name, highScoreName, sizeof(best.name) );
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
				SDL_memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
			}
			
			scores[highScoreLevel].score = score;
			SDL_strlcpy( scores[highScoreLevel].name, highScoreName, sizeof(scores[highScoreLevel].name) );
		}
	}
}
