// keyselect.h


#ifdef __cplusplus
extern "C"
#endif
int SDLTypingFilter(const SDL_Event *event);

void StartWatchingTyping();
void StopWatchingTyping();
MBoolean CheckTyping( char* ascii, SDLKey* sdl );
void CheckKeys();


extern SDLKey playerKeys[2][4];
extern const SDLKey defaultPlayerKeys[2][4];
