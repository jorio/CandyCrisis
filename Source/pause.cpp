// pause.cpp

// All of this code is fugly. I really needed a dialog manager, but I didn't know it at the time,
// and instead I cobbled this together. It is just barely good enough to work. Fortunately it looks
// decent to the end user...


#include "SDL.h"
#include "SDLU.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"
#include "gameticks.h"
#include "blitter.h"
#include "graphics.h"
#include "gworld.h"
#include "pause.h"
#include "random.h"
#include "font.h"
#include "music.h"
#include "soundfx.h"
#include "keyselect.h"
#include "level.h"
#include "victory.h"
#include "hiscore.h"
#include "score.h"
#include "RegAlgorithm.h"


const char kEscapeKey = 0x1B;

typedef struct
{
	float red, green, blue;
}
FRGBColor;

SDL_Surface* backSurface;
SDL_Surface* drawSurface;
SDL_Surface* logoSurface;
SDL_Surface* logoMaskSurface;
SDL_Surface* logoAlphaSurface;

SkittlesFontPtr smallFont, bigFont, dashedLineFont, continueFont, tinyFont, batsuFont;
FRGBColor backColor[4];
MBoolean continueTimeOut;

static int dialogType, dialogStage, dialogTimer, dialogUndimTime, dialogTarget, dialogShade, dialogItem;
static float colorWrap = 0, colorInc;
static MRect logoRect = {0, 0, 111, 246}, lastPauseRect;
static MBoolean dialogStageComplete;
static MBoolean timeToRedraw = false;

// for the controls dialog
static int controlToReplace = -1;

// for the enter code dialog
static char  nameField[256], keyField[256];
static char* whichField = nameField;
static int   batsuAlpha = 0;

static void ItsTimeToRedraw()
{
	timeToRedraw = true;
}

enum
{
	kTextRainbow,
	kTextBrightRainbow,
	kTextWhite,
	kTextBlueGlow,
	kTextGray,
	kTextAlmostWhite
};

static MPoint DrawRainbowText( SkittlesFontPtr font, const char *line, MPoint dPoint, float wave, int bright )
{
	int   length, current;
	int   r,g,b;
	float s;
	
	current = 0;
	length = strlen(line);
	
	switch( bright )
	{	
			case kTextGray:
				r = g = b = 12;
				break;
				
			case kTextBlueGlow:
				s = sin(wave);
				r = (int)(11.0 + 15.0 * s * s);
				g = r;
				b = 31;
				break;
				
			case kTextWhite:
				r = g = b = 31;
				break;
				
			case kTextAlmostWhite:
				r = g = b = 28;
				break;
				
	}

	while( line[current] )
	{
		switch( bright )
		{
			case kTextBrightRainbow:
				r = (int)(26.0 + 5.0 * sin(wave                    ));
				g = (int)(26.0 + 5.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(26.0 + 5.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;

			case kTextRainbow:
				r = (int)(16.0 + 12.0 * sin(wave                    ));
				g = (int)(16.0 + 12.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(16.0 + 12.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;
		}

		SurfaceBlitCharacter( font, line[current], &dPoint, r, g, b, 1 );			
		
		wave += 0.2;
		current++;
	}
	
	return dPoint;
}


#define kEdgeSize 8
static short edge[4][kEdgeSize][kEdgeSize];

void SurfaceGetEdges( SDL_Surface* edgeSurface, const MRect *rect )
{
	unsigned char* src[4];
	int            srcRowBytes, width, height, count;
	
	src[0] = src[1] = src[2] = src[3] = (unsigned char*) edgeSurface->pixels;
	srcRowBytes = edgeSurface->pitch;

	width  = rect->right  - rect->left;
	height = rect->bottom - rect->top; 

	src[0] += (srcRowBytes * (rect->top               )) + ((rect->left             ) * 2);
	src[1] += (srcRowBytes * (rect->top               )) + ((rect->right - kEdgeSize) * 2);
	src[2] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->left             ) * 2);
	src[3] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->right - kEdgeSize) * 2);
	
	for( count=0; count<4; count++ )
	{
		for( height=0; height<kEdgeSize; height++ )
		{
			memcpy( edge[count][height], src[count], kEdgeSize * 2 );
			src[count] += srcRowBytes;
		}
	}
}


void SurfaceCurveEdges( SDL_Surface* edgeSurface, const MRect *rect )
{
	unsigned char* src[4];
	int srcRowBytes, width, height, count;
	char edgeMap[4][kEdgeSize][kEdgeSize+1]={  "      --",
					                           "    -...",
					                           "   -.xxX",
					                           "  -.xXXX",
					                           " -.xXXXX",
					                           " .xXXXXX",
					                           "-.xXXXXX",
					                           "-.XXXXXX",
					                           "--      ",
					                           "...-    ",
					                           "Xxx.-   ",
					                           "XXXx.-  ",
					                           "XXXXx.- ",
					                           "XXXXXx. ",
					                           "XXXXXx.-",
					                           "XXXXXX.-",
					                           "-.XXXXXX",
					                           "-.xXXXXX",
					                           " .xXXXXX",
					                           " -.xXXXX",
					                           "  -.xXXX",
					                           "   -.xxX",
					                           "    -...",
					                           "      --",
					                           "XXXXXX.-",
					                           "XXXXXx.-",
					                           "XXXXXx. ",
					                           "XXXXx.- ",
					                           "XXXx.-  ",
					                           "Xxx.-   ",
					                           "...-    ",
					                           "--      "  };
	                         	                         
	
	src[0] = src[1] = src[2] = src[3] = (unsigned char*) edgeSurface->pixels;
	srcRowBytes = edgeSurface->pitch;

	src[0] += (srcRowBytes * (rect->top               )) + ((rect->left             ) * 2);
	src[1] += (srcRowBytes * (rect->top               )) + ((rect->right - kEdgeSize) * 2);
	src[2] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->left             ) * 2);
	src[3] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->right - kEdgeSize) * 2);
	
	// Draw top/bottom border
	{
		short *srcT1 = (short*) (src[0]) + kEdgeSize;
		short *srcB1 = (short*) (src[2] + (srcRowBytes*(kEdgeSize-1))) + kEdgeSize;
		short *srcT2 = srcT1 + (srcRowBytes/2);
		short *srcB2 = srcB1 - (srcRowBytes/2);
		
		for( width = rect->right - rect->left - (kEdgeSize * 2); width > 0; width-- )
		{
			*srcT1 = 0; srcT1++;
			*srcB1 = 0; srcB1++;
			*srcT2 = (*srcT2 >> 1) & 0x3DEF; srcT2++;
			*srcB2 = (*srcB2 >> 1) & 0x3DEF; srcB2++;
		}
	}
	
	// Draw left/right border
	{
		unsigned char *srcL1 = (src[0] + (srcRowBytes * kEdgeSize));
		unsigned char *srcR1 = (src[1] + (srcRowBytes * kEdgeSize)) + 2*(kEdgeSize-1);

		unsigned char *srcL2 = srcL1 + 2;
		unsigned char *srcR2 = srcR1 - 2;
		
		for( height = rect->bottom - rect->top - (kEdgeSize * 2); height > 0; height-- )
		{
			*(short*)srcL1 = 0; 
			*(short*)srcR1 = 0;
			*(short*)srcL2 = (*(short*)srcL2 >> 1) & 0x3DEF; 
			*(short*)srcR2 = (*(short*)srcR2 >> 1) & 0x3DEF; 
			
			srcL1 += srcRowBytes; 
			srcR1 += srcRowBytes;
			srcL2 += srcRowBytes; 
			srcR2 += srcRowBytes;
		}
	}
		
	// Draw curved edges
	for( count=0; count<4; count++ )
	{
		short *srcS = (short*) src[count];
		
		for( height=0; height<kEdgeSize; height++ )
		{
			for( width=0; width<kEdgeSize; width++ )
			{
				switch( edgeMap[count][height][width] )
				{
					case ' ': 	*srcS = edge[count][height][width]; break;
					case '.': 	*srcS = 0; break;
					case 'x': 	*srcS = (*srcS >> 1) & 0x3DEF; break;
					case '-':	*srcS = (edge[count][height][width] >> 1) & 0x3DEF; break;
					case 'X': 	break;
				}
				srcS++;
			}
			srcS += (srcRowBytes / 2) - kEdgeSize;
		}
	}
}

static MBoolean SharewareNoticeIsStillWaiting()
{
	return MTickCount() < dialogUndimTime;
}


#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))
#define arrsize(x) (sizeof(x)/sizeof(x[0]))

enum
{
	kOpening = 0, 
	kClosing
};

static MBoolean DrawDialogBox( MBoolean larger, int animationType, int *target, int skip, float *colorWrap, float colorInc, MRect *pauseRect )
{
	MBoolean animationStageComplete = false;
	MRect normalRect[2][19]     = { { { 240 - 10,  320 - 30,  240 + 10,  320 + 30  },
	                                  { 240 - 40,  320 - 120, 240 + 40,  320 + 120 },
	                                  { 240 - 60,  320 - 180, 240 + 60,  320 + 180 },
	                                  { 240 - 70,  320 - 210, 240 + 70,  320 + 210 },
	                                  { 240 - 80,  320 - 230, 240 + 80,  320 + 230 },
	                                  { 240 - 88,  320 - 245, 240 + 88,  320 + 245 },
	                                  { 240 - 95,  320 - 252, 240 + 95,  320 + 252 },
	                                  { 240 - 101, 320 - 255, 240 + 101, 320 + 255 },
	                                  { 240 - 106, 320 - 252, 240 + 106, 320 + 252 },
	                                  { 240 - 110, 320 - 245, 240 + 110, 320 + 245 },
	                                  { 240 - 113, 320 - 238, 240 + 113, 320 + 238 },
	                                  { 240 - 115, 320 - 232, 240 + 115, 320 + 232 },
	                                  { 240 - 116, 320 - 228, 240 + 116, 320 + 228 },
	                                  { 240 - 118, 320 - 232, 240 + 118, 320 + 230 },
	                                  { 240 - 118, 320 - 238, 240 + 118, 320 + 232 },
	                                  { 240 - 119, 320 - 242, 240 + 119, 320 + 242 },
	                                  { 240 - 119, 320 - 244, 240 + 119, 320 + 244 },
	                                  { 240 - 119, 320 - 242, 240 + 119, 320 + 242 },
	                                  { 240 - 120, 320 - 240, 240 + 120, 320 + 240 }  },
	                                { { 240 - 110, 320 - 220, 240 + 110, 320 + 220 }, 
	                                  { 240 - 105, 320 - 210, 240 + 105, 320 + 210 }, 
	                                  { 240 - 100, 320 - 200, 240 + 100, 320 + 200 }, 
	                                  { 240 - 95,  320 - 190, 240 + 95,  320 + 190 }, 
	                                  { 240 - 90,  320 - 180, 240 + 90,  320 + 180 }, 
	                                  { 240 - 85,  320 - 170, 240 + 85,  320 + 170 }, 
	                                  { 240 - 80,  320 - 160, 240 + 80,  320 + 160 }, 
	                                  { 240 - 75,  320 - 150, 240 + 75,  320 + 150 }, 
	                                  { 240 - 70,  320 - 140, 240 + 70,  320 + 140 }, 
	                                  { 240 - 65,  320 - 130, 240 + 65,  320 + 130 }, 
	                                  { 240 - 60,  320 - 120, 240 + 60,  320 + 120 }, 
	                                  { 240 - 55,  320 - 110, 240 + 55,  320 + 110 }, 
	                                  { 240 - 50,  320 - 100, 240 + 50,  320 + 100 }, 
	                                  { 240 - 45,  320 - 90,  240 + 45,  320 + 90  }, 
	                                  { 240 - 40,  320 - 80,  240 + 40,  320 + 80  }, 
	                                  { 240 - 35,  320 - 70,  240 + 35,  320 + 70  }, 
	                                  { 240 - 30,  320 - 60,  240 + 30,  320 + 60  }, 
	                                  { 240 - 25,  320 - 50,  240 + 25,  320 + 50  }, 	                                
	                                  { 240 - 20,  320 - 40,  240 + 20,  320 + 40  }  }
	                              };

	MRect largerRect[2][19]     = { { { 240 - 11,  320 - 30,  240 + 11,  320 + 30  },
	                                  { 240 - 44,  320 - 120, 240 + 44,  320 + 120 },
	                                  { 240 - 66,  320 - 180, 240 + 66,  320 + 180 },
	                                  { 240 - 77,  320 - 210, 240 + 77,  320 + 210 },
	                                  { 240 - 88,  320 - 230, 240 + 88,  320 + 230 },
	                                  { 240 - 97,  320 - 245, 240 + 97,  320 + 245 },
	                                  { 240 - 104, 320 - 252, 240 + 104, 320 + 252 },
	                                  { 240 - 111, 320 - 255, 240 + 111, 320 + 255 },
	                                  { 240 - 117, 320 - 252, 240 + 117, 320 + 252 },
	                                  { 240 - 121, 320 - 245, 240 + 121, 320 + 245 },
	                                  { 240 - 124, 320 - 238, 240 + 124, 320 + 238 },
	                                  { 240 - 126, 320 - 232, 240 + 126, 320 + 232 },
	                                  { 240 - 128, 320 - 228, 240 + 128, 320 + 228 },
	                                  { 240 - 130, 320 - 232, 240 + 130, 320 + 230 },
	                                  { 240 - 130, 320 - 238, 240 + 130, 320 + 232 },
	                                  { 240 - 131, 320 - 242, 240 + 131, 320 + 242 },
	                                  { 240 - 131, 320 - 244, 240 + 131, 320 + 244 },
	                                  { 240 - 131, 320 - 242, 240 + 131, 320 + 242 },
	                                  { 240 - 132, 320 - 240, 240 + 132, 320 + 240 }  },
	                                { { 240 - 121, 320 - 220, 240 + 121, 320 + 220 }, 
	                                  { 240 - 115, 320 - 210, 240 + 115, 320 + 210 }, 
	                                  { 240 - 110, 320 - 200, 240 + 110, 320 + 200 }, 
	                                  { 240 - 104, 320 - 190, 240 + 104, 320 + 190 }, 
	                                  { 240 - 99,  320 - 180, 240 + 99,  320 + 180 }, 
	                                  { 240 - 93,  320 - 170, 240 + 93,  320 + 170 }, 
	                                  { 240 - 88,  320 - 160, 240 + 88,  320 + 160 }, 
	                                  { 240 - 82,  320 - 150, 240 + 82,  320 + 150 }, 
	                                  { 240 - 77,  320 - 140, 240 + 77,  320 + 140 }, 
	                                  { 240 - 71,  320 - 130, 240 + 71,  320 + 130 }, 
	                                  { 240 - 66,  320 - 120, 240 + 66,  320 + 120 }, 
	                                  { 240 - 60,  320 - 110, 240 + 60,  320 + 110 }, 
	                                  { 240 - 55,  320 - 100, 240 + 55,  320 + 100 }, 
	                                  { 240 - 49,  320 - 90,  240 + 49,  320 + 90  }, 
	                                  { 240 - 44,  320 - 80,  240 + 44,  320 + 80  }, 
	                                  { 240 - 38,  320 - 70,  240 + 38,  320 + 70  }, 
	                                  { 240 - 33,  320 - 60,  240 + 33,  320 + 60  }, 
	                                  { 240 - 27,  320 - 50,  240 + 27,  320 + 50  }, 	                                
	                                  { 240 - 22,  320 - 40,  240 + 22,  320 + 40  }  }
	                              };

	int      colorInt, shading;
	float    colorFrac, nColorFrac;
	MRect    newRect;
	SDL_Rect sdlRect;
	
	if( *target > 18 )
	{
		*target = 18;
		animationStageComplete = true;
	}

	colorInt  = (int) floor( *colorWrap );
	colorFrac = *colorWrap - colorInt;

	newRect = larger? largerRect[animationType][*target]: normalRect[animationType][*target];
	shading = ((animationType == 0) ? (*target * 24 / 18): (24 - (*target * 2 / 3)));
	
	{
		float r1 = backColor[colorInt      ].red, g1 = backColor[colorInt      ].green, b1 = backColor[colorInt      ].blue,
		      r2 = backColor[(colorInt+1)&3].red, g2 = backColor[(colorInt+1)&3].green, b2 = backColor[(colorInt+1)&3].blue,
		      r3 = backColor[(colorInt+2)&3].red, g3 = backColor[(colorInt+2)&3].green, b3 = backColor[(colorInt+2)&3].blue,
		      r4 = backColor[(colorInt+3)&3].red, g4 = backColor[(colorInt+3)&3].green, b4 = backColor[(colorInt+3)&3].blue;
		
		nColorFrac = 1 - colorFrac;
		
		SDLU_AcquireSurface( drawSurface );
		
		SurfaceBlitBlendOver(  backSurface,  drawSurface, 
		                      &newRect,     &newRect,
  		                       (int)((r1 * nColorFrac) + (r2 * colorFrac)), 
						       (int)((g1 * nColorFrac) + (g2 * colorFrac)), 
						       (int)((b1 * nColorFrac) + (b2 * colorFrac)), 
						       (int)((r2 * nColorFrac) + (r3 * colorFrac)), 
						       (int)((g2 * nColorFrac) + (g3 * colorFrac)), 
						       (int)((b2 * nColorFrac) + (b3 * colorFrac)), 
						       (int)((r4 * nColorFrac) + (r1 * colorFrac)), 
						       (int)((g4 * nColorFrac) + (g1 * colorFrac)), 
						       (int)((b4 * nColorFrac) + (b1 * colorFrac)), 
						       (int)((r3 * nColorFrac) + (r4 * colorFrac)), 
						       (int)((g3 * nColorFrac) + (g4 * colorFrac)), 
						       (int)((b3 * nColorFrac) + (b4 * colorFrac)), 
						       shading );

		if( pauseRect->left < newRect.left ) 
		{
			MRect eraseRect = *pauseRect;
			pauseRect->left = eraseRect.right = newRect.left;
			
			SDLU_MRectToSDLRect( &eraseRect, &sdlRect );
			SDLU_BlitSurface( backSurface, &sdlRect,
			                  drawSurface, &sdlRect  );
		}

		if( pauseRect->right > newRect.right ) 
		{
			MRect eraseRect = *pauseRect;
			pauseRect->right = eraseRect.left = newRect.right;
			
			SDLU_MRectToSDLRect( &eraseRect, &sdlRect );
			SDLU_BlitSurface( backSurface, &sdlRect,
			                  drawSurface, &sdlRect  );
		}

		if( pauseRect->top < newRect.top ) 
		{
			MRect eraseRect = *pauseRect;
			pauseRect->top = eraseRect.bottom = newRect.top;

			SDLU_MRectToSDLRect( &eraseRect, &sdlRect );
			SDLU_BlitSurface( backSurface, &sdlRect,
			                  drawSurface, &sdlRect  );
		}

		if( pauseRect->bottom > newRect.bottom ) 
		{
			MRect eraseRect = *pauseRect;
			pauseRect->bottom = eraseRect.top = newRect.bottom;

			SDLU_MRectToSDLRect( &eraseRect, &sdlRect );
			SDLU_BlitSurface( backSurface, &sdlRect,
			                  drawSurface, &sdlRect  );
		}

		SDLU_ReleaseSurface( drawSurface );
	}
	
	*pauseRect = newRect;
	
	*colorWrap += colorInc * skip;
	if( *colorWrap >= 4 ) *colorWrap -= 4;

	*target += skip;

	return animationStageComplete;
}

static void DrawDialogCursor( MRect *pauseRect, int *shade )
{
	MPoint p, q;
    shade;
    
	SDLU_GetMouse( &p );
	
	if( p.h < (pauseRect->left      ) ) p.h = pauseRect->left;
	if( p.h > (pauseRect->right  - 5) ) p.h = pauseRect->right  - 5;
	if( p.v < (pauseRect->top       ) ) p.v = pauseRect->top;
	if( p.v > (pauseRect->bottom - 5) ) p.v = pauseRect->bottom - 5;
	q = p;
	
	SDLU_AcquireSurface( drawSurface );

	SurfaceBlitCharacter( smallFont, '°', &p,  0,  0,  0, 0 );			
	SurfaceBlitCharacter( smallFont, '¢', &q, 31, 31, 31, 0 );			
	
	SDLU_ReleaseSurface( drawSurface );
}

static void DrawDialogLogo( MRect *pauseRect, int shade )
{
	MRect drawRect;
	int alpha;

	drawRect.left   = (pauseRect->left + ((pauseRect->right - pauseRect->left) * 1 / 2) ) - (logoRect.right / 2);
	drawRect.top    = (pauseRect->top + 14);
	drawRect.bottom = drawRect.top + logoRect.bottom;
	drawRect.right  = drawRect.left + logoRect.right;
	
	SDLU_AcquireSurface( drawSurface );
	
	alpha = (shade > 63)? 31: (shade / 2);
		
	SurfaceBlitWeightedDualAlpha(  drawSurface,  logoSurface,  logoMaskSurface,  logoAlphaSurface,  drawSurface,
                                  &drawRect,    &logoRect,    &logoRect,        &logoRect,         &drawRect,
                                   alpha );

	SDLU_ReleaseSurface( drawSurface );
}


enum
{ 
	kNothing = -1,
	
// main pause screen (kEndGame is reused in continue and register)
	kMusic = 0,		kEndGame,
	kSound,			kPauseGame,
	kControls,		kResume,
	kRegisterNow,   kSecret,
	kWarp,       	kSoundTest,

// continue screen
    kContinue,      
    
// register screen
    kLater,
    
// controls screen
    k1PLeft,        k2PLeft,
    k1PRight,       k2PRight,
    k1PDrop,        k2PDrop,
    k1PRotate,      k2PRotate,
    kControlsOK,    kControlsReset,
   
// shareware notice screen
	kSharewareNoticeNotYet, 
	kSharewareNoticeEnterCode,
	kSharewareNoticePurchase,
	
// enter code screen
	kEnterCodeOK,
	kEnterCodeNotYet
};

static void DrawContinueContents( int *item, int shade )
{
	char line[4][50] = { "Do you want to continue?",
	                     "Yes",
	                     "No",
	                     "" };	                 
	MPoint dPoint[4] = { {233, 210}, {280, 220}, {280, 400}, {335, 400} }, hPoint = {255, 320};
	static int lastCountdown = 0;
	int index, countdown, fade;
	int r, g, b;
	                 
	sprintf( line[3], "%d credit%c", credits, (credits != 1)? 's': ' ' );

	SDLU_AcquireSurface( drawSurface );

	for( index=0; index<4; index++ )
	{	
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), 
						 ( (index == 0)                          ||
						  ((index == 1) && (*item == kContinue)) ||
						  ((index == 2) && (*item == kEndGame ))    )? kTextBrightRainbow: kTextRainbow );
	}
	
	countdown = shade / 100;
	if( countdown < 10 )
	{
		continueTimeOut = false;
		
		if( (countdown != 0) && (countdown != lastCountdown) )
		{
			PlayMono( kContinueSnd );
		}
		lastCountdown = countdown;
		
		if( countdown < 5 )
		{
			r = (countdown * 31) / 5;
			g = 31;
		}
		else
		{
			r = 31;
			g = ((10 - countdown) * 31) / 5;
		}
			
		fade = shade % 100;
		if( fade > 50 ) fade = 50;
		r = ((31 * (49 - fade)) + (r * fade)) / 49;
		g = ((31 * (49 - fade)) + (g * fade)) / 49;
		b = ((31 * (49 - fade))) / 49;
		
		countdown = '9' - countdown;
		hPoint.h -= continueFont->width[countdown] / 2;

		for( shade = 4; shade > 0; shade-- )
		{
			MPoint hP = hPoint;
			
			hP.h += 2 * shade;
			hP.v += 2 * shade;

			SurfaceBlitWeightedCharacter( continueFont, countdown, &hP, 0, 0, 0, 20 - 4*shade ); 
		}

		SurfaceBlitCharacter( continueFont, countdown, &hPoint, r, g, b, 0 ); 
	}
	else
	{
		continueTimeOut = true;
	}
	
	SDLU_ReleaseSurface( drawSurface );
}

static void DrawHiScoreContents( int *item, int shade )
{
	MPoint dPoint[3] = { {240, 640}, {260, 640}, {335, 400} }, hPoint = {294, 145};
	MPoint dashedLinePoint = { 320, 140 };
	int    index;		
	int    nameLength;
	char  *line[3], *scan;
	
	item; // is unused

	line[0] = highScoreText;
	line[1] = "Please enter your name and press return:";
	line[2] = highScoreRank;

	for( index=0; index<2; index++ )
	{
		scan = line[index];
		while( *scan )
			dPoint[index].h -= smallFont->width[*scan++];
		
		dPoint[index].h /= 2;
	}	
		
	SDLU_AcquireSurface( drawSurface );	

	while( dashedLinePoint.h < 490 )
	{
		SurfaceBlitCharacter( dashedLineFont, '.', &dashedLinePoint, 0, 0, 0, 0 );
	}
	
	nameLength = strlen(highScoreName);
	for( index = 0; index < nameLength; index++ )
	{
		SurfaceBlitCharacter( bigFont, highScoreName[index], &hPoint, 31, 31, 31, 1 );			
		if( hPoint.h >= 475 )
		{
			highScoreName[index] = '\0';
			break;
		}
	}

	index = (int)(( 1.0 + sin( MTickCount() / 7.5 ) ) * 15.0);	
	SurfaceBlitCharacter( bigFont, '|', &hPoint, index, index, 31, 1 );			

	for( index=0; index<3; index++ )
	{	
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), (index != 2)? kTextBrightRainbow: kTextRainbow );
	}
	
	SDLU_ReleaseSurface( drawSurface );	
}

static void DrawRegisterContents( int *item, int shade )
{
	int index;		
	MPoint dPoint[4] = { {240, 150}, {260, 160}, {305, 170}, {305, 400} };
	char line[4][50] = {  "Sorry, you must register Candy Crisis",
	                      "to gain access to two player mode!",
	                      "Register Now",
	                      "Not Yet" };
	
	SDLU_AcquireSurface( drawSurface );	

	for( index=0; index<4; index++ )
	{	
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), 
						 ( (index == 0)                              ||
						   (index == 1)                              ||
						  ((index == 2) && (*item == kRegisterNow )) ||
						  ((index == 3) && (*item == kLater       ))    )? kTextBrightRainbow: kTextRainbow );
	}

	SDLU_ReleaseSurface( drawSurface );	
}

static void DrawControlsContents( int *item, int shade )
{
	MBoolean    highlight;
	MPoint      dPoint;
	int         index;
	const char* controlName;
	int         r, g, b;
	const char  label[8][20] = { "1P Left",   "2P Left", 
	                             "1P Right",  "2P Right", 
	                             "1P Drop",   "2P Drop",
	                             "1P Rotate", "2P Rotate" };
	                           
	                         
	SDLU_AcquireSurface( drawSurface );	

	for( index=0; index<8; index++ )
	{	
		highlight = (index == (*item - k1PLeft));
		
		dPoint.v = 229 + ((index & ~1) * 13);
		dPoint.h = (index & 1)? 325: 130;
		DrawRainbowText( smallFont, label[index], dPoint, (0.25 * index) + (0.075 * shade), highlight? kTextBrightRainbow: kTextRainbow );
				
		dPoint.v = 245 + ((index & ~1) * 13);
		dPoint.h = (index & 1)? 420: 225;		
		r = (int)(highlight? 31.0: 0.0);
		g = b = (int)(highlight? 31.0 - (11.0 * (sin(shade * 0.2) + 1.0)): 0.0);
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 ); 
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 ); 
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 ); 
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 ); 
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );  // 80 pixels across
		
		controlName = SDL_GetKeyName( playerKeys[index & 1][index >> 1] );
		if( controlName == NULL ) controlName = "???";
		
		dPoint.v = 231 + ((index & ~1) * 13);
		dPoint.h = (index & 1)? 460: 265;		
		dPoint.h -= GetTextWidth( tinyFont, controlName ) / 2;
		DrawRainbowText( tinyFont, controlName, dPoint, (0.1 * shade), (controlToReplace == index)? kTextBlueGlow: kTextWhite );		
	}

	dPoint.h = 200;
	dPoint.v = 340;
	DrawRainbowText( smallFont, "£ OK", dPoint, 8.0 + (0.075 * shade), (*item == kControlsOK)? kTextBrightRainbow: kTextRainbow );

	dPoint.h = 365;
	dPoint.v = 340;
	DrawRainbowText( smallFont, "£ Reset", dPoint, 8.25 + (0.075 * shade), (*item == kControlsReset)? kTextBrightRainbow: kTextRainbow );
	
	SDLU_ReleaseSurface( drawSurface );
}

static void DrawPauseContents( int *item, int shade )
{
	MPoint dPoint;
	int itemCount = IsRegistered()? 6: 7;
	int index;
	char *line[7] = { "Á Music",           "£ End Game",
	                  "Á Sound",           "£ Hide Game",
	                  "£ Controls",        "£ Resume",
	                  "£ Register Now"                       };

	
	if( level == kTutorialLevel ) line[1] = "£ Skip Tutorial";
	
	if( !musicOn ) line[0] = "ª Music";
	if( !soundOn ) line[2] = "ª Sound";

	SDLU_AcquireSurface( drawSurface );	
	
	for( index=0; index<itemCount; index++ )
	{	
		dPoint.h = (index & 1)? 340: 180;
		dPoint.v = 240 + ((index & ~1) * 15);
		
		DrawRainbowText( smallFont, line[index], dPoint, (0.25 * index) + (0.075 * shade), (*item == index)? kTextBrightRainbow: kTextRainbow );
	}
	
	SDLU_ReleaseSurface( drawSurface );
}

static void DrawSharewareNoticeContents( int *item, int shade )
{
	MPoint      dPoint;
	int         index;
	MBoolean    itemsAreDimmed;
	const char* line[10] = { "Candy Crisis is not free! You have a 30 day trial",
	                         "period to try out the software. After 30 days, if",
	                         "you wish to continue playing Candy Crisis, you",
	                         "are expected to register.",
	                         "",
	                         "When you register Candy Crisis, we will send a",
	                         "unique registration code to you by e-mail or postal",
	                         "mail. This registration code will allow you to play",
	                         "all twelve levels of the game, enables two player",
	                         "mode, and removes these shareware notices." };

	itemsAreDimmed = SharewareNoticeIsStillWaiting();
	
	SDLU_AcquireSurface( drawSurface );	
	
	dPoint.v = 124;
	dPoint.h = 140;
	DrawRainbowText( bigFont, "Candy Crisis is shareware!", dPoint, 0, kTextWhite );
	
	for( index=0; index<10; index++ )
	{	
		dPoint.v = 165 + (16 * index);
		dPoint.h = 156;
		DrawRainbowText( tinyFont, line[index], dPoint, 0, 2 );
	}
	
	dPoint.v = 340;
	dPoint.h = 130;
	DrawRainbowText( smallFont, "£ Not Yet", dPoint, (shade * 0.1), itemsAreDimmed? kTextGray: (*item == kSharewareNoticeNotYet? kTextBlueGlow: kTextWhite) );

	dPoint.v = 340;
	dPoint.h = 260;
	DrawRainbowText( smallFont, "£ Purchase", dPoint, (shade * 0.1), itemsAreDimmed? kTextGray: (*item == kSharewareNoticePurchase? kTextBlueGlow: kTextWhite) );

	dPoint.v = 340;
	dPoint.h = 390;
	DrawRainbowText( smallFont, "£ Enter Code", dPoint, (shade * 0.1), itemsAreDimmed? kTextGray: (*item == kSharewareNoticeEnterCode? kTextBlueGlow: kTextWhite) );

	SDLU_ReleaseSurface( drawSurface );
}

static void DrawEnterCodeContents( int *item, int shade )
{
	MPoint      dPoint;
	int         index;
	const char* line[4] = { "Thank you for your purchase! First, enter your",
	                        "name below. Please make sure it matches your",
	                        "name exactly as it appears in your registration",
	                        "email." };
	                         
	SDLU_AcquireSurface( drawSurface );	
	
	dPoint.v = 124;
	dPoint.h = 240;
	DrawRainbowText( bigFont, "Enter Code", dPoint, 0, kTextWhite );
	
	for( index=0; index<4; index++ )
	{	
		dPoint.v = 165 + (16 * index);
		dPoint.h = 156;
		DrawRainbowText( tinyFont, line[index], dPoint, 0, kTextAlmostWhite );
	}

	dPoint.v = 165 + (int)(5.75 * 16);
	dPoint.h = 140;
	DrawRainbowText( dashedLineFont, "......................", dPoint, 0, kTextGray );

	dPoint.v = 165 + (int)(4.5 * 16);
	dPoint.h = 150;
	dPoint = DrawRainbowText( smallFont, nameField, dPoint, 0, kTextWhite );
	
	if( whichField == nameField )
	{
		DrawRainbowText( smallFont, "|", dPoint, (shade * 0.1), kTextBlueGlow );
	}
	
	dPoint.v = 165 + (int)(7 * 16);
	dPoint.h = 156;
	DrawRainbowText( tinyFont, "Next, enter your registration code.", dPoint, 0, kTextAlmostWhite );
	

	dPoint.v = 165 + (int)(9.75 * 16);
	dPoint.h = 140;
	DrawRainbowText( dashedLineFont, "......................", dPoint, 0, kTextGray );

	dPoint.v = 165 + (int)(8.5 * 16);
	dPoint.h = 150;
	dPoint = DrawRainbowText( smallFont, keyField, dPoint, 0, kTextWhite );

	if( whichField == keyField )
	{
		DrawRainbowText( smallFont, "|", dPoint, (shade * 0.1), kTextBlueGlow );
	}
	
	dPoint.v = 340;
	dPoint.h = 150;
	DrawRainbowText( smallFont, "£ OK", dPoint, (shade * 0.1), (*item == kEnterCodeOK? kTextBlueGlow: kTextWhite) );

	dPoint.v = 340;
	dPoint.h = 380;
	DrawRainbowText( smallFont, "£ Go Back", dPoint, (shade * 0.1), (*item == kEnterCodeNotYet? kTextBlueGlow: kTextWhite) );

	if( batsuAlpha > 0 )
	{
		dPoint.v = 240 - 111;
		dPoint.h = 320 - 111;
		SurfaceBlitWeightedCharacter( batsuFont, 'X', &dPoint, 31, 0, 0, batsuAlpha-- );
	}
	
	SDLU_ReleaseSurface( drawSurface );
}

static MBoolean ContinueSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	MRect yes = {280, 220, 300, 260}, no = {280, 400, 300, 440};
	MPoint p;
	
	inSDLKey; // is unused 
	
	if( continueTimeOut )
	{
		*item = kEndGame;
		return true;
	}
	
	if( inKey == kEscapeKey )
	{
		*item = kContinue;
		return true;
	}
	
	SDLU_GetMouse( &p );

	     if( MPointInMRect( p, &yes ) ) *item = kContinue;	
	else if( MPointInMRect( p, &no  ) ) *item = kEndGame;
	else *item = kNothing;
	
	return( SDLU_Button( ) && (*item != kNothing) );
}

static MBoolean RegisterSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	MRect registerNow = {305, 170, 325, 290}, registerLater = {305, 400, 325, 470};
	MPoint p;
	
	inKey, inSDLKey; // is unused 
		
	if( inKey == kEscapeKey )
	{
		*item = kLater;
		return true;
	}
	
	SDLU_GetMouse( &p );

	     if( MPointInMRect( p, &registerNow   ) ) *item = kRegisterNow;	
	else if( MPointInMRect( p, &registerLater ) ) *item = kLater;
	else *item = kNothing;
	
	if( SDLU_Button( ) )
	{
		switch( *item ) 
		{
			case kRegisterNow:
				PlayMono( kClick );
				return true;
			
			case kLater:
				PlayMono( kClick );
				return true;
		}
	}
	
	return false;
}

static MBoolean HiScoreSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	int nameLength;
	
	inSDLKey; // is unused
	
	nameLength = strlen(highScoreName);
	
	// return
	if( inKey == 13 ) 
	{
		if( nameLength > 0 )
		{
			*item = kResume;
			PlayMono( kSquishy );
			return true;
		}
		else
		{
			PlayMono( kClick );
		}
	}
	
	// backspace
	else if( inKey == 8 )
	{
		if( nameLength > 0 )
		{
			highScoreName[ nameLength-1 ] = '\0';
			PlayMono( kClick );
		}
	}
	
	// characters
	else if( bigFont->width[inKey] != 0 )
	{
		highScoreName[ nameLength++ ] = inKey;
		highScoreName[ nameLength   ] = '\0';
		PlayMono( kPlace );
	}
	
	*item = kNothing;
	return false;
}


static MBoolean ControlsSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	MPoint          p;
	MRect           dRect;
	int             index;
	static MBoolean lastDown = false;
	MBoolean        down;
	MRect           okRect = { 340, 200, 360, 255 };
	MRect           resetRect = { 340, 365, 360, 450 };
	int             returnValue = 0;
	
	inKey; // unused
	
	*item = kNothing;

	down = SDLU_Button();
	SDLU_GetMouse( &p );

	if( MPointInMRect( p, &okRect ) )
	{
		*item = kControlsOK;
		if( down )
		{
			PlayMono( kClick );
			returnValue = 1;
			controlToReplace = -1;
		}
	}
	else if( MPointInMRect( p, &resetRect ) )
	{
		*item = kControlsReset;
		if( down && !lastDown )
		{
			PlayMono( kClick );
			memcpy( playerKeys, defaultPlayerKeys, sizeof(playerKeys) );
		}
	}
	else
	{
		for( index=0; index<8; index++ )
		{
			dRect.top    = 229 + ((index & ~1) * 13);
			dRect.left   = (index & 1)? 325: 130;
			dRect.bottom = dRect.top + 24;
			dRect.right  = dRect.left + 175;

			if( MPointInMRect( p, &dRect ) )
			{
				*item = k1PLeft + index;
				if( down && !lastDown && !AnyKeyIsPressed() ) 
				{
					controlToReplace = (controlToReplace == index)? -1: index;
				}
				break;
			}
		}
	}
	
	if( inSDLKey != 0 && controlToReplace != -1 )
	{
		playerKeys[controlToReplace & 1][controlToReplace >> 1] = inSDLKey;
		controlToReplace = -1;
	}
	
	lastDown = down;
	
	return returnValue;
}


static MBoolean PauseSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	inSDLKey; // is unused
	
	MRect targetRect[] = 
	{	
		{ 240, 180, 260, 320 },
		{ 240, 340, 260, 480 },
		{ 270, 180, 290, 320 },
		{ 270, 340, 290, 480 },
		{ 300, 180, 320, 320 },
		{ 300, 340, 320, 480 },
		{ 330, 180, 350, 320 },
	    { 120, 550, 130, 560 }
	};

	static MBoolean lastDown = false;
	int trigger;
	int index;
	MPoint p;
	
	SDLU_GetMouse( &p );
	
	trigger = SDLU_Button();
	if( inKey == kEscapeKey )
	{
		*item = kResume;
		trigger = true;
	}
	else
	{
		*item = kNothing;
		for( index=0; index<arrsize(targetRect); index++ )
		{
			if( MPointInMRect( p, &targetRect[index] ) )
			{
				*item = index;
			}
		}
	}
	
	if( trigger )
	{
		if( !lastDown )
		{
			lastDown = true;
			
			switch( *item )
			{
				case kSound:     PlayMono( kClick ); soundOn = !soundOn; PlayMono( kClick );     return false;
				case kMusic:     PlayMono( kClick ); musicOn = !musicOn; EnableMusic( musicOn ); return false;
				case kEndGame:   PlayMono( kClick );                                             return true;
				case kResume:    PlayMono( kClick );                                             return true;

				case kPauseGame: 
					PlayMono( kClick );
					SDL_WM_IconifyWindow();
                    WaitForRegainFocus();
					ItsTimeToRedraw();
					return false;

				case kRegisterNow:
					PlayMono( kClick );
					return true;
					
				case kControls:  
					PlayMono( kClick );
					return true;
				
				case kSecret:
					if( ControlKeyIsPressed( ) )
					{
						*item = kWarp;
						level = Warp( );
						return true;
					}
					else if( OptionKeyIsPressed( ) )
					{
						//SoundTest( );
						ItsTimeToRedraw();
					}
					return false;
			}
		}
	}
	else
	{
		lastDown = false;
	}
	
	return false;
}

static MBoolean SharewareNoticeSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	MRect	 notYetRect    = { 340, 130, 360, 220 };
	MRect    purchaseRect  = { 340, 260, 360, 365 };
	MRect    enterCodeRect = { 340, 390, 360, 520 };
	MPoint   p;
	MBoolean button;

	inKey, inSDLKey; // are unused
	
	*item = kNothing;
	
	if( !SharewareNoticeIsStillWaiting() )
	{
		SDLU_GetMouse( &p );
		button = SDLU_Button();
		
		     if( MPointInMRect( p, &notYetRect ) )      *item = kSharewareNoticeNotYet;
		else if( MPointInMRect( p, &enterCodeRect ) )   *item = kSharewareNoticeEnterCode;
		else if( MPointInMRect( p, &purchaseRect ) )
		{
			*item = kSharewareNoticePurchase;
			if( button )
			{
				WaitForRelease();
				LaunchURL( "http://candycrisis.com/register.html" );
				button = false;
			}
		}
		
		return (*item != kNothing) && button;
	}
	 
	return false;
}


static MBoolean EnterCodeSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	MRect  okRect     = { 340, 150, 360, 220 };
	MRect  notYetRect = { 340, 380, 360, 500 };
	MRect  nameRect   = { 237, 140, 257, 500 };
	MRect  keyRect    = { 301, 140, 321, 500 };
	MPoint p;
	int    fieldLength;
	int    button;

	inSDLKey; // is unused
	
	// -- Handle keyboard stuff. (Ripped off from high score code.)
	fieldLength = strlen(whichField);
	
	// return or tab
	if( inKey == 13 || inKey == 9 ) 
	{
		whichField = (whichField == nameField)? keyField: nameField;
		PlayMono( kRotate );		
	}
	
	// backspace
	else if( inKey == 8 )
	{
		if( fieldLength > 0 )
		{
			whichField[ fieldLength-1 ] = '\0';
			PlayMono( kClick );
		}
	}
	
	// characters
	else if( (fieldLength < 40) && (smallFont->width[inKey] != 0) )
	{
		if( whichField == keyField ) inKey = toupper(inKey);
		
		whichField[ fieldLength++ ] = inKey;
		whichField[ fieldLength   ] = '\0';
		PlayMono( kPlace );
	}

	// -- Handle mouse.
	button = SDLU_Button();

	*item = kNothing;
	SDLU_GetMouse( &p );
	
	     if( MPointInMRect( p, &notYetRect ) )        *item = kEnterCodeNotYet;
	else if( MPointInMRect( p, &nameRect) && button ) whichField = nameField;
	else if( MPointInMRect( p, &keyRect) && button )  whichField = keyField;
    else if( MPointInMRect( p, &okRect ) )
    {
    	*item = kEnterCodeOK;

    	if( button )
		{
			WaitForRelease();
			playerIsRegistered = ValidateCode( nameField, keyField );
			if( playerIsRegistered )
			{
				strcpy( registeredName, nameField );
				strcpy( registeredKey, keyField );
			}
			else
			{
				batsuAlpha = 31;
				PlayMono( kBatsuSnd );
				button = false;
			}
		}
	}
	
	return (*item != kNothing) && button;
}


void SharewareNotice( int forceWait )
{
	SDL_FillRect( frontSurface, &frontSurface->clip_rect, SDL_MapRGB( frontSurface->format, 40, 40, 40 ) );
	SDL_Flip( frontSurface );
	dialogUndimTime = MTickCount() + forceWait;

	HandleDialog( kSharewareNoticeDialog );
}


void HandleDialog( int type )
{	
	const float    lighten[4] = { 12.0f, 6.0f, 1.0f, 6.0f };
	const MRect    boardWorldZRect = {0, 0, kBlobVertSize * (kGridDown-1), kBlobHorizSize * kGridAcross};
	const MRect    fullRect = { 0, 0, 480, 640 };
	SDL_Rect       fullSDLRect = { 0, 0, 640, 480 };
	SDL_Rect       joinSDLRect;
	int            skip = 1;
	int            count;
	char           inASCII;
	SDLKey         inSDLKey;
	MRect          pauseRect, joinRect;
	
	// Clear state 
	whichField = nameField;
	nameField[0] = '\0';
	keyField[0] = '\0';
	batsuAlpha = 0;
	controlToReplace = -1;
	
	// Remember dialog info
	dialogType = type;
	dialogStage = kOpening;
	colorWrap = 0;
	colorInc = (RandomBefore(250) + 250.0) / 10000.0;

	smallFont      = GetFont( picFont );
	tinyFont       = GetFont( picTinyFont );
	bigFont        = GetFont( picHiScoreFont );
	dashedLineFont = GetFont( picDashedLineFont );
	continueFont   = GetFont( picContinueFont );
	batsuFont      = GetFont( picBatsuFont );
		
	if( type == kSharewareNoticeDialog || type == kEnterCodeDialog )
	{	
		// People shouldn't enjoy the nag dialogs, so let's not have flashy animated colors.
		for( count=0; count<4; count++ )
		{
			backColor[count].red   = 2;
			backColor[count].green = 2;
			backColor[count].blue  = 2;
		}
	}
	else
	{
		// Pick some colors to animate.
		for( count=0; count<4; count++ )
		{
			SDL_Color inColor;
			
			SDLU_GetPixel( boardSurface[0], RandomBefore( boardWorldZRect.right ), RandomBefore( boardWorldZRect.bottom ), &inColor );
		
			backColor[count].red   = inColor.r * (32.0f / 256.0f);
			backColor[count].green = inColor.g * (32.0f / 256.0f);
			backColor[count].blue  = inColor.b * (32.0f / 256.0f);

			backColor[count].red   = min( 31.0f, backColor[count].red   + lighten[count] );
			backColor[count].green = min( 31.0f, backColor[count].green + lighten[count] );
			backColor[count].blue  = min( 31.0f, backColor[count].blue  + lighten[count] );
		}
	}
	
	// Get some graphics that we're going to need
	logoSurface      = LoadPICTAsSurface( picLogo, 16 );
	logoAlphaSurface = LoadPICTAsSurface( picLogoAlpha, 16 );
	logoMaskSurface  = LoadPICTAsSurface( picLogoMask, 1 );

	// Get a copy of the current game window contents
	backSurface      = SDLU_InitSurface( &fullSDLRect, 16 );
	
	SDLU_BlitSurface( frontSurface, &frontSurface->clip_rect,
	                  backSurface,  &backSurface->clip_rect );
		
	drawSurface      = SDLU_InitSurface( &fullSDLRect, 16 );

	SDLU_BlitSurface( backSurface, &backSurface->clip_rect,
	                  drawSurface, &drawSurface->clip_rect  );

	//
		
	PlayMono( kWhomp );
	dialogTimer = MTickCount();
	dialogTarget = 0;
	dialogShade = 0;
	dialogStageComplete = false;
	dialogItem = kNothing;
	lastPauseRect.top = lastPauseRect.left = 9999;
	lastPauseRect.bottom = lastPauseRect.right = -9999;

	SDLU_StartWatchingTyping();
	
	DoFullRepaint = ItsTimeToRedraw;

	while( ((dialogStage != kClosing) || !dialogStageComplete) && !finished )
	{
		dialogTimer += skip;

		// Check mouse and keyboard
		SDLU_CheckTyping( &inASCII, &inSDLKey );
		
		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			MBoolean (*DialogSelected[kNumDialogs])( int *item, unsigned char inKey, SDLKey inSDLKey ) =
			{
				PauseSelected,
				HiScoreSelected,
				ContinueSelected,
				RegisterSelected,
				ControlsSelected,
				SharewareNoticeSelected,
				EnterCodeSelected
			};
			
			if( DialogSelected[dialogType]( &dialogItem, inASCII, inSDLKey ) )
			{
				dialogStage = kClosing; 
				dialogTarget = 0;
			}
		}

		// Do animation ...
		{
			const MBoolean dialogIsLarge[kNumDialogs] = { false, false, false, false, true, true, true };

			pauseRect = lastPauseRect;
			dialogStageComplete = DrawDialogBox( dialogIsLarge[dialogType], dialogStage, &dialogTarget, skip, &colorWrap, colorInc, &pauseRect );
			SurfaceGetEdges( backSurface, &pauseRect );
		}

		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			void (*DialogDraw[kNumDialogs])( int *item, int shade ) =
			{
				DrawPauseContents,
				DrawHiScoreContents,
				DrawContinueContents,
				DrawRegisterContents,
				DrawControlsContents,
				DrawSharewareNoticeContents,
				DrawEnterCodeContents
			};

			// Refresh screen if necessary
			if( timeToRedraw )
			{
				SDLU_BlitFrontSurface( backSurface, &fullSDLRect, &fullSDLRect );
				timeToRedraw = false;
			}
			
			// ... and fade in the logo

			dialogShade += skip;

			{
				const MBoolean dialogHasCandyCrisisLogo[kNumDialogs] = { true, true, true, true, true, false, false };
				
				if( dialogHasCandyCrisisLogo[dialogType] )
					DrawDialogLogo( &pauseRect, dialogShade );
			}
			
			// ... and animation is complete so add content			
			DialogDraw[dialogType]( &dialogItem, dialogShade );
			
			// ... and cursor
			DrawDialogCursor( &pauseRect, &dialogShade );
		}

		SurfaceCurveEdges( drawSurface, &pauseRect );

		// Draw new animation on screen
		UnionMRect( &lastPauseRect, &pauseRect, &joinRect );
		SDLU_MRectToSDLRect( &joinRect, &joinSDLRect );
		SDLU_BlitFrontSurface( drawSurface, &joinSDLRect, &joinSDLRect );

		lastPauseRect = pauseRect;

		// Wait for next frame
		if( dialogTimer <= MTickCount( ) )
		{
			dialogTimer = MTickCount( );
			skip = 2;
		}
		else
		{
			skip = 1;
			while( dialogTimer > MTickCount( ) ) 
			{
				SDLU_Yield();  
			}
		}
	}
	
	DoFullRepaint = NoPaint;

	SDLU_StopWatchingTyping();

	// Bring back previous screen
	SDLU_BlitFrontSurface( backSurface, &fullSDLRect, &fullSDLRect );

	// Dispose the GWorlds and fonts we used
	SDL_FreeSurface( backSurface );
	SDL_FreeSurface( drawSurface );
	SDL_FreeSurface( logoSurface );
	SDL_FreeSurface( logoAlphaSurface );
	SDL_FreeSurface( logoMaskSurface );
			
	switch( dialogItem )
	{
		case kRegisterNow:
			SharewareNotice( 0 );
			RefreshAll();
			break;
		
		case kSharewareNoticeEnterCode:
			HandleDialog( kEnterCodeDialog );
			break;
			
		case kEnterCodeNotYet:
			HandleDialog( kSharewareNoticeDialog );
			break;
			
		case kControls:
			HandleDialog( kControlsDialog );
			HandleDialog( kPauseDialog );
			break;
		
		case kEndGame:
			ChooseMusic( -1 );
			AddHiscore( score[0] );
			if( players == 1 )
			{
				ShowGameOverScreen( );
			}
			else
			{
				QuickFadeOut(NULL);
			}
			
			showStartMenu = true;
			break;
		
		case kContinue:
			displayedScore[0] = score[0] = roundStartScore[0];
			ShowScore( 0 );
			BeginRound( true );
			break;
			
		case kWarp:
		{
			int newLevel = level;
			
			InitGame( OptionKeyIsPressed()? kAIControl: kPlayerControl, kAIControl ); // this clears "level" ...
			level = newLevel;                                                         // so we need to set "level" afterwards
			BeginRound( true );	
			break;
		}
	}
}
