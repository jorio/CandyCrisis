// random.h

void InitRandom( int numPieces );
int GetPiece( int player );
int RandomBefore( int what );
int GetMagic( int player );
int GetGrenade( int player );
void AddExtraPiece( void );

extern int pieceMap[kBlobTypes];
