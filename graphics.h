// graphics.h


void DrawSpriteBlobs( int player, int type );
void EraseSpriteBlobs( int player );
void CleanSpriteArea( int player, MRect *myRect );
void CalcBlobRect( int x, int y, MRect *myRect );
void DrawBackdrop( void );
void ShowTitle( void );
void InitBackdrop( void );


extern SDL_Surface* backdropSurface;


enum 
{
	blobBlinkAnimation = 0,
	blobJiggleAnimation,
	blobCryAnimation
};
