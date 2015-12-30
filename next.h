// next.h


void InitNext( void );
void ShowNext( int player );
void RefreshNext( int player );
void UpdateNext( int player );
void PullNext( int player );
void ShowPull( int player );

extern SDL_Surface* nextSurface;
extern SDL_Surface* nextDrawSurface;

extern MBoolean nextWindowVisible[2];
extern MRect nextWindowZRect, nextWindowRect[2];
