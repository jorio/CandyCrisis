///
///  SDLU.c
///
///  SDL utilities.
///
///  John Stiles, 2002/10/12
///

#include "SDL.h"
#include "SDLU.h"
#include "gameticks.h"
#include "music.h"

#include "main.h" // for Error


// for acquiresurface
const int           k_acquireMax = 10;
static int          s_acquireHead = -1;
static SDL_Surface* s_acquireList[k_acquireMax];

// for button and getmouse
static int          s_mouseButton;
static MPoint       s_mousePosition;

// for event loop
static MBoolean     s_isForeground = true;
 
// for checktyping
static MBoolean     s_interestedInTyping = false;
static char         s_keyBufferASCII[16] = { 0 };
static SDLKey       s_keyBufferSDL[16]   = { (SDLKey) 0};
static int          s_keyBufferPullFrom = 0;
static int          s_keyBufferPutAt = 0;
static int          s_keyBufferFilled = 0;

static void	SDLUi_Blit8BitTo1Bit( SDL_Surface* surface8, SDL_Rect* rect8,
                                  SDL_Surface* surface1, SDL_Rect* rect1  )
{
	// NOTE: for now this copy assumes that we're copying the whole thing.
	// That's probably true for everything I'm doing. If it turns out that
	// I need partial 8->1 copies, this won't be too hard to fix.
	
	int          x, y, across, down;
	SDL_Color*   palette8;

	rect8; // is unused for now
	
//	ASSERTN( surface8->format->BitsPerPixel == 8, surface8->format->BitsPerPixel );
//	ASSERTN( surface1->format->BitsPerPixel == 1, surface8->format->BitsPerPixel );
//	ASSERT( rect8->w == rect1->w );
//	ASSERT( rect8->h == rect1->h );
	
	palette8 = surface8->format->palette->colors;
	down     = rect1->h;
	across   = (rect1->w + 7) & ~7;
	
	for( y=0; y<down; y++ )
	{
		unsigned char* src = (unsigned char*) surface8->pixels + (y * surface8->pitch);
		unsigned char* dst = (unsigned char*) surface1->pixels + (y * surface1->pitch);
		
		for( x=0; x<across; x+=8 )
		{
			*dst = (palette8[src[0]].r? 0: 0x80) |
			       (palette8[src[1]].r? 0: 0x40) |
			       (palette8[src[2]].r? 0: 0x20) |
			       (palette8[src[3]].r? 0: 0x10) |
			       (palette8[src[4]].r? 0: 0x08) |
			       (palette8[src[5]].r? 0: 0x04) |
			       (palette8[src[6]].r? 0: 0x02) |
			       (palette8[src[7]].r? 0: 0x01)   ;
			
			dst += 1;
			src += 8;
		}
	}
}


static void	SDLUi_Blit15BitTo16Bit( SDL_Surface* surface15, SDL_Rect* rect15,
                                    SDL_Surface* surface16, SDL_Rect* rect16  )
{
	int             x, y, srcPitch, dstPitch;
	unsigned short* src;
	unsigned short* dst;
	unsigned short  work;
	int             rect15x = rect15->x, rect15y = rect15->y, rect15w = rect15->w, rect15h = rect15->h; 
	int             rect16x = rect16->x, rect16y = rect16->y, rect16w = rect16->w, rect16h = rect16->h; 
	int             surface15w = surface15->w, surface15h = surface15->h;
	int             surface16w = surface16->w, surface16h = surface16->h;
	
	// Clip.
	if( rect15x < 0 )      { rect15w += rect15x; rect16w += rect15x; rect16x -= rect15x; rect15x = 0; }
	if( rect15y < 0 )      { rect15h += rect15y; rect16h += rect15y; rect16y -= rect15y; rect15y = 0; }
	if( (rect15x + rect15w) > surface15w ) { rect15w = surface15w - rect15x; }
	if( (rect15y + rect15h) > surface15h ) { rect15h = surface15h - rect15y; }

	if( rect16x < 0 )      { rect16w += rect16x; rect15w += rect16x; rect15x -= rect16x; rect16x = 0; }
	if( rect16y < 0 )      { rect16h += rect16y; rect15h += rect16y; rect15y -= rect16y; rect16y = 0; }
	if( (rect16x + rect16w) > surface16w ) { rect16w = surface16w - rect16x; }
	if( (rect16y + rect16h) > surface16h ) { rect16h = surface16h - rect16y; }

	// SDL changes the destination rectangle to the actual copied bounds. We need to do the same.
	if( rect16w <= 0 || rect16h <= 0 ) 
	{
		rect16->w = 0; rect16->h = 0;
		return;
	}

	rect16->x = rect16x; rect16->y = rect16y; rect16->w = rect16w; rect16->h = rect16h; 

	// Blit.
	srcPitch = surface15->pitch;
	dstPitch = surface16->pitch;
	
	src = (unsigned short*) ( ((unsigned char*) surface15->pixels) + (rect15x * 2) + (rect15y * srcPitch) );
	dst = (unsigned short*) ( ((unsigned char*) surface16->pixels) + (rect16x * 2) + (rect16y * dstPitch) );
	
	for( y=0; y<rect16h; y++ )
	{
		for( x=0; x<rect16w; x++ )
		{
			work = src[x];
			work = (work << 1) & 0xFFC0 |
			       (work >> 4) & 0x0020 |
			       (work     ) & 0x001F;
			       
			dst[x] = work;
		}
		
		src = (unsigned short*) ( ((unsigned char*) src) + srcPitch );
		dst = (unsigned short*) ( ((unsigned char*) dst) + dstPitch );
	}
}


static void	SDLUi_Blit16BitTo15Bit( SDL_Surface* surface16, SDL_Rect* rect16,
                                    SDL_Surface* surface15, SDL_Rect* rect15  )
{
	int             x, y, srcPitch, dstPitch;
	unsigned short* src;
	unsigned short* dst;
	unsigned short  work;
	int             rect15x = rect15->x, rect15y = rect15->y, rect15w = rect15->w, rect15h = rect15->h; 
	int             rect16x = rect16->x, rect16y = rect16->y, rect16w = rect16->w, rect16h = rect16->h; 
	int             surface15w = surface15->w, surface15h = surface15->h;
	int             surface16w = surface16->w, surface16h = surface16->h;

	// Clip.
	if( rect15x < 0 )      { rect15w += rect15x; rect16w += rect15x; rect16x -= rect15x; rect15x = 0; }
	if( rect15y < 0 )      { rect15h += rect15y; rect16h += rect15y; rect16y -= rect15y; rect15y = 0; }
	if( (rect15x + rect15w) > surface15w ) { rect15w = surface15w - rect15x; }
	if( (rect15y + rect15h) > surface15h ) { rect15h = surface15h - rect15y; }

	if( rect16x < 0 )      { rect16w += rect16x; rect15w += rect16x; rect15x -= rect16x; rect16x = 0; }
	if( rect16y < 0 )      { rect16h += rect16y; rect15h += rect16y; rect15y -= rect16y; rect16y = 0; }
	if( (rect16x + rect16w) > surface16w ) { rect16w = surface16w - rect16x; }
	if( (rect16y + rect16h) > surface16h ) { rect16h = surface16h - rect16y; }

	// SDL changes the destination rectangle to the actual copied bounds. We need to do the same.
	if( rect15w <= 0 || rect15h <= 0 ) 
	{
		rect15->w = 0; rect15->h = 0;
		return;
	}

	rect15->x = rect15x; rect15->y = rect15y; rect15->w = rect15w; rect15->h = rect15h; 
	
	// Blit.
	srcPitch = surface16->pitch;
	dstPitch = surface15->pitch;
	
	src = (unsigned short*) ( ((unsigned char*) surface16->pixels) + (rect16x * 2) + (rect16y * srcPitch) );
	dst = (unsigned short*) ( ((unsigned char*) surface15->pixels) + (rect15x * 2) + (rect15y * dstPitch) );
	
	for( y=0; y<rect15h; y++ )
	{
		for( x=0; x<rect15w; x++ )
		{
			work = src[x];
			work = (work >> 1) & 0x7FE0 |
			       (work     ) & 0x001F;
			       
			dst[x] = work;
		}
		
		src = (unsigned short*) ( ((unsigned char*) src) + srcPitch );
		dst = (unsigned short*) ( ((unsigned char*) dst) + dstPitch );
	}
}


static void	SDLUi_Blit24BitTo15BitHQ( SDL_Surface* surface24, SDL_Rect* rect24,
                                      SDL_Surface* surface15, SDL_Rect* rect15  )
{
	int             x, y, x3, srcPitch, dstPitch;
	unsigned char*  src;
	unsigned short* dst;
	unsigned long   work;
	int             rect15x = rect15->x, rect15y = rect15->y, rect15w = rect15->w, rect15h = rect15->h, rect15wE; 
	int             rect24x = rect24->x, rect24y = rect24->y, rect24w = rect24->w, rect24h = rect24->h; 
	int             surface15w = surface15->w, surface15h = surface15->h;
	int             surface24w = surface24->w, surface24h = surface24->h;

	const unsigned char* aPixels = NULL;
	const unsigned char* bPixels = NULL;
	const unsigned char* swapPixels = NULL;
	
	const unsigned char copyPixels[256] = 
	{
		0,  0,  0,  0,  0,  0,  0,  0,  
		1,  1,  1,  1,  1,  1,  1,  1,  
		2,  2,  2,  2,  2,  2,  2,  2,  
		3,  3,  3,  3,  3,  3,  3,  3,  
		4,  4,  4,  4,  4,  4,  4,  4,  
		5,  5,  5,  5,  5,  5,  5,  5,  
		6,  6,  6,  6,  6,  6,  6,  6,  
		7,  7,  7,  7,  7,  7,  7,  7,  
		8,  8,  8,  8,  8,  8,  8,  8,  
		9,  9,  9,  9,  9,  9,  9,  9,  
		10, 10, 10, 10, 10, 10, 10, 10, 
		11, 11, 11, 11, 11, 11, 11, 11, 
		12, 12, 12, 12, 12, 12, 12, 12, 
		13, 13, 13, 13, 13, 13, 13, 13, 
		14, 14, 14, 14, 14, 14, 14, 14, 
		15, 15, 15, 15, 15, 15, 15, 15, 
		16, 16, 16, 16, 16, 16, 16, 16, 
		17, 17, 17, 17, 17, 17, 17, 17, 
		18, 18, 18, 18, 18, 18, 18, 18, 
		19, 19, 19, 19, 19, 19, 19, 19,  
		20, 20, 20, 20, 20, 20, 20, 20, 
		21, 21, 21, 21, 21, 21, 21, 21, 
		22, 22, 22, 22, 22, 22, 22, 22, 
		23, 23, 23, 23, 23, 23, 23, 23, 
		24, 24, 24, 24, 24, 24, 24, 24, 
		25, 25, 25, 25, 25, 25, 25, 25, 
		26, 26, 26, 26, 26, 26, 26, 26, 
		27, 27, 27, 27, 27, 27, 27, 27, 
		28, 28, 28, 28, 28, 28, 28, 28, 
		29, 29, 29, 29, 29, 29, 29, 29,  
		30, 30, 30, 30, 30, 30, 30, 30, 
		31, 31, 31, 31, 31, 31, 31, 31 
	};
	
	const unsigned char ditherPixels[256] = 
	{
		0,  0,  0,  0,  1,  1,  1,  1,  
		1,  1,  1,  1,  2,  2,  2,  2,  
		2,  2,  2,  2,  3,  3,  3,  3,  
		3,  3,  3,  3,  4,  4,  4,  4,  
		4,  4,  4,  4,  5,  5,  5,  5,  
		5,  5,  5,  5,  6,  6,  6,  6,  
		6,  6,  6,  6,  7,  7,  7,  7,  
		7,  7,  7,  7,  8,  8,  8,  8,  
		8,  8,  8,  8,  9,  9,  9,  9,  
		9,  9,  9,  9,  10, 10, 10, 10, 
		10, 10, 10, 10, 11, 11, 11, 11, 
		11, 11, 11, 11, 12, 12, 12, 12, 
		12, 12, 12, 12, 13, 13, 13, 13, 
		13, 13, 13, 13, 14, 14, 14, 14, 
		14, 14, 14, 14, 15, 15, 15, 15, 
		15, 15, 15, 15, 16, 16, 16, 16, 
		16, 16, 16, 16, 17, 17, 17, 17, 
		17, 17, 17, 17, 18, 18, 18, 18, 
		18, 18, 18, 18, 19, 19, 19, 19, 
		19, 19, 19, 19, 20, 20, 20, 20,  
		20, 20, 20, 20, 21, 21, 21, 21, 
		21, 21, 21, 21, 22, 22, 22, 22, 
		22, 22, 22, 22, 23, 23, 23, 23, 
		23, 23, 23, 23, 24, 24, 24, 24, 
		24, 24, 24, 24, 25, 25, 25, 25, 
		25, 25, 25, 25, 26, 26, 26, 26, 
		26, 26, 26, 26, 27, 27, 27, 27, 
		27, 27, 27, 27, 28, 28, 28, 28, 
		28, 28, 28, 28, 29, 29, 29, 29, 
		29, 29, 29, 29, 30, 30, 30, 30,  
		30, 30, 30, 30, 31, 31, 31, 31, 
		31, 31, 31, 31, 31, 31, 31, 31 
	};
	
	// Clip.
	if( rect15x < 0 )      { rect15w += rect15x; rect24w += rect15x; rect24x -= rect15x; rect15x = 0; }
	if( rect15y < 0 )      { rect15h += rect15y; rect24h += rect15y; rect24y -= rect15y; rect15y = 0; }
	if( (rect15x + rect15w) > surface15w ) { rect15w = surface15w - rect15x; }
	if( (rect15y + rect15h) > surface15h ) { rect15h = surface15h - rect15y; }

	if( rect24x < 0 )      { rect24w += rect24x; rect15w += rect24x; rect15x -= rect24x; rect24x = 0; }
	if( rect24y < 0 )      { rect24h += rect24y; rect15h += rect24y; rect15y -= rect24y; rect24y = 0; }
	if( (rect24x + rect24w) > surface24w ) { rect24w = surface24w - rect24x; }
	if( (rect24y + rect24h) > surface24h ) { rect24h = surface24h - rect24y; }

	// SDL changes the destination rectangle to the actual copied bounds. We need to do the same.
	if( rect15w <= 0 || rect15h <= 0 ) 
	{
		rect15->w = 0; rect15->h = 0;
		return;
	}

	rect15->x = rect15x; rect15->y = rect15y; rect15->w = rect15w; rect15->h = rect15h; 
	
	// Blit.
	srcPitch = surface24->pitch;
	dstPitch = surface15->pitch;
	
	src      = (unsigned char*)  ( ((unsigned char*) surface24->pixels) + (rect24x * 3) + (rect24y * srcPitch) );
	dst      = (unsigned short*) ( ((unsigned char*) surface15->pixels) + (rect15x * 2) + (rect15y * dstPitch) );
	rect15wE = rect15w & ~1;
	
	for( y=0; y<rect15h; y++ )
	{
		if( y & 1 )
		{
			aPixels = copyPixels;
			bPixels = ditherPixels;
		}
		else
		{
			aPixels = ditherPixels;
			bPixels = copyPixels;
		}
		
		x=0; x3=0;
		while( x < rect15wE )
		{
			work  = (aPixels[src[x3+0]] << 10) |
			        (aPixels[src[x3+1]] <<  5) |
			        (aPixels[src[x3+2]]      );
			       
			dst[x] = work;
			
			x++; x3+=3;
			
			work  = (bPixels[src[x3+0]] << 10) |
			        (bPixels[src[x3+1]] <<  5) |
			        (bPixels[src[x3+2]]      );
			       
			dst[x] = work;

			x++; x3+=3;
		}
		
		if( rect15w & 1 )
		{
			work  = (aPixels[src[x3+0]] << 10) |
			        (aPixels[src[x3+1]] <<  5) |
			        (aPixels[src[x3+2]]      );
			       
			dst[x] = work;
		}
		
		src = (unsigned char*)  ( ((unsigned char*) src) + srcPitch );
		dst = (unsigned short*) ( ((unsigned char*) dst) + dstPitch );
	}
}


static void SDLUi_SetGrayscaleColors( SDL_Surface* surface )
{
	SDL_Color  grayscalePalette[256];
	int        index;
	
	for( index=0; index<256; index++ )
	{
		grayscalePalette[index].r = 
		grayscalePalette[index].g = 
		grayscalePalette[index].b = 255 - index; 
		grayscalePalette[index].unused = 0; 
	}
	
	SDL_SetColors( surface, grayscalePalette, 0, 256 );
}


/*
GWorldPtr SDLU_SurfaceToGWorld( SDL_Surface* surface )
{
	OSErr     err;
	int       depth = 0;
	GWorldPtr gworld;
	
	switch( surface->format->BitsPerPixel )
	{
		case 1:
			depth = 1;
			break;
		
		case 15:
			depth = 16;
			break;
		
		default:
			Error( errUnknown, "\pSDLU_SurfaceToGWorld: depth" );
			break;
	}
	
	MRect mRect = { 0, 0, surface->h, surface->w };
	
	err = NewGWorldFromPtr( &gworld, depth, &mRect, NULL, NULL, 0, (Ptr) surface->pixels, surface->pitch );
	if( err != noErr )
	{
		Error( errUnknown, "\pSDLU_SurfaceToGWorld: NewGWorldFromPtr" );
	}
	
	return gworld;
}
*/


/*
void SDLU_DumpSurface( SDL_Surface* surface, const char* filenamePre, const char* filenamePost )
{
#if TARGET_API_MAC_CARBON
	OSErr     err;
	int       depth = 0;
	GWorldPtr gworld;
	
	switch( surface->format->BitsPerPixel )
	{
		case 1:
			depth = 1;
			break;
		
		case 15:
			depth = 16;
			break;

		case 16:
			depth = 16;
			break;
		
		default:
			Error( "SDLU_DumpSurface: depth" );
			break;
	}
	
	Rect mRect = { 0, 0, surface->h, surface->w };
	
	err = NewGWorldFromPtr( &gworld, depth, &mRect, NULL, NULL, 0, (Ptr) surface->pixels, surface->pitch );
	if( err != noErr )
	{
		Error( "SDLU_DumpSurface: NewGWorldFromPtr" );
	}

	CGrafPtr port; GDHandle gdh;
	GetGWorld( &port, &gdh );
	SetGWorld( gworld, NULL );
	PicHandle thePicture = OpenPicture( &mRect );
	CopyBits( GetPortBitMapForCopyBits(gworld), GetPortBitMapForCopyBits(gworld),
				&mRect,	&mRect,
				srcCopy, nil );
	ClosePicture( );
	SetGWorld( port, gdh );
	
	FSSpec filespec = {0};
	static int incr = 0;
	filespec.name[0] = sprintf( (char*) &filespec.name[1], "%s%d%s.jpg", filenamePre, incr++, filenamePost );
	
	OSType filetype = kQTFileTypeJPEG;
	OSType filecreator = 'TVOD';
	int    filescriptcode = smSystemScript;
	
	// see: http://developer.apple.com/quicktime/icefloe/dispatch014.html
	{
	    Handle h;
	    OSErr err;
	    GraphicsImportComponent gi = 0;
	    
	    // Convert the picture handle into a PICT file (still in a handle) 
	    // by adding a 512-byte header to the start.
	    h = NewHandleClear(512);
	    err = MemError();
	    if(err) goto bail;
	    err = HandAndHand((Handle)thePicture,h);
	    
	    err = OpenADefaultComponent(
	                GraphicsImporterComponentType,
	                kQTFileTypePicture,
	                &gi);
	    if(err) goto bail;
	    
	    err = GraphicsImportSetDataHandle(gi, h);
	    if(err) goto bail;
	    
	    err = GraphicsImportExportImageFile(
	                gi, 
	                filetype, 
	                filecreator, 
	                &filespec, 
	                filescriptcode);
	    if(err) goto bail;
	    
	bail:
	    if(gi) CloseComponent(gi);
	    if(h) DisposeHandle(h);
	}
	
	DisposeHandle( (Handle) thePicture );
	DisposeGWorld( gworld );
#endif
}
*/


SDL_Rect* SDLU_MRectToSDLRect( const MRect* in, SDL_Rect* out )
{
	int t = in->top, l = in->left, b = in->bottom, r = in->right;
	
	out->x = l;
	out->y = t;
	out->w = r - l;
	out->h = b - t; 
	
	return out;
}


MRect* SDLU_SDLRectToMRect( const SDL_Rect* in, MRect* out )
{
	int x = in->x, y = in->y, w = in->w, h = in->h;
	
	out->top    = y;
	out->left   = x;
	out->bottom = y + h;
	out->right  = x + w;
	
	return out; 
}


int SDLU_BlitSurface( SDL_Surface* src, SDL_Rect* srcrect,
			          SDL_Surface* dst, SDL_Rect* dstrect  )
{
	if( src->format->BitsPerPixel == 8 && dst->format->BitsPerPixel == 1 )
	{
		// SDL BUG!! SDL cannot blit 8-bit to 1-bit surfaces.
		SDLUi_Blit8BitTo1Bit( src, srcrect,
		                      dst, dstrect  );
		return 0;
	}

	// SDL PERFORMANCE: SDL sucks at blitting 15->16 and 16->15.
	// Use hand-rolled fast blitters to solve this.
	if( src->format->BitsPerPixel == 15 && dst->format->BitsPerPixel == 16 )
	{
		SDLUi_Blit15BitTo16Bit( src, srcrect,
		                        dst, dstrect  );
		return 0;	
	}
	
	
	if( src->format->BitsPerPixel == 16 && dst->format->BitsPerPixel == 15 )
	{
		SDLUi_Blit16BitTo15Bit( src, srcrect,
		                        dst, dstrect  );
		return 0;	
	}

	// Let SDL handle this.
	return SDL_BlitSurface( src, srcrect,
	                        dst, dstrect  );
}


int SDLU_BlitSurfaceHQ( SDL_Surface* src, SDL_Rect* srcrect,
			            SDL_Surface* dst, SDL_Rect* dstrect  )
{
	// QUALITY: Use a hand-rolled 24->15 dithering blitter.
	if( src->format->BitsPerPixel == 24 && dst->format->BitsPerPixel == 15 )
	{
		SDLUi_Blit24BitTo15BitHQ( src, srcrect,
		                          dst, dstrect  );
		return 0;	
	}
	
	// Let SDL handle this.
	return SDLU_BlitSurface( src, srcrect,
	                         dst, dstrect  );
}


void SDLU_GetPixel(	SDL_Surface* surface, int x, int y, SDL_Color* pixel )
{
	unsigned long px;
	unsigned char* ptr;
	
	switch( surface->format->BytesPerPixel )
	{
		case 1:
			ptr = (unsigned char*)surface->pixels + (y * surface->pitch) + (x);
			px = *(unsigned char*) ptr;
			break;
		
		case 2:
			ptr = (unsigned char*)surface->pixels + (y * surface->pitch) + (x * 2);
			px = *(unsigned short*) ptr;
			break;

		case 4:
			ptr = (unsigned char*)surface->pixels + (y * surface->pitch) + (x * 4);
			px = *(unsigned long*) ptr;
			break;
	}
	
	return SDL_GetRGB( px, surface->format, &pixel->r, &pixel->g, &pixel->b );
}


void SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth )
{
	SDL_Surface* newSurface;

	newSurface = SDLU_InitSurface( &surface[0]->clip_rect, depth );
	
	SDLU_BlitSurfaceHQ( *surface,    &surface[0]->clip_rect,
	                     newSurface, &newSurface->clip_rect  );
			
	SDL_FreeSurface( *surface );
	
	*surface = newSurface;
}


SDL_Surface* SDLU_InitSurface( SDL_Rect* rect, int depth )
{
	SDL_Surface*    surface = NULL;
	SDL_Color       k_oneBitPalette[2] = { { 0xFF, 0xFF, 0xFF, 0x00 },
	                                       { 0x00, 0x00, 0x00, 0x00 }  };
	
	switch( depth )
	{
		case 16:
			surface = SDL_CreateRGBSurface( 
							SDL_SWSURFACE, 
							rect->w, 
							rect->h, 
							15, 
							0x7C00, 0x03E0, 0x001F, 0x0000 );
			break;
		
		case 8:
			surface = SDL_CreateRGBSurface( 
							SDL_SWSURFACE, 
							rect->w, 
							rect->h, 
							8, 
							0, 0, 0, 0 );

			SDLUi_SetGrayscaleColors( surface );
			break;
		
		case 1:
			surface = SDL_CreateRGBSurface( 
							SDL_SWSURFACE, 
							rect->w, 
							rect->h, 
							1, 
							0, 0, 0, 0 );
			
			SDL_SetColors( surface, k_oneBitPalette, 0, 2 );
			break;
	}					
	
	if( surface == NULL )
	{
		Error( "SDLU_InitSurface: SDL_CreateRGBSurface" );
		return NULL;
	}
	
	// SDL BUG!! SDL_FillRect blows up with 1-depth surfaces. WORKAROUND: don't auto-clear them. Seems OK.
	//           (Next step is always to copy a PNG into them.)
	if( depth >= 8 )
		SDL_FillRect( surface, rect, SDL_MapRGB( surface->format, 0xFF, 0xFF, 0xFF ) );
	
	return surface;
}


void SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect )
{
	extern SDL_Surface* frontSurface;
	SDLU_BlitSurface( source,       sourceSDLRect,
	                  frontSurface, destSDLRect );
	
	SDL_UpdateRect( frontSurface, destSDLRect->x, destSDLRect->y, destSDLRect->w, destSDLRect->h );
}


void SDLU_SetBrightness( float b )
{
	Uint16 table[256];
	int    index;
	
	for( index=0; index<256; index++ )
	{
		table[index] = (int)(index * b * 257.0f); // 255 * 257 = 65535
	}
	
	SDL_SetGammaRamp( table, table, table );
}

void SDLU_Yield()
{
    SDL_Delay( 2 );
    SDL_PumpEvents();
}

void SDLU_PumpEvents()
{
	static unsigned long lastPump = 0;
	unsigned long time = MTickCount();
	
	if( lastPump != time )
	{
        SDL_Event evt;
        while( SDL_PollEvent( &evt ) ) { }
		lastPump = time;
	}
}


MBoolean SDLU_IsForeground()
{
    return s_isForeground;
}


int SDLU_EventFilter( const SDL_Event *event )
{
	// Put keydowns in a buffer
	if( event->type == SDL_KEYDOWN )
	{
		if(    s_interestedInTyping
		    && event->key.keysym.unicode <= 127 
		    && s_keyBufferFilled < sizeof(s_keyBufferASCII) )
		{
			s_keyBufferFilled++;
			s_keyBufferASCII[s_keyBufferPutAt] = event->key.keysym.unicode;
			s_keyBufferSDL  [s_keyBufferPutAt] = event->key.keysym.sym;
			s_keyBufferPutAt = (s_keyBufferPutAt + 1) % sizeof(s_keyBufferASCII);
		}

		if(    (event->key.keysym.sym == SDLK_F4)
		    && (event->key.keysym.mod & (KMOD_LALT | KMOD_RALT)) )
		{
			finished = true;
		}
		
		return 0;
	}
	
	// Get mouse state
	if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		if( event->button.button == SDL_BUTTON_LEFT )
			s_mouseButton = true;
		
		s_mousePosition.v = event->button.y;
		s_mousePosition.h = event->button.x;
		return 0;
	}
	
	if( event->type == SDL_MOUSEBUTTONUP )
	{
		if( event->button.button == SDL_BUTTON_LEFT )
			s_mouseButton = false;
	
		s_mousePosition.v = event->button.y;
		s_mousePosition.h = event->button.x;
		return 0;
	}

	if( event->type == SDL_MOUSEMOTION )
	{
		s_mousePosition.v = event->motion.y;
		s_mousePosition.h = event->motion.x;
		s_mouseButton = event->motion.state & SDL_BUTTON(1);
		return 0;
	}
 
	if( event->type == SDL_QUIT ) 
	{
		finished = true;
		return 0;
	}
	
	// Handle gaining and losing focus (kind of cheesy)
	if( event->type == SDL_ACTIVEEVENT && (event->active.state & SDL_APPINPUTFOCUS) )
	{
		if( !event->active.gain && s_isForeground )
		{
			FreezeGameTickCount();
			EnableMusic(false);
            s_isForeground = false;
		}
		else if( event->active.gain && !s_isForeground )
		{
			UnfreezeGameTickCount();
			EnableMusic(musicOn);
            s_isForeground = true;

			DoFullRepaint();
		}
	}
	
	// We never poll for events, we just process them here, so discard everything.
	return 0;
}


void SDLU_StartWatchingTyping()
{
	s_interestedInTyping = true;
	s_keyBufferFilled = s_keyBufferPullFrom = s_keyBufferPutAt = 0;
	SDL_EnableUNICODE( 1 );
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
}


void SDLU_StopWatchingTyping()
{
	s_interestedInTyping = false;
	SDL_EnableUNICODE( 0 );
	SDL_EnableKeyRepeat( 0, 0 );
}


MBoolean SDLU_CheckTyping( char* ascii, SDLKey* sdl )
{
	if( s_keyBufferFilled > 0 )
	{
		*ascii = s_keyBufferASCII[s_keyBufferPullFrom];
		*sdl   = s_keyBufferSDL  [s_keyBufferPullFrom];
		s_keyBufferPullFrom = (s_keyBufferPullFrom + 1) % sizeof(s_keyBufferASCII);
		s_keyBufferFilled--;
		return true;
	}
	
	*ascii = '\0';
	*sdl   = SDLK_UNKNOWN;
	return false;
}


void SDLU_GetMouse( MPoint* pt )
{
	SDLU_PumpEvents();
	*pt = s_mousePosition;
}


int SDLU_Button()
{
	SDLU_PumpEvents();
	return s_mouseButton;
}


void SDLU_AcquireSurface( SDL_Surface* surface )
{
	s_acquireList[++s_acquireHead] = surface;
}


SDL_Surface* SDLU_GetCurrentSurface()
{	
	return s_acquireList[s_acquireHead];
}


void SDLU_ReleaseSurface( SDL_Surface* surface )
{
	if( s_acquireList[s_acquireHead] != surface )
		Error( "SDLU_ReleaseSurface: out of order" );
		
	if( s_acquireHead < 0 )
		Error( "SDLU_ReleaseSurface: underflow" );
		
	s_acquireHead--;
}
