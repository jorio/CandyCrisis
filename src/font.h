// font.h


#ifndef __FONT__
#define __FONT__


typedef struct 
{
	SDL_Surface *surface;
	int width[256];
	int across[256];
}
SkittlesFont, *SkittlesFontPtr;


void            InitFont( void );
SkittlesFontPtr GetFont( int pictID );
int             GetTextWidth( SkittlesFontPtr font, const char *text );


#endif
