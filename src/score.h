// score.h

void InitScore( void );
void UpdateScore( int player );
void ShowScore( int player );
void DrawCharacter( char which, const MRect *myRect );

#define kNumberHorizSize 16
#define kNumberVertSize 32

extern MRect scoreWindowZRect, scoreWindowRect[2];
extern MBoolean scoreWindowVisible[2];
extern int roundStartScore[2], score[2], displayedScore[2];

extern const char characterList[];

#define kCharacterScore '!'
#define kCharacterStage '#'
