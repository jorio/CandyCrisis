// music.h


void EnableMusic( MBoolean on );
void PauseMusic( void );
void ResumeMusic( void );
void FastMusic( void );
void SlowMusic( void );
int GetCurrentMusic( void );
void ChooseMusic( short which );
void ShutdownMusic();


extern MBoolean musicOn;
extern int      musicSelection;

