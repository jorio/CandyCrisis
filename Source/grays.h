// grays.h

void InitGrays( void );
void CalculateGrays( int player, int BlobsToDrop );
void SetupGrays( int player );
void DropGrays( int player );
void BlinkGrays( int player );
void PlaceGrayRow( int player, int grayX );
void Bounce( int player );
void LockGrays( int player );
MBoolean BusyDroppingGrays( int player );

extern int grays[2][kGridAcross], grayAir[2][kGridAcross], graySpeed[2];
extern int unallocatedGrays[2], lockGrays[2], rowBounce[2][kGridAcross], splat[2][kGridAcross];
extern int blinkTime[2], sunTime[2], blinkStage[2], sunStage[2];

#define kTimeBetweenBlinks 120
