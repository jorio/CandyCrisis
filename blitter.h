// blitter.h

#include "font.h"


#define _5TO8(x)      (((x) << 3) | ((x) >> 2))
#define _15TO8_8_8(x) _5TO8((x >> 10) & 0x1F), \
                      _5TO8((x >> 5 ) & 0x1F), \
                      _5TO8(x & 0x1F)


// SDL type
void SurfaceBlitMask( SDL_Surface* object,     SDL_Surface* mask,     SDL_Surface* dest,
                      const MRect* objectRect, const MRect* maskRect, const MRect* destRect );

void SurfaceBlitBlob( const MRect* blobRect, const MRect* destRect ); 

void SurfaceBlitColor( SDL_Surface* mask,     SDL_Surface* dest,
                       const MRect* maskRect, const MRect* destRect, 
                       int r, int g, int b, int weight );

void SurfaceBlitAlpha( SDL_Surface* back,     SDL_Surface* source,     SDL_Surface* alpha,     SDL_Surface* dest,
                       const MRect* backRect, const MRect* sourceRect, const MRect* alphaRect, const MRect* destRect );

void SurfaceBlitWeightedDualAlpha(SDL_Surface* back,     SDL_Surface* source,  SDL_Surface* mask,     SDL_Surface* alpha,     SDL_Surface* dest,
                                   const MRect* backRect, const MRect* srcRect, const MRect* maskRect, const MRect* alphaRect, const MRect* destRect,
                                   int inWeight );

void SurfaceBlitWeightedCharacter( SkittlesFontPtr font, unsigned char text, MPoint *dPoint, int r, int g, int b, int alpha );

void SurfaceBlitCharacter( SkittlesFontPtr font, unsigned char text, MPoint *dPoint, int r, int g, int b, int dropShadow );

void SurfaceBlitBlendOver(SDL_Surface* source,     SDL_Surface* dest,
                           const MRect* sourceRect, const MRect* destRect, 
                           int r1, int g1, int b1, 
                           int r2, int g2, int b2, 
                           int r3, int g3, int b3, 
                           int r4, int g4, int b4, 
                           int weight );
                          
void SurfaceBlitColorOver(SDL_Surface* source,     SDL_Surface* dest,
                           const MRect* sourceRect, const MRect* destRect,
                           int r, int g, int b, int weight );



void SetUpdateRect( int player, MRect *where );
void UpdatePlayerWindow( int player );
void InitBlitter( void );

extern MBoolean update[2][kGridAcross][kGridDown];
extern MBoolean refresh[2];
extern MPoint topLeft[2];
