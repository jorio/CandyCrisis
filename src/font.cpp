// font.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "font.h"
#include "gworld.h"


const int kNumFonts = (picBatsuFont-picFont+1);

static SkittlesFont s_font[kNumFonts] = {};


static SkittlesFontPtr LoadFont( SkittlesFontPtr font, int pictID, unsigned char *letterMap )
{
	unsigned char* lastLine;
	unsigned char  white;
	MBoolean       success = false;
	int            start, across, skip;
	SDL_Surface*   temporarySurface;
	SDL_Rect       sdlRect;
	
	temporarySurface = LoadPICTAsSurface( pictID, 8 );
	
	if( temporarySurface )
	{
		sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.h = temporarySurface->h;
		sdlRect.w = temporarySurface->w;
		
		font->surface = SDLU_InitSurface( &sdlRect, 8 );
		
		SDLU_BlitSurface(  temporarySurface, &temporarySurface->clip_rect,
		                   font->surface,    &temporarySurface->clip_rect  );
		
		SDL_FreeSurface( temporarySurface );
		
		white    = SDL_MapRGB( font->surface->format, 0xFF, 0xFF, 0xFF );
		lastLine = (unsigned char*) font->surface->pixels + (font->surface->pitch * (font->surface->h - 1));
		across   = 0;
		
		// Measure empty space between character breaks
		while( lastLine[across] == white ) across++;
		skip = across;
		
		success = true;

		// Measure character starts and widths
		while( *letterMap )
		{
			while( lastLine[across] != white ) across++;
			if( across > font->surface->pitch ) 
			{
				success = false;
				break;
			}
			
			start = across;
			font->across[*letterMap] = across + (skip/2);

			while( lastLine[across] == white ) across++;		
			font->width [*letterMap] = across - start - skip;

			letterMap++;
		}
	}
	
	if( success )
	{
		return font;
	}
	else
	{
		Error( "LoadFont: files are missing or corrupt" );
		return NULL;
	}
}


void InitFont( void ) 
{
	LoadFont( &s_font[0],  picFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!:,.()*?0123456789'|-\x01\x02\x03\x04\x05 " );
	LoadFont( &s_font[1],  picHiScoreFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*().,/-=_+<>?|'\":; " );
	LoadFont( &s_font[2],  picContinueFont, (unsigned char*) "0123456789" );
	LoadFont( &s_font[3],  picBalloonFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+;:,./<>? \x01\x02'\"" );
	LoadFont( &s_font[4],  picZapFont, (unsigned char*) "0123456789*PS" );
	LoadFont( &s_font[5],  picZapOutlineFont, (unsigned char*) "0123456789*" );
	LoadFont( &s_font[6],  picVictoryFont, (unsigned char*) "AB" );
	LoadFont( &s_font[7],  picBubbleFont, (unsigned char*) "*" );
	LoadFont( &s_font[8],  picTinyFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789,.!`-=[];'/~_+{}:\"\\ " );
	LoadFont( &s_font[9],  picDashedLineFont, (unsigned char*) "." );
	LoadFont( &s_font[10], picBatsuFont, (unsigned char*) "X" );
}


SkittlesFontPtr GetFont( int pictID )
{
	int fontID = pictID - picFont;
	
	if( (fontID < 0) || (fontID >= kNumFonts) )
		Error( "GetFont: fontID" );
			
	return &s_font[fontID];
} 


int GetTextWidth( SkittlesFontPtr font, const char *text )
{
	int width = 0;
	while( *text )
	{
		width += font->width[*text++];
	}
	
	return width;
}

