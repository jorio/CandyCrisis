// gworld.h


#define kBlobHorizSize 24
#define kBlobVertSize 24
#define kBlobShadowDepth 6
#define kBlobShadowError 2

#define kBlastWidth 72
#define kBlastHeight 72
#define kBlastFrames 14

void GetBlobGraphics();
void InitPlayerWorlds();

void         DrawPICTInSurface( SDL_Surface* surface, int pictID );
SDL_Surface* LoadPICTAsSurface( int pictID, int depth );
MBoolean      PICTExists( int pictID );

void SurfaceDrawBoard( int player, const MRect *myRect );
void SurfaceDrawShadow( const MRect *myRect, int blob, int state );
void SurfaceDrawAlpha( const MRect *myRect, int blob, int mask, int state );
void SurfaceDrawColor( const MRect *myRect, int blob, int state, int r, int g, int b, int w );
void SurfaceDrawBlob( int player, const MRect *myRect, int blob, int state, int charred );
void SurfaceDrawSprite( const MRect *myRect, int blob, int state );

extern SDL_Surface* blobSurface;
extern SDL_Surface* maskSurface;
extern SDL_Surface* charMaskSurface;
extern SDL_Surface* boardSurface[2];
extern SDL_Surface* blastSurface;
extern SDL_Surface* blastMaskSurface;
extern SDL_Surface* playerSurface[2];
extern SDL_Surface* playerSpriteSurface[2];
