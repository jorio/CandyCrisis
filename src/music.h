// music.h


void EnableMusic( MBoolean on );
void PauseMusic( void );
void ResumeMusic( void );
void FastMusic( void );
void SlowMusic( void );
void ChooseMusic( short which );


extern MBoolean musicOn;
extern int      musicSelection;

