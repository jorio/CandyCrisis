// score.h

void InitScore( void );
void UpdateScore( int player );
void ShowScore( int player );
void DrawCharacter( char which, const MRect *myRect );

#define kNumberHorizSize 16
#define kNumberVertSize 32
#define kNumberAmount 41

extern MRect scoreWindowZRect, scoreWindowRect[2];
extern MBoolean scoreWindowVisible[2];
extern long roundStartScore[2], score[2], displayedScore[2];

extern const char characterList[];
/* 
{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', '.', '0', '1', '2', 
  '3', '4', '5', '6', '7', '8', '9' };
*/
#define kCharacterZero  27
#define kCharacterScore '!'
#define kCharacterStage '#'
