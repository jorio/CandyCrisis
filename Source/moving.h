// moving.h

MBoolean CanMoveDirection( int player, int dirX, int dirY );
void CalcSecondBlobOffset( int player, int *x, int *y );
MBoolean CanGoLeft( int player );
void GoLeft( int player );
MBoolean CanGoRight( int player );
void GoRight( int player );
MBoolean CanFall( int player );
void DoFall( int player );
MBoolean CanRotate( int player );
void DoRotate( int player );
void DoDrop( int player );
void StopDrop( int player );
