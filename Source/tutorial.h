// tutorial.h

typedef enum
{
	kMessage = 0,
	kIdleTicks,
	kRetrieve,
	kPosition,
	kSpin,
	kBlockUntilLand,
	kBlockUntilDrop,
	kPunish,
	kComplete,
	kBlockUntilComplete
}
AutoCommand;

typedef struct
{
	AutoCommand command;
	int d1, d2;
	char *message;
}
AutoPattern, *AutoPatternPtr;

extern AutoPatternPtr autoPattern;
extern int tutorialTime;

void InitTutorial( void );
void StartBalloon( char *message );
void StopBalloon( void );
void UpdateBalloon( void );
void EndTutorial( void );
