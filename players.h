// players.h

void HandlePlayers( void );
void RetrieveBlobs( int player );
void Falling( int player );
void PlaceBlobs( int player );
void DropBlobs( int player );
void ResolveSuction( int player );
void BlinkBored( int player );
void InitPlayers( void );
void JiggleBlobs( int player );
void FastJiggleBlobs( int player );
void LockdownBlobs( int player );
void HandleMagic( int player );
void ChooseGlowingBlobs( int player );
void ConsiderGlow( int player, int color, int x, int y );
void GlowBlobs( int player );
void PlaceGrenade( int player );
void FadeCharred( int player );
void RedrawBoardContents( int player );

extern unsigned int  boredTime[2], hintTime[2], fadeCharTime[2], animTime[2], shadowDepth[2], messageTime;
extern MBoolean idling[2];
extern int emotions[2];
extern int glowColors[][3];

enum
{
	kFalling = 0,
	kRetrieveBlobs,
	kPlaceBlobs,
	kJiggleBlobs,
	kFastJiggleBlobs,
	kLockdownBlobs,
	kDropBlobs,
	kZapBlobs,
	kKillBlobs,
	kDropGrays,
	kLosing,
	kWinning,
	kChooseDifficulty,
	kWaitingToStart,
	kIdlePlayer,
	kWaitForRetrieval
};

#define kDropSpeed 1
#define kFallSpeed 1
#define kFlashSpeed 5

#define kLeftPlayerWindowCenter  0.16
#define kRightPlayerWindowCenter 0.84
