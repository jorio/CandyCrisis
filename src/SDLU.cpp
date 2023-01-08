///
///  SDLU.c
///
///  SDL utilities.
///
///  John Stiles, 2002/10/12
///

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
static SDL_Palette* s_grayscalePalette;

// for button and getmouse
static int          s_mouseButton;
static MPoint       s_mousePosition;

// for event loop
static MBoolean     s_isForeground = true;

// for fade out / fade in
static float        s_fadeGamma = 1;
 
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
    s_fadeGamma = b;
}

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
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);

    SDL_UpdateTexture(g_windowTexture, NULL, g_frontSurface->pixels, g_frontSurface->pitch);
    SDL_RenderClear(g_renderer);
    
    SDL_RenderCopy(g_renderer, g_windowTexture, widescreen ? &g_widescreenCrop : NULL, NULL);

    if (s_fadeGamma < 1.0)
    {
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, (Uint8)((1.0f - s_fadeGamma) * 255.0f));
        SDL_RenderFillRect(g_renderer, NULL);
    }

    SDL_RenderPresent(g_renderer);

#if 0
    static int         s_fpsAccumulator = 0;
    static int         s_fpsSampleStart = 0;
    const int          k_fpsSampleInterval = 500;

    s_fpsAccumulator++;
    int now = SDL_GetTicks();
    int elapsed = now - s_fpsSampleStart;
    if (elapsed > k_fpsSampleInterval)
    {
        float fps = s_fpsAccumulator / (elapsed / 1000.0f);
#if _DEBUG
        printf("FPS: %.1f\n", fps);
#endif
        s_fpsAccumulator = 0;
        s_fpsSampleStart = now;
    }
#endif
}
