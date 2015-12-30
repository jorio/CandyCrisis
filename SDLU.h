///
///  SDLU.h
///


#ifndef __SDLU__
#define __SDLU__

#include "MTypes.h"

#define BLUE_MASK          0x0000FF
#define GREEN_MASK         0x00FF00
#define RED_MASK           0xFF0000
#define CHANNEL_MASK       BLUE_MASK
#define FULL_WEIGHT        (1+CHANNEL_MASK)

#define COLOR_T            int

#define BITS_PER_1CHANNEL  8
#define BITS_PER_2CHANNELS 16
#define BITS_PER_3CHANNELS 24

#define BYTES_PER_PIXEL    4

void         SDLU_Init();
SDL_Rect*    SDLU_MRectToSDLRect( const MRect* in, SDL_Rect* out );
MRect*       SDLU_SDLRectToMRect( const SDL_Rect* in, MRect* out );
int          SDLU_BlitSurface( SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect );
void         SDLU_GetPixel( SDL_Surface* surface, int x, int y, SDL_Color* pixel );
void         SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth );
SDL_Surface* SDLU_InitSurface( SDL_Rect* rect, int depth );
void         SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect );
void         SDLU_SetBrightness( float b );
void         SDLU_AcquireSurface( SDL_Surface* surface );
SDL_Surface* SDLU_GetCurrentSurface();
void         SDLU_ReleaseSurface( SDL_Surface* surface );
void         SDLU_GetMouse( MPoint* pt );
int          SDLU_Button();
void         SDLU_Yield();
void         SDLU_PumpEvents();
void         SDLU_StartWatchingTyping();
void         SDLU_StopWatchingTyping();
MBoolean     SDLU_CheckASCIITyping(char* ascii);
MBoolean     SDLU_CheckSDLTyping(SDL_Keycode* sdlKey);
MBoolean     SDLU_IsForeground();
void         SDLU_Present();


#endif
