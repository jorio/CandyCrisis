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

// for acquiresurface
#define k_acquireMax 10
static int          s_acquireHead = -1;
static SDL_Surface* s_acquireList[k_acquireMax];

// for initsurface
static SDL_Palette* s_grayscalePalette;

// for button and getmouse
static int          s_mouseButton;
static MPoint       s_mousePosition;

// system mouse cursors
static SDL_Cursor*  s_standardCursor = NULL;
static SDL_Cursor*  s_handCursor = NULL;

// for event loop
static MBoolean     s_isForeground = true;

// for fade out / fade in
static float        s_fadeGamma = 1;
 
// for checktyping
typedef struct BufferedKey
{
    bool isASCII;
    
    union
    {
        char         ascii;
        SDL_Keycode  keycode;
    } value;
} BufferedKey;

#define k_maxBufferedKeys 256

static MBoolean                s_interestedInTyping = false;
static BufferedKey             s_keyBuffer[k_maxBufferedKeys];
static int                     s_keyBufferSize = 0;

bool SDLUi_EventFilter(void* junk, SDL_Event *event)
{
	(void) junk;
	
    switch (event->type)
    {
		case SDL_EVENT_TEXT_INPUT:
        {
            // Put text input into a buffer.
            if (s_interestedInTyping)
            {
                for (char* asciiPtr = event->text.text; *asciiPtr; ++asciiPtr)
                {
                    BufferedKey key;
                    key.isASCII = true;
                    key.value.ascii = *asciiPtr;
                    s_keyBuffer[s_keyBufferSize] = key;
                    s_keyBufferSize = MinInt(k_maxBufferedKeys, s_keyBufferSize + 1);
                }
            }
            break;
        }
    
		case SDL_EVENT_KEY_DOWN:
        {
            // Put keydowns in a buffer
            if (s_interestedInTyping)
            {
                BufferedKey key;
                key.isASCII = false;
				key.value.keycode = event->key.key;
                s_keyBuffer[s_keyBufferSize] = key;
                s_keyBufferSize = MinInt(k_maxBufferedKeys, s_keyBufferSize + 1);
            }
            break;
        }
    
        // Get mouse state
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            if( event->button.button == SDL_BUTTON_LEFT )
                s_mouseButton = true;
            
			SDL_ConvertEventToRenderCoordinates(g_renderer, event);
            s_mousePosition.v = event->button.y;
            s_mousePosition.h = event->button.x;
            break;
        }

		case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            if( event->button.button == SDL_BUTTON_LEFT )
                s_mouseButton = false;
            
			SDL_ConvertEventToRenderCoordinates(g_renderer, event);
            s_mousePosition.v = event->button.y;
            s_mousePosition.h = event->button.x;
            break;
        }

		case SDL_EVENT_MOUSE_MOTION:
        {
			SDL_ConvertEventToRenderCoordinates(g_renderer, event);
            s_mousePosition.v = event->motion.y;
            s_mousePosition.h = event->motion.x;
            s_mouseButton = event->motion.state & SDL_BUTTON_LMASK;
            break;
        }

		case SDL_EVENT_QUIT:
        {
            finished = true;
            break;
        }

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			if (s_isForeground)
			{
				FreezeGameTickCount();
				//EnableMusic(false);
				s_isForeground = false;
			}
			break;
		
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			if (!s_isForeground)
			{
				UnfreezeGameTickCount();
				//EnableMusic(musicOn);
				s_isForeground = true;
				
				DoFullRepaint();
			}
			break;
		
		case SDL_EVENT_WINDOW_RESIZED:
		{
			SDLU_CreateRendererTexture();
			break;
		}
	}

	return true;
}


void SDLU_CreateRendererTexture()
{
	if (!g_renderer)
		return;
	
	int logicalWidth = 640;
	int logicalHeight = widescreen ? 360 : 480;
	
	SDL_RendererLogicalPresentation logicalPresentation = SDL_LOGICAL_PRESENTATION_LETTERBOX;
	SDL_ScaleMode scaleMode = SDL_SCALEMODE_LINEAR;
	
	if (crispUpscaling)
	{
		int currentWidth = 0;
		int currentHeight = 0;
		SDL_GetWindowSizeInPixels(g_window, &currentWidth, &currentHeight);
		
		if (currentWidth >= logicalWidth && currentHeight >= logicalHeight)
		{
			logicalPresentation = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
			scaleMode = SDL_SCALEMODE_NEAREST;
		}
	}

	if (g_windowTexture)
		SDL_DestroyTexture(g_windowTexture);

	g_windowTexture = SDL_CreateTexture(g_renderer,
		SDL_PIXELFORMAT_XRGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		640, 480);

	SDL_SetTextureScaleMode(g_windowTexture, scaleMode);
	SDL_SetRenderLogicalPresentation(g_renderer, logicalWidth, logicalHeight, logicalPresentation);
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

    s_grayscalePalette = SDL_CreatePalette(256);
    bool ok = SDL_SetPaletteColors(s_grayscalePalette, grayscaleColors, 0, arrsize(grayscaleColors));
	SDL_assert(ok);

    s_standardCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    s_handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
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
	return SDL_BlitSurface( src, srcrect, dst, dstrect );
}


int SDLU_BlitSurface1to1( SDL_Surface* src, SDL_Surface* dst )
{
	SDL_Rect srcrect, dstrect;
	SDL_GetSurfaceClipRect( src, &srcrect );
	SDL_GetSurfaceClipRect( dst, &dstrect );
	return SDL_BlitSurface( src, &srcrect,
							dst, &dstrect  );
}


void SDLU_GetPixel(	SDL_Surface* surface, int x, int y, SDL_Color* pixel )
{
	unsigned int   px;
	unsigned char* ptr;
	
	switch( SDL_BYTESPERPIXEL(surface->format) )
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
	
	SDL_GetRGB( px, SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), &pixel->r, &pixel->g, &pixel->b );
}


void SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth )
{
	SDL_Surface* oldSurface = *surface;
	SDL_Surface* newSurface;
	SDL_Rect clipRect;
	
	SDL_GetSurfaceClipRect( oldSurface, &clipRect );

	newSurface = SDLU_InitSurface( &clipRect, depth );
	
	SDLU_BlitSurface1to1( oldSurface, newSurface );

	SDL_DestroySurface( oldSurface );
	
	*surface = newSurface;
}


SDL_Surface* SDLU_InitSurface( SDL_Rect* rect, int depth )
{
	SDL_Surface*            surface = NULL;
	bool ok = true;
	
	switch( depth )
	{
		case 32:
			surface = SDL_CreateSurface(rect->w, rect->h, SDL_PIXELFORMAT_XRGB8888);
			break;

		case 8:
			surface = SDL_CreateSurface(rect->w, rect->h, SDL_PIXELFORMAT_INDEX8);
			SDL_assert(surface);
			ok = SDL_SetSurfacePalette(surface, s_grayscalePalette);
			SDL_assert(ok);
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
	{
		ok = SDL_FillSurfaceRect( surface, rect, SDL_MapSurfaceRGB( surface, 0xFF, 0xFF, 0xFF ) );
		SDL_assert(ok);
	}
	
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
    s_keyBufferSize = 0;  // clear keybuffer
	SDL_StartTextInput(g_window);
}


void SDLU_StopWatchingTyping()
{
	s_interestedInTyping = false;
	SDL_StopTextInput(g_window);
}


MBoolean SDLU_CheckASCIITyping(char* ascii)
{
    if (s_keyBufferSize > 0 && s_keyBuffer[0].isASCII)
	{
        *ascii = s_keyBuffer[0].value.ascii;

        s_keyBufferSize--;
        SDL_memcpy(&s_keyBuffer[0], &s_keyBuffer[1], (s_keyBufferSize) * sizeof(BufferedKey));

		return true;
	}

	*ascii = '\0';
	return false;
}


MBoolean SDLU_CheckSDLTyping(SDL_Keycode* sdlKey)
{
    if (s_keyBufferSize > 0 && !s_keyBuffer[0].isASCII)
    {
        *sdlKey = s_keyBuffer[0].value.keycode;

        s_keyBufferSize--;
        SDL_memcpy(&s_keyBuffer[0], &s_keyBuffer[1], (s_keyBufferSize) * sizeof(BufferedKey));

        return true;
    }
    
    *sdlKey = SDLK_UNKNOWN;
    return false;
}


void SDLU_GetMouse( MPoint* pt )
{
	SDLU_PumpEvents();
	*pt = s_mousePosition;
	if (widescreen)
	{
		pt->h += g_widescreenCrop.x;
		pt->v += g_widescreenCrop.y;
	}
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
    
	SDL_FRect wsc = {.x=g_widescreenCrop.x, .y=g_widescreenCrop.y, .w=g_widescreenCrop.w, .h=g_widescreenCrop.h};
	SDL_RenderTexture(g_renderer, g_windowTexture, widescreen ? &wsc : NULL, NULL);

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


void SDLU_SetSystemCursor(int which)
{
#if USE_CURSOR_SPRITE
	SDL_HideCursor();
#else
    switch (which)
    {
        case SYSTEM_CURSOR_OFF:
			SDL_HideCursor();
            SDL_SetCursor(s_standardCursor);
            break;

        case SYSTEM_CURSOR_ARROW:
            SDL_SetCursor(s_standardCursor);
			SDL_ShowCursor();
            break;

        case SYSTEM_CURSOR_HAND:
            SDL_SetCursor(s_handCursor);
			SDL_ShowCursor();
            break;
    }
#endif
}
