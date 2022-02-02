// gameticks.h

#include "MTypes.h"

MTicks MTickCount();
void InitGameTickCount( void );
void FreezeGameTickCount( void );
void UnfreezeGameTickCount( void );
unsigned int  GameTickCount( void );
