// zap.h

#include "font.h"

void ZapScoreDisplay( int player, int amount, int multiplier, int x, int y, int c );
int SizeUp( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color );
int GetChainSize( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color );
void CleanSize( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color );
void CleanWithPolish( signed char myGrid[kGridAcross][kGridDown], signed char polish[kGridAcross][kGridDown], int x, int y, int color );
void RemoveBlobs( int player, int x, int y, int color, int generation );
void KillBlobs( int player );
void ZapBlobs( int player );
void CleanChunks( int player, int x, int y, int level, int style );
void DrawChunks( int player, int x, int y, int level, int style );
void CleanSplat( int player, int x, int y, int level );
void DrawSplat( int player, int x, int y, int level );
void GetZapStyle( int player, MRect *myRect, int *color, int *type, int which,
				  int level, int style );
void InitZapStyle( void );

extern signed char death[2][kGridAcross][kGridDown];
extern int zapIteration[2];

extern int grenadeFrame[2];
extern MRect grenadeRect[2];

extern SkittlesFontPtr zapFont, zapOutline;

#define kBlobClusterSize 4
#define kZapFrames 20
