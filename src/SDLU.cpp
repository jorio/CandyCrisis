///
///  SDLU.c
///
///  SDL utilities.
///
///  John Stiles, 2002/10/12
///

#include "stdafx.h"
#include "SDLU.h"
#include "gameticks.h"
#include "music.h"

#include "main.h" // for Error
#include <deque>

using std::deque;

// for acquiresurface
const int           k_acquireMax = 10;
static int          s_acquireHead = -1;
static SDL_Surface* s_acquireList[k_acquireMax];

// for initsurface
static SDL_Palette* s_oneBitPalette;
static SDL_Palette* s_grayscalePalette;

// for button and getmouse
static int          s_mouseButton;
static MPoint       s_mousePosition;

// for event loop
static MBoolean     s_isForeground = true;
 
// for checktyping
struct BufferedKey
{
    bool isASCII;
    
    union
    {
        char         ascii;
        SDL_Keycode  keycode;
    } value;
};


static MBoolean                s_interestedInTyping = false;
static std::deque<BufferedKey> s_keyBuffer;


int SDLUi_EventFilter(void*, SDL_Event *event)
{
    switch (event->type)
    {
        case SDL_TEXTINPUT:
        {
            // Put text input into a buffer.
            if (s_interestedInTyping)
            {
                for (char* asciiPtr = event->text.text; *asciiPtr; ++asciiPtr)
                {
                    BufferedKey key;
                    key.isASCII = true;
                    key.value.ascii = *asciiPtr;
                    s_keyBuffer.push_back(key);
                }
            }
            break;
        }
    
        case SDL_KEYDOWN:
        {
            // Put keydowns in a buffer
            if (s_interestedInTyping)
            {
                BufferedKey key;
                key.isASCII = false;
                key.value.keycode = event->key.keysym.sym;
                s_keyBuffer.push_back(key);
            }
            break;
        }
    
        // Get mouse state
        case SDL_MOUSEBUTTONDOWN:
        {
            if( event->button.button == SDL_BUTTON_LEFT )
                s_mouseButton = true;
            
            s_mousePosition.v = event->button.y;
            s_mousePosition.h = event->button.x;
            break;
        }
    
        case SDL_MOUSEBUTTONUP:
        {
            if( event->button.button == SDL_BUTTON_LEFT )
                s_mouseButton = false;
            
            s_mousePosition.v = event->button.y;
            s_mousePosition.h = event->button.x;
            break;
        }
    
        case SDL_MOUSEMOTION:
        {
            s_mousePosition.v = event->motion.y;
            s_mousePosition.h = event->motion.x;
            s_mouseButton = event->motion.state & SDL_BUTTON(1);
            break;
        }
    
        case SDL_QUIT:
        {
            finished = true;
            break;
        }
            
        case SDL_WINDOWEVENT:
        {
            if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST && s_isForeground)
            {
                FreezeGameTickCount();
                EnableMusic(false);
                s_isForeground = false;
            }
            else if (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED && !s_isForeground)
            {
                UnfreezeGameTickCount();
                EnableMusic(musicOn);
                s_isForeground = true;
                
                DoFullRepaint();
            }
            break;
        }
    }
    
    return 1;
}


void SDLU_Init()
{
    SDL_SetEventFilter(SDLUi_EventFilter, NULL);

    // Initialize one bit palette.
    static const SDL_Color  k_oneBitColors[2] = { { 0xFF, 0xFF, 0xFF, 0x00 },
                                                  { 0x00, 0x00, 0x00, 0x00 }  };

    s_oneBitPalette = SDL_AllocPalette(2);
    
    SDL_SetPaletteColors(s_oneBitPalette, k_oneBitColors, 0, arrsize(k_oneBitColors));

    // Initialize eight bit grayscale ramp palette.
    SDL_Color  grayscaleColors[256];
    for (int index=0; index<256; index++)
    {
        grayscaleColors[index].r =
        grayscaleColors[index].g =
        grayscaleColors[index].b = 255 - index;
        grayscaleColors[index].a = 255;
    }
    
    s_grayscalePalette = SDL_AllocPalette(256);
    SDL_SetPaletteColors(s_grayscalePalette, grayscaleColors, 0, arrsize(grayscaleColors));
}


static void	SDLUi_Blit8BitTo1Bit( SDL_Surface* surface8, SDL_Rect* rect8,
                                  SDL_Surface* surface1, SDL_Rect* rect1  )
{
	// NOTE: for now this copy assumes that we're copying the whole thing.
	// That's true for everything I'm doing.
	
	int          x, y, across, down;
	SDL_Color*   palette8;

	(void) rect8; // is unused for now
	
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
   
	// Let SDL handle this.
	return SDL_BlitSurface( src, srcrect,
	                        dst, dstrect  );
}


void SDLU_GetPixel(	SDL_Surface* surface, int x, int y, SDL_Color* pixel )
{
	unsigned int   px;
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
			px = *(unsigned int *) ptr;
			break;
        
        default:
            Error("SDLU_GetPixel: unrecognized surface format");
            return;
	}
	
	return SDL_GetRGB( px, surface->format, &pixel->r, &pixel->g, &pixel->b );
}


void SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth )
{
	SDL_Surface* newSurface;

	newSurface = SDLU_InitSurface( &surface[0]->clip_rect, depth );
	
	SDLU_BlitSurface( *surface,    &surface[0]->clip_rect,
	                   newSurface, &newSurface->clip_rect  );
			
	SDL_FreeSurface( *surface );
	
	*surface = newSurface;
}


SDL_Surface* SDLU_InitSurface( SDL_Rect* rect, int depth )
{
	SDL_Surface*            surface = NULL;
	
	switch( depth )
	{
        case 32:
            surface = SDL_CreateRGBSurface(
                            SDL_SWSURFACE,
                            rect->w,
                            rect->h, 
                            32,
                            RED_MASK, GREEN_MASK, BLUE_MASK, 0);
            break;
          
		case 8:
			surface = SDL_CreateRGBSurface( 
							SDL_SWSURFACE, 
							rect->w, 
							rect->h, 
							8, 
							0, 0, 0, 0 );

            SDL_SetSurfacePalette(surface, s_grayscalePalette);
			break;
		
		case 1:
			surface = SDL_CreateRGBSurface( 
							SDL_SWSURFACE, 
							rect->w, 
							rect->h, 
							1, 
							0, 0, 0, 0 );
			
            SDL_SetSurfacePalette(surface, s_oneBitPalette);
			break;
            
        default:
            Error("SDLU_InitSurface: invalid depth");
            return NULL;
	}					
	
	if( surface == NULL )
	{
		Error( "SDLU_InitSurface: SDL_CreateRGBSurface" );
		return NULL;
	}
	
	// SDL_FillRect only works on 8-bit or higher surfaces.
	if( depth >= 8 )
		SDL_FillRect( surface, rect, SDL_MapRGB( surface->format, 0xFF, 0xFF, 0xFF ) );
	
	return surface;
}


void SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect )
{
	SDLU_BlitSurface( source,       sourceSDLRect,
	                  g_frontSurface, destSDLRect );
}


void SDLU_SetBrightness( float b )
{
/*	Uint16 table[256];
	int    index;
	
	for( index=0; index<256; index++ )
	{
		table[index] = (int)(index * b * 257.0f); // 255 * 257 = 65535
	}
	
	SDL_SetWindowGammaRamp(g_window, table, table, table);
*/}

void SDLU_Yield()
{
    SDL_Delay( 2 );
    SDL_PumpEvents();
}

void SDLU_PumpEvents()
{
	static unsigned int  lastPump = 0;
	unsigned int  time = MTickCount();
	
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


void SDLU_StartWatchingTyping()
{
	s_interestedInTyping = true;
    s_keyBuffer.clear();
}


void SDLU_StopWatchingTyping()
{
	s_interestedInTyping = false;
}


MBoolean SDLU_CheckASCIITyping(char* ascii)
{
    if (!s_keyBuffer.empty() && s_keyBuffer.front().isASCII)
	{
        *ascii = s_keyBuffer.front().value.ascii;
        s_keyBuffer.pop_front();
		return true;
	}

	*ascii = '\0';
	return false;
}


MBoolean SDLU_CheckSDLTyping(SDL_Keycode* sdlKey)
{
    if (!s_keyBuffer.empty() && !s_keyBuffer.front().isASCII)
    {
        *sdlKey = s_keyBuffer.front().value.keycode;
        s_keyBuffer.pop_front();
        return true;
    }
    
    *sdlKey = SDLK_UNKNOWN;
    return false;
}


static MPoint SDLUi_TranslatePointFromWindowToFrontSurface(MPoint pt)
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(g_window, &windowWidth, &windowHeight);
    
    pt.h = pt.h * g_frontSurface->w / windowWidth;
    pt.v = pt.v * g_frontSurface->h / windowHeight;

    return pt;
}


void SDLU_GetMouse( MPoint* pt )
{
	SDLU_PumpEvents();
	*pt = SDLUi_TranslatePointFromWindowToFrontSurface(s_mousePosition);
}


int SDLU_Button()
{
	SDLU_PumpEvents();
	return s_mouseButton;
}


void SDLU_AcquireSurface( SDL_Surface* surface )
{
	if (s_acquireHead >= arrsize(s_acquireList) - 1)
		Error("SDLU_AcquireSurface: overflow");

	s_acquireList[++s_acquireHead] = surface;
}


SDL_Surface* SDLU_GetCurrentSurface()
{	
	return s_acquireList[s_acquireHead];
}


void SDLU_ReleaseSurface( SDL_Surface* surface )
{
    if (s_acquireHead < 0)
        Error( "SDLU_ReleaseSurface: underflow" );
		
    if( s_acquireList[s_acquireHead] != surface )
		Error( "SDLU_ReleaseSurface: out of order" );
		
	s_acquireHead--;
}


void SDLU_Present()
{
    SDL_UpdateTexture(g_windowTexture, NULL, g_frontSurface->pixels, g_frontSurface->pitch);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_windowTexture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
}
