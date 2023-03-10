// opponent.h

void InitOpponent( void );
void BeginOpponent( int which );
void UpdateOpponent( void );
void DrawFrozenOpponent( void );
void OpponentChatter( MBoolean on );
void OpponentPissed( void );

enum
{
	kBored=0,
	kLookRight,
	kLookLeft,
	kBlink1,
	kBlink2
};

#define kOppFrames (kBlink2+1)
#define kGlowArraySize 30

extern int opponentMood, opponentFrame;
extern MTicks opponentTime, glowTime[kGlows], panicTime;
extern int glowFrame[kGlows], panicFrame;
extern int glowArray[kGlowArraySize], lightGlowArray[kGlowArraySize];

extern MRect opponentWindowZRect, opponentWindowRect;
