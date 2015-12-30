// pause.cpp

// All of this code is fugly. I really needed a dialog manager, but I didn't know it at the time,
// and instead I cobbled this together. It is just barely good enough to work. Fortunately it looks
// decent to the end user...


#include "stdafx.h"

#if __APPLE__
#include <Cocoa/Cocoa.h>
#endif

#include "SDLU.h"

#include <algorithm>
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

using std::min;
using std::max;

const char kEscapeKey = 0x1B;

struct FRGBColor
{
	float red, green, blue;
};

SDL_Surface* backSurface;
SDL_Surface* drawSurface;
SDL_Surface* logoSurface;
SDL_Surface* logoMaskSurface;
SDL_Surface* logoAlphaSurface;

SkittlesFontPtr smallFont, bigFont, dashedLineFont, continueFont, tinyFont, batsuFont;
FRGBColor backColor[4];
MBoolean continueTimeOut;

static int dialogType, dialogStage, dialogTimer, dialogTarget, dialogShade, dialogItem;
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
	int   current;
	int   r,g,b;
	float s;
	
	current = 0;
	
	switch( bright )
	{	
			case kTextGray:
				r = g = b = 96;
				break;
				
			case kTextBlueGlow:
				s = sin(wave);
				r = (int)(88.0 + 120.0 * s * s);
				g = r;
				b = 255;
				break;
				
			case kTextWhite:
				r = g = b = 255;
				break;
				
			case kTextAlmostWhite:
            default:
				r = g = b = 224;
				break;
				
	}

	while( line[current] )
	{
		switch( bright )
		{
			case kTextBrightRainbow:
				r = (int)(208.0 + 40.0 * sin(wave                    ));
				g = (int)(208.0 + 40.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(208.0 + 40.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;

			case kTextRainbow:
				r = (int)(128.0 + 96.0 * sin(wave                    ));
				g = (int)(128.0 + 96.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(128.0 + 96.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;
		}

		SurfaceBlitCharacter( font, line[current], &dPoint, r, g, b, 1 );
		
		wave += 0.2;
		current++;
	}
	
	return dPoint;
}


#define kEdgeSize 8
static COLOR_T edge[4][kEdgeSize][kEdgeSize];

void SurfaceGetEdges( SDL_Surface* edgeSurface, const MRect *rect )
{
	unsigned char* src[4];
	int            srcRowBytes;
	
	src[0] = src[1] = src[2] = src[3] = (unsigned char*) edgeSurface->pixels;
	srcRowBytes = edgeSurface->pitch;

	src[0] += (srcRowBytes * (rect->top               )) + ((rect->left             ) * BYTES_PER_PIXEL);
	src[1] += (srcRowBytes * (rect->top               )) + ((rect->right - kEdgeSize) * BYTES_PER_PIXEL);
	src[2] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->left             ) * BYTES_PER_PIXEL);
	src[3] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->right - kEdgeSize) * BYTES_PER_PIXEL);
	
	for (int count=0; count<4; count++)
	{
		for (int height=0; height<kEdgeSize; height++)
		{
			memcpy( edge[count][height], src[count], kEdgeSize * BYTES_PER_PIXEL );
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

	src[0] += (srcRowBytes * (rect->top               )) + ((rect->left             ) * BYTES_PER_PIXEL);
	src[1] += (srcRowBytes * (rect->top               )) + ((rect->right - kEdgeSize) * BYTES_PER_PIXEL);
	src[2] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->left             ) * BYTES_PER_PIXEL);
	src[3] += (srcRowBytes * (rect->bottom - kEdgeSize)) + ((rect->right - kEdgeSize) * BYTES_PER_PIXEL);
	
    const int HALFBRIGHT_MASK =
           (CHANNEL_MASK >> 1)
        | ((CHANNEL_MASK >> 1) << BITS_PER_1CHANNEL)
        | ((CHANNEL_MASK >> 1) << BITS_PER_2CHANNELS);

    // Draw top/bottom border
	{
		COLOR_T *srcT1 = (COLOR_T*) (src[0]) + kEdgeSize;
		COLOR_T *srcB1 = (COLOR_T*) (src[2] + (srcRowBytes*(kEdgeSize-1))) + kEdgeSize;
		COLOR_T *srcT2 = srcT1 + (srcRowBytes / BYTES_PER_PIXEL);
		COLOR_T *srcB2 = srcB1 - (srcRowBytes / BYTES_PER_PIXEL);
        
		for( width = rect->right - rect->left - (kEdgeSize * 2); width > 0; width-- )
		{
			*srcT1 = 0; srcT1++;
			*srcB1 = 0; srcB1++;
			*srcT2 = (*srcT2 >> 1) & HALFBRIGHT_MASK; srcT2++;
			*srcB2 = (*srcB2 >> 1) & HALFBRIGHT_MASK; srcB2++;
		}
	}
	
	// Draw left/right border
	{
		unsigned char *srcL1 = (src[0] + (srcRowBytes * kEdgeSize));
		unsigned char *srcR1 = (src[1] + (srcRowBytes * kEdgeSize)) + BYTES_PER_PIXEL * (kEdgeSize-1);

		unsigned char *srcL2 = srcL1 + BYTES_PER_PIXEL;
		unsigned char *srcR2 = srcR1 - BYTES_PER_PIXEL;
		
		for( height = rect->bottom - rect->top - (kEdgeSize * 2); height > 0; height-- )
		{
			*(COLOR_T*)srcL1 = 0;
			*(COLOR_T*)srcR1 = 0;
			*(COLOR_T*)srcL2 = (*(COLOR_T*)srcL2 >> 1) & HALFBRIGHT_MASK;
			*(COLOR_T*)srcR2 = (*(COLOR_T*)srcR2 >> 1) & HALFBRIGHT_MASK;
			
			srcL1 += srcRowBytes; 
			srcR1 += srcRowBytes;
			srcL2 += srcRowBytes; 
			srcR2 += srcRowBytes;
		}
	}
		
	// Draw curved edges
	for( count=0; count<4; count++ )
	{
		COLOR_T *srcS = (COLOR_T*) src[count];
		
		for( height=0; height<kEdgeSize; height++ )
		{
			for( width=0; width<kEdgeSize; width++ )
			{
				switch( edgeMap[count][height][width] )
				{
					case ' ': 	*srcS = edge[count][height][width]; break;
                    case '-':	*srcS = (edge[count][height][width] >> 1) & HALFBRIGHT_MASK; break;
					case '.': 	*srcS = 0; break;
					case 'x': 	*srcS = (*srcS >> 1) & HALFBRIGHT_MASK; break;
					case 'X': 	break;
				}
				srcS++;
			}
			srcS += (srcRowBytes / BYTES_PER_PIXEL) - kEdgeSize;
		}
	}
}

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
						       _5TO8(shading) );

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
    (void) shade;
    
	SDLU_GetMouse( &p );
	
	if( p.h < (pauseRect->left      ) ) p.h = pauseRect->left;
	if( p.h > (pauseRect->right  - 5) ) p.h = pauseRect->right  - 5;
	if( p.v < (pauseRect->top       ) ) p.v = pauseRect->top;
	if( p.v > (pauseRect->bottom - 5) ) p.v = pauseRect->bottom - 5;
	q = p;
	
	SDLU_AcquireSurface( drawSurface );

	SurfaceBlitCharacter( smallFont, '\x05', &p,  0,  0,  0, 0 );
	SurfaceBlitCharacter( smallFont, '\x04', &q, 255, 255, 255, 0 );
	
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
	
	alpha = (shade > 63)? 255: (shade * 4);
		
	SurfaceBlitWeightedDualAlpha( drawSurface,  logoSurface,  logoMaskSurface,  logoAlphaSurface,  drawSurface,
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
	kSecret,
	kWarp,       	kSoundTest,

// continue screen
    kContinue,      
    
// controls screen
    k1PLeft,        k2PLeft,
    k1PRight,       k2PRight,
    k1PDrop,        k2PDrop,
    k1PRotate,      k2PRotate,
    kControlsOK,    kControlsReset,
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

            int weight = 20 - 4*shade;
            
			SurfaceBlitWeightedCharacter( continueFont, countdown, &hP, 0, 0, 0, _5TO8(weight) );
		}

		SurfaceBlitCharacter( continueFont, countdown, &hPoint, _5TO8(r), _5TO8(g), _5TO8(b), 0 );
	}
	else
	{
		continueTimeOut = true;
	}
	
	SDLU_ReleaseSurface( drawSurface );
}

static void DrawHiScoreContents( int *item, int shade )
{
	MPoint       dPoint[3] = { {240, 640}, {260, 640}, {335, 400} }, hPoint = {294, 145};
	MPoint       dashedLinePoint = { 320, 140 };
	int          index;
	int          nameLength;
    const char*  line[3];
    const char*  scan;
	
	(void) item; // is unused

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
	
	nameLength = int(strlen(highScoreName));
	for( index = 0; index < nameLength; index++ )
	{
		SurfaceBlitCharacter( bigFont, highScoreName[index], &hPoint, 255, 255, 255, 1 );
		if( hPoint.h >= 475 )
		{
			highScoreName[index] = '\0';
			break;
		}
	}

	index = (int)(( 1.0 + sin( MTickCount() / 7.5 ) ) * 120.0);
	SurfaceBlitCharacter( bigFont, '|', &hPoint, index, index, 255, 1 );

	for( index=0; index<3; index++ )
	{	
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), (index != 2)? kTextBrightRainbow: kTextRainbow );
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

        r = highlight? 255: 0;
		g = b = (int)(highlight? 255.0 - (88.0 * (sin(shade * 0.2) + 1.0)): 0.0);
		
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
	DrawRainbowText( smallFont, "\x03 OK", dPoint, 8.0 + (0.075 * shade), (*item == kControlsOK)? kTextBrightRainbow: kTextRainbow );

	dPoint.h = 365;
	dPoint.v = 340;
	DrawRainbowText( smallFont, "\x03 Reset", dPoint, 8.25 + (0.075 * shade), (*item == kControlsReset)? kTextBrightRainbow: kTextRainbow );
	
	SDLU_ReleaseSurface( drawSurface );
}

static void DrawPauseContents( int *item, int shade )
{
	MPoint dPoint;
	int index;
	const char *line[]  = { "\x01 Music",           "\x03 End Game",
                            "\x01 Sound",           "\x03 Hide Game",
                            "\x03 Controls",        "\x03 Resume"       };

    const int itemCount = arrsize(line);
	
	if( level == kTutorialLevel ) line[1] = "\x03 Skip Tutorial";
	
	if( !musicOn ) line[0] = "\x02 Music";
	if( !soundOn ) line[2] = "\x02 Sound";

	SDLU_AcquireSurface( drawSurface );	
	
	for( index=0; index<itemCount; index++ )
	{	
		dPoint.h = (index & 1)? 340: 180;
		dPoint.v = 240 + ((index & ~1) * 15);
		
		DrawRainbowText( smallFont, line[index], dPoint, (0.25 * index) + (0.075 * shade), (*item == index)? kTextBrightRainbow: kTextRainbow );
	}
	
	SDLU_ReleaseSurface( drawSurface );
}


static MBoolean ContinueSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	MRect yes = {280, 220, 300, 260}, no = {280, 400, 300, 440};
	MPoint p;
	
	(void) inSDLKey; // is unused
	
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

static MBoolean HiScoreSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	int nameLength = int(strlen(highScoreName));
	
	// return (SDL key)
	if( inSDLKey == SDLK_RETURN )
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
	
	// backspace (SDL key)
	if( inSDLKey == SDLK_BACKSPACE )
	{
		if( nameLength > 0 )
		{
			highScoreName[ nameLength-1 ] = '\0';
			PlayMono( kClick );
		}
	}
	
	// characters (ASCII key!)
	if( bigFont->width[inKey] != 0 )
	{
		highScoreName[ nameLength++ ] = inKey;
		highScoreName[ nameLength   ] = '\0';
		PlayMono( kPlace );
	}
	
	*item = kNothing;
	return false;
}


static MBoolean ControlsSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	MPoint          p;
	MRect           dRect;
	int             index;
	static MBoolean lastDown = false;
	MBoolean        down;
	MRect           okRect = { 340, 200, 360, 255 };
	MRect           resetRect = { 340, 365, 360, 450 };
	int             returnValue = 0;
	
	(void) inKey; // unused
	
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


static MBoolean PauseSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	(void) inSDLKey; // is unused
	
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
                {
					PlayMono( kClick );

                #if __APPLE__
                    // Activate the Finder, as if the user pressed cmd-tab. SDL will automatically realize focus
                    // was lost and will do the right thing.
                    NSArray *apps = [[NSWorkspace sharedWorkspace] runningApplications];
                    
                    for (NSRunningApplication *app in apps)
                    {
                        if ([app.bundleIdentifier.lowercaseString isEqualToString:@"com.apple.finder"])
                        {
                            [app activateWithOptions:NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps];
                            break;
                        }
                    }
                #endif
                    
                    return false;
                }
                    
				
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


void HandleDialog( int type )
{	
	const float    lighten[4] = { 96.0f, 48.0f, 8.0f, 48.0f };
	const MRect    boardWorldZRect = {0, 0, kBlobVertSize * (kGridDown-1), kBlobHorizSize * kGridAcross};
	SDL_Rect       fullSDLRect = { 0, 0, 640, 480 };
	SDL_Rect       joinSDLRect;
	int            skip = 1;
	int            count;
	char           inASCII;
	SDL_Keycode    inSDLKey;
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
		
    // Pick some colors to animate.
    for( count=0; count<4; count++ )
    {
        SDL_Color inColor;
        
        SDLU_GetPixel( boardSurface[0], RandomBefore( boardWorldZRect.right ), RandomBefore( boardWorldZRect.bottom ), &inColor );
    
        backColor[count].red   = min( 255.0f, inColor.r + lighten[count] );
        backColor[count].green = min( 255.0f, inColor.g + lighten[count] );
        backColor[count].blue  = min( 255.0f, inColor.b + lighten[count] );
    }
	
	// Get some graphics that we're going to need
	logoSurface      = LoadPICTAsSurface( picLogo, 32 );
	logoAlphaSurface = LoadPICTAsSurface( picLogoAlpha, 32 );
	logoMaskSurface  = LoadPICTAsSurface( picLogoMask, 1 );

	// Get a copy of the current game window contents
	backSurface      = SDLU_InitSurface( &fullSDLRect, 32 );
	
	SDLU_BlitSurface( g_frontSurface, &g_frontSurface->clip_rect,
	                  backSurface,  &backSurface->clip_rect );
		
	drawSurface      = SDLU_InitSurface( &fullSDLRect, 32 );

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

        UpdateSound();
        
		// Check mouse and keyboard
        SDLU_CheckASCIITyping( &inASCII );
        SDLU_CheckSDLTyping( &inSDLKey );
		
		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			MBoolean (*DialogSelected[kNumDialogs])( int *item, unsigned char inKey, SDL_Keycode inSDLKey ) =
			{
				PauseSelected,
				HiScoreSelected,
				ContinueSelected,
				ControlsSelected,
			};
			
			if( DialogSelected[dialogType]( &dialogItem, inASCII, inSDLKey ) )
			{
				dialogStage = kClosing; 
				dialogTarget = 0;
			}
		}

		// Do animation ...
		{
			const MBoolean dialogIsLarge[kNumDialogs] = { false, false, false, true };

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
				DrawControlsContents,
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
				const MBoolean dialogHasCandyCrisisLogo[kNumDialogs] = { true, true, true, true };
				
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
        SDLU_Present();

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
    SDLU_Present();

	// Dispose the GWorlds and fonts we used
	SDL_FreeSurface( backSurface );
	SDL_FreeSurface( drawSurface );
	SDL_FreeSurface( logoSurface );
	SDL_FreeSurface( logoAlphaSurface );
	SDL_FreeSurface( logoMaskSurface );
			
	switch( dialogItem )
	{
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
