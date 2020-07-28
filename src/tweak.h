// tweak.h

void TweakFirstBlob( int player, MRect *first );
void TweakSecondBlob( int player, MRect *second );
void StartTweak( int player, int direction, int rotate, int fall );
void UpdateTweak( int player, int suction );
void InitTweak( void );

#define d2r(x) ((x)*(pi/180))

#define kTweakDelay 1
