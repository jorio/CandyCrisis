// blitter.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "gworld.h"
#include "blitter.h"
#include "font.h"
#include "level.h"
#include "graphics.h"

MBoolean update[2][kGridAcross][kGridDown];
MBoolean refresh[2];

void InitBlitter( void )
{
	int player, x, y;

	for( player=0; player<=1; player++ )
	{
		refresh[player] = false;
		
		for( x=0; x<kGridAcross; x++ )
		{
			for( y=0; y<kGridDown; y++ )
			{
				update[player][x][y] = false;
			}
		}
	}
}

void UpdatePlayerWindow( int player )
{
	SDL_Rect fullSDLRect, offsetSDLRect;
	int      x, y;
	
	if( control[player] == kNobodyControl ) return;
	
	if( playerWindowVisible[player] && refresh[player] )
	{
		MRect updateRect = {0, 0, 0, 0}, fullRect, offsetRect;
		MBoolean first = true;
		
		for( x=0; x<kGridAcross; x++ )
		{
			for( y=1; y<kGridDown; y++ )
			{
				if( update[player][x][y] )
				{
					updateRect.top  = y * kBlobVertSize;
					updateRect.left = x * kBlobHorizSize;
					updateRect.bottom = updateRect.top + kBlobVertSize;
					updateRect.right = updateRect.left + kBlobHorizSize;
					if( first )
					{
						fullRect = updateRect;
						first = false;
					}
					else
					{
						UnionMRect( &fullRect, &updateRect, &fullRect );
					}

					update[player][x][y] = false;
				}
			}
		}

		if( !first )
		{
			offsetRect = fullRect;
			OffsetMRect( &offsetRect, playerWindowRect[player].left, playerWindowRect[player].top - kBlobVertSize );
			
			SDLU_BlitFrontSurface( playerSpriteSurface[player], 
								   SDLU_MRectToSDLRect( &fullRect, &fullSDLRect ),
								   SDLU_MRectToSDLRect( &offsetRect, &offsetSDLRect ) );
		}
	}
}

void SetUpdateRect( int player, MRect *where )
{
	int x,y;
	int xMin, xMax, yMin, yMax;
	
	xMin = where->left / kBlobHorizSize;
	xMax = ( where->right + kBlobHorizSize - 1 ) / kBlobHorizSize;
	
	if( xMin < 0 ) xMin = 0;
	if( xMin > (kGridAcross-1) ) xMin = kGridAcross-1;
	if( xMax < 0 ) xMax = 0;
	if( xMax > kGridAcross ) xMax = kGridAcross;
	
	yMin = where->top / kBlobVertSize;
	yMax = ( where->bottom + kBlobVertSize - 1 ) / kBlobVertSize;

	if( yMin < 0 ) yMin = 0;
	if( yMin > (kGridDown-1) ) yMin = kGridDown-1;
	if( yMax < 0 ) yMax = 0;
	if( yMax > kGridDown ) yMax = kGridDown;
	
	for( x=xMin; x<xMax; x++ )
	{
		for( y=yMin; y<yMax; y++ )
		{
			update[player][x][y] = true;
		}
	}
	
	refresh[player] = true;
}


void SurfaceBlitMask( SDL_Surface* object,     SDL_Surface* mask,     SDL_Surface* dest,
                      const MRect*  objectRect, const MRect*  maskRect, const MRect*  destRect )
{
	int            startX = 0, startY = 0, endX, endY, x, y, srcRowBytes, mskRowBytes, dstRowBytes;
	unsigned int   bit, startBit, maskBits;
	unsigned char *src, *msk, *dst;
	MRect          destBounds;

	SDLU_SDLRectToMRect( &dest->clip_rect, &destBounds );
	
	endX = objectRect->right - objectRect->left;
	endY = objectRect->bottom - objectRect->top;

	if( destRect->left   > destBounds.right  ||			// completely clipped?
		destRect->right  < destBounds.left   ||
		destRect->top    > destBounds.bottom ||
		destRect->bottom < destBounds.top )
	{
		return; 												// do nothing
	}
	
	src         = (unsigned char*) object->pixels;
	msk         = (unsigned char*) mask->pixels;
	dst         = (unsigned char*) dest->pixels;
	srcRowBytes = object->pitch;
	mskRowBytes = mask->pitch;
	dstRowBytes = dest->pitch;
	
	src += (objectRect->top * srcRowBytes) + (objectRect->left * BYTES_PER_PIXEL);
	msk += (maskRect->top   * mskRowBytes) + (maskRect->left   / 8);
	dst += (destRect->top   * dstRowBytes) + (destRect->left   * BYTES_PER_PIXEL);
	
	if( destRect->left   < destBounds.left   ) startX -= destRect->left - destBounds.left;
	if( destRect->right  > destBounds.right  ) endX   -= destRect->right - destBounds.right;
	if( destRect->top    < destBounds.top    ) startY -= destRect->top - destBounds.top;
	if( destRect->bottom > destBounds.bottom ) endY   -= destRect->bottom - destBounds.bottom;
	
	startBit = 0x80000000 >> (startX & 31);
	msk += (mskRowBytes * startY) + ((startX & ~31) / 8);
	src += (srcRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	dst += (dstRowBytes * startY) + (startX * BYTES_PER_PIXEL);

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tDst = dst, *tMsk = msk;
		
		maskBits = SDL_SwapBE32( *(unsigned int *)msk );
		bit = startBit;
		
		for( x=startX; x<endX; x++ )
		{
			if( maskBits & bit )
			{
				*(COLOR_T*)dst = *(COLOR_T*)src;
			}
			
			if( !(bit >>= 1) ) { msk += 4; maskBits = SDL_SwapBE32( *(unsigned int *)msk ); bit = 0x80000000; }
			src += BYTES_PER_PIXEL; dst += BYTES_PER_PIXEL;
		}
		
		src = tSrc + srcRowBytes;
		dst = tDst + dstRowBytes;
		msk = tMsk + mskRowBytes;
	}
}


void SurfaceBlitBlob( const MRect* blobRect, const MRect* destRect )
{
	SurfaceBlitMask( blobSurface, maskSurface, SDLU_GetCurrentSurface(),
	                 blobRect,    blobRect,    destRect                  );
}


void SurfaceBlitColor( SDL_Surface* mask,     SDL_Surface* dest,
                       const MRect* maskRect, const MRect* destRect, 
                       int r, int g, int b, int weight )
{
	int            startX = 0, startY = 0, endX, endY, x, y, mskRowBytes, dstRowBytes;
	unsigned int   bit, startBit, maskBits;
	unsigned char *msk, *dst;
	MRect          destBounds;
	
	endX = maskRect->right - maskRect->left;
	endY = maskRect->bottom - maskRect->top;
	
	SDLU_SDLRectToMRect( &dest->clip_rect, &destBounds );

	if( destRect->left   > destBounds.right  ||			// completely clipped?
		destRect->right  < destBounds.left   ||
		destRect->top    > destBounds.bottom ||
		destRect->bottom < destBounds.top )
	{
		return; 												// do nothing
	}
	
	msk         = (unsigned char*) mask->pixels;
	dst         = (unsigned char*) dest->pixels;
	mskRowBytes = mask->pitch;
	dstRowBytes = dest->pitch;
	
	msk += (maskRect->top * mskRowBytes) + (maskRect->left / 8);
	dst += (destRect->top * dstRowBytes) + (destRect->left * BYTES_PER_PIXEL);
	
	if( destRect->left   < destBounds.left   ) startX -= destRect->left - destBounds.left;
	if( destRect->right  > destBounds.right  ) endX   -= destRect->right - destBounds.right;
	if( destRect->top    < destBounds.top    ) startY -= destRect->top - destBounds.top;
	if( destRect->bottom > destBounds.bottom ) endY   -= destRect->bottom - destBounds.bottom;
	
	startBit = 0x80000000 >> (startX & 31);
	msk += (mskRowBytes * startY) + ((startX & ~31) / 8);
	dst += (dstRowBytes * startY) + (startX * BYTES_PER_PIXEL);

	r *= weight;
	g *= weight;
	b *= weight;
	weight = FULL_WEIGHT - weight;
	
	for( y=startY; y<endY; y++ )
	{
		unsigned char *tMsk = msk, *tDst = dst;
		int work, workB, workG, workR;
		
		maskBits = SDL_SwapBE32( *(unsigned int *)msk );
		bit = startBit;
				
		for( x=startX; x<endX; x++ )
		{
			if( maskBits & bit )
			{
				work = *(COLOR_T*)dst;
                workB = ((((work                    ) & CHANNEL_MASK)*weight) + b) >> BITS_PER_1CHANNEL;
                workG = ((((work>>BITS_PER_1CHANNEL ) & CHANNEL_MASK)*weight) + g)                     ;
                workR = ((((work>>BITS_PER_2CHANNELS) & CHANNEL_MASK)*weight) + r) << BITS_PER_1CHANNEL;

                *(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
			}
			
			if( !(bit >>= 1) ) { msk += 4; maskBits = SDL_SwapBE32( *(unsigned int *)msk ); bit = 0x80000000; }
			dst += BYTES_PER_PIXEL;
		}
		
		msk = tMsk + mskRowBytes;
		dst = tDst + dstRowBytes;
	}
}


void SurfaceBlitAlpha( SDL_Surface* back,     SDL_Surface* source,     SDL_Surface* alpha,     SDL_Surface* dest,
                       const MRect* backRect, const MRect* sourceRect, const MRect* alphaRect, const MRect* destRect )
{
	int startX = 0, startY = 0, endX, endY, x, y, srcRowBytes, alfRowBytes, dstRowBytes, bckRowBytes;
	unsigned char *bck, *src, *alf, *dst;
	MRect destBounds;
	
	endX = sourceRect->right - sourceRect->left;
	endY = sourceRect->bottom - sourceRect->top;
	
	SDLU_SDLRectToMRect( &dest->clip_rect, &destBounds );

	if( destRect->left   > destBounds.right  ||			// completely clipped?
		destRect->right  < destBounds.left   ||
		destRect->top    > destBounds.bottom ||
		destRect->bottom < destBounds.top )
	{
		return; 												// do nothing
	}
	
	bck         = (unsigned char*) back->pixels;
	src         = (unsigned char*) source->pixels; 
	alf         = (unsigned char*) alpha->pixels;
	dst         = (unsigned char*) dest->pixels; 
	bckRowBytes = back->pitch; 
	srcRowBytes = source->pitch;
	alfRowBytes = alpha->pitch;
	dstRowBytes = dest->pitch;
	
	bck += (backRect->top   * bckRowBytes) + (backRect->left   * BYTES_PER_PIXEL);
	src += (sourceRect->top * srcRowBytes) + (sourceRect->left * BYTES_PER_PIXEL);
	alf += (alphaRect->top  * alfRowBytes) + (alphaRect->left  * BYTES_PER_PIXEL);
	dst += (destRect->top   * dstRowBytes) + (destRect->left   * BYTES_PER_PIXEL);
	
	if( destRect->left   < destBounds.left   ) startX -= destRect->left   - destBounds.left;
	if( destRect->right  > destBounds.right  ) endX   -= destRect->right  - destBounds.right;
	if( destRect->top    < destBounds.top    ) startY -= destRect->top    - destBounds.top;
	if( destRect->bottom > destBounds.bottom ) endY   -= destRect->bottom - destBounds.bottom;
	
	bck += (bckRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	src += (srcRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	alf += (alfRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	dst += (dstRowBytes * startY) + (startX * BYTES_PER_PIXEL);

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tAlf = alf, *tDst = dst, *tBck = bck;
		int pixS, pixB, weightS, weightB, workB, workG, workR;
		
		for( x=startX; x<endX; x++ )
		{
			weightB = *(COLOR_T*)alf;
			
			if( (weightB & (RED_MASK | GREEN_MASK | BLUE_MASK)) == (RED_MASK | GREEN_MASK | BLUE_MASK))
			{
				*(COLOR_T*)dst = *(COLOR_T*)bck;
			}
			else if( (weightB & (RED_MASK | GREEN_MASK | BLUE_MASK)) == 0)
			{
				*(COLOR_T*)dst = *(COLOR_T*)src;
			}
			else
			{
				weightS = ~weightB;

				pixS = *(COLOR_T*)src;
				pixB = *(COLOR_T*)bck;
								
				workB = ((((pixS                      ) & CHANNEL_MASK) * ((weightS                      ) & CHANNEL_MASK)) + (((pixB                      ) & CHANNEL_MASK) * ((weightB                      ) & CHANNEL_MASK))) >> BITS_PER_1CHANNEL;
				workG = ((((pixS >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * ((weightS >> BITS_PER_1CHANNEL ) & CHANNEL_MASK)) + (((pixB >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * ((weightB >> BITS_PER_1CHANNEL ) & CHANNEL_MASK)));
				workR = ((((pixS >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * ((weightS >> BITS_PER_2CHANNELS) & CHANNEL_MASK)) + (((pixB >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * ((weightB >> BITS_PER_2CHANNELS) & CHANNEL_MASK))) << BITS_PER_1CHANNEL;

				*(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
			}

			src += BYTES_PER_PIXEL;
			alf += BYTES_PER_PIXEL;
			bck += BYTES_PER_PIXEL;
            dst += BYTES_PER_PIXEL;
		}
		
		bck = tBck + bckRowBytes;
		src = tSrc + srcRowBytes;
		alf = tAlf + alfRowBytes;
		dst = tDst + dstRowBytes;
	}
}


void SurfaceBlitWeightedDualAlpha(SDL_Surface* back,     SDL_Surface* source,     SDL_Surface* mask,     SDL_Surface* alpha,     SDL_Surface* dest,
                                   const MRect* backRect, const MRect* sourceRect, const MRect* maskRect, const MRect* alphaRect, const MRect* destRect,
                                   int inWeight )
{
	int startX = 0, startY = 0, endX, endY, x, y,
	    srcRowBytes, alfRowBytes, mskRowBytes, dstRowBytes, bckRowBytes;
	unsigned int   bit, startBit, maskBits;
	unsigned char *bck, *src, *alf, *msk, *dst;
	MRect destBounds;

	endX = sourceRect->right - sourceRect->left;
	endY = sourceRect->bottom - sourceRect->top;
	
	SDLU_SDLRectToMRect( &dest->clip_rect, &destBounds );
	
	if( destRect->left   > destBounds.right  ||			// completely clipped?
		destRect->right  < destBounds.left   ||
		destRect->top    > destBounds.bottom ||
		destRect->bottom < destBounds.top )
	{
		return; 												// do nothing
	}
	
	bck = (unsigned char*) back->pixels;
	src = (unsigned char*) source->pixels;
	msk = (unsigned char*) mask->pixels;
	alf = (unsigned char*) alpha->pixels;
	dst = (unsigned char*) dest->pixels;
	
	bckRowBytes = back->pitch;
	srcRowBytes = source->pitch;
	mskRowBytes = mask->pitch;
	alfRowBytes = alpha->pitch;
	dstRowBytes = dest->pitch;
	
	bck += (backRect->top   * bckRowBytes) + (backRect->left   * BYTES_PER_PIXEL);
	src += (sourceRect->top * srcRowBytes) + (sourceRect->left * BYTES_PER_PIXEL);
	alf += (alphaRect->top  * alfRowBytes) + (alphaRect->left  * BYTES_PER_PIXEL);
	dst += (destRect->top   * dstRowBytes) + (destRect->left   * BYTES_PER_PIXEL);
	msk += (maskRect->top   * mskRowBytes) + (maskRect->left   / 8);
	
	if( destRect->left   < destBounds.left   ) startX -= destRect->left - destBounds.left;
	if( destRect->right  > destBounds.right  ) endX   -= destRect->right - destBounds.right;
	if( destRect->top    < destBounds.top    ) startY -= destRect->top - destBounds.top;
	if( destRect->bottom > destBounds.bottom ) endY   -= destRect->bottom - destBounds.bottom;
	
	bck += (bckRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	src += (srcRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	alf += (alfRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	dst += (dstRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	msk += (mskRowBytes * startY) + ((startX & ~31) / 8);
	startBit = 0x80000000 >> (startX & 31);
	
	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tAlf = alf, *tDst = dst, *tBck = bck, *tMsk = msk;
		int pixS, pixB, weightS, weightB, work, workB, workG, workR;
		
		maskBits = SDL_SwapBE32( *(unsigned int *)msk );
		bit = startBit;
		
		for( x=startX; x<endX; x++ )
		{
			if( maskBits & bit )
			{
				work = *(COLOR_T*)alf;
				workB = ((((work                    ) & CHANNEL_MASK)*inWeight) ) >> BITS_PER_1CHANNEL;
				workG = ((((work>>BITS_PER_1CHANNEL ) & CHANNEL_MASK)*inWeight) );
				workR = ((((work>>BITS_PER_2CHANNELS) & CHANNEL_MASK)*inWeight) ) << BITS_PER_1CHANNEL;

				weightB = ~((workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK));
				
                if( (weightB & (RED_MASK | GREEN_MASK | BLUE_MASK)) == (RED_MASK | GREEN_MASK | BLUE_MASK))
				{
					*(COLOR_T*)dst = *(COLOR_T*)bck;
				}
                else if( (weightB & (RED_MASK | GREEN_MASK | BLUE_MASK)) == 0)
				{
					*(COLOR_T*)dst = *(COLOR_T*)src;
				}
				else
				{
					weightS = ~weightB;

					pixS = *(COLOR_T*)src;
					pixB = *(COLOR_T*)bck;
									
					workB = ((((pixS                      ) & CHANNEL_MASK) * ((weightS                      ) & CHANNEL_MASK)) + (((pixB                      ) & CHANNEL_MASK) * ((weightB                      ) & CHANNEL_MASK))) >> BITS_PER_1CHANNEL;
					workG = ((((pixS >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * ((weightS >> BITS_PER_1CHANNEL ) & CHANNEL_MASK)) + (((pixB >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * ((weightB >> BITS_PER_1CHANNEL ) & CHANNEL_MASK)));
					workR = ((((pixS >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * ((weightS >> BITS_PER_2CHANNELS) & CHANNEL_MASK)) + (((pixB >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * ((weightB >> BITS_PER_2CHANNELS) & CHANNEL_MASK))) << BITS_PER_1CHANNEL;

					*(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
				}
			}

			if( !(bit >>= 1) ) { msk += 4; maskBits = SDL_SwapBE32( *(unsigned int *)msk ); bit = 0x80000000; }
			src += BYTES_PER_PIXEL;
			alf += BYTES_PER_PIXEL;
			bck += BYTES_PER_PIXEL;
			dst += BYTES_PER_PIXEL;
		}
		
		bck = tBck + bckRowBytes;
		src = tSrc + srcRowBytes;
		alf = tAlf + alfRowBytes;
		dst = tDst + dstRowBytes;
		msk = tMsk + mskRowBytes;
	}
}

void SurfaceBlitWeightedCharacter( SkittlesFontPtr font, unsigned char text, MPoint *dPoint, int r, int g, int b, int alpha )
{
	if (alpha == FULL_WEIGHT)
	{
		SurfaceBlitCharacter( font, text, dPoint, r, g, b, 0 );
        return;
	}
    
	if (alpha == 0)
	{
		return;
	}

    SDL_Surface*   destSurface;
    unsigned char* src;
    unsigned char* dst;
    int            srcRowBytes;
    int            dstRowBytes;
    int            index;
    MRect          destBounds;
    
    int height = font->surface->h;
    int width  = font->width[text];
    int across = font->across[text];
    
    destSurface = SDLU_GetCurrentSurface();
    SDLU_SDLRectToMRect( &destSurface->clip_rect, &destBounds );
    
    if( (dPoint->h + width)  > destBounds.right           ||      // clipped?
        (dPoint->v + height) > destBounds.bottom          || 
         dPoint->h           < destBounds.left            ||
         dPoint->v           < destBounds.top                )
    {
        dPoint->h += width;
        return;                                               // do nothing
    }
    
    srcRowBytes = font->surface->pitch;
    dstRowBytes = destSurface->pitch;
    
    src = (unsigned char*) font->surface->pixels + across;
    dst = (unsigned char*) destSurface->pixels + (dPoint->h * BYTES_PER_PIXEL) + (dPoint->v * dstRowBytes);
    
    while( height-- )
    {
        unsigned char *tSrc = src, *tDst = dst;
        
        for( index=0; index<width; index++ )
        {
            int workR, workG, workB, work, weightS, weightD;
            weightS = *src & CHANNEL_MASK;
            
            weightS = (weightS * alpha) >> BITS_PER_1CHANNEL;
            
            if( weightS )
            {
                weightD = FULL_WEIGHT - weightS;

                work = *(COLOR_T*)dst;
                workB = ((((work                    ) & CHANNEL_MASK) * weightD) + (b * weightS)) >> BITS_PER_1CHANNEL;
                workG = ((((work>>BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weightD) + (g * weightS));
                workR = ((((work>>BITS_PER_2CHANNELS) & CHANNEL_MASK) * weightD) + (r * weightS)) << BITS_PER_1CHANNEL;

                *(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
            }
            
            src++;
            dst+=BYTES_PER_PIXEL;
        }
        
        src = tSrc + srcRowBytes;
        dst = tDst + dstRowBytes;
    }
        
    dPoint->h += width;
}

void SurfaceBlitCharacter( SkittlesFontPtr font, unsigned char text, MPoint *dPoint, int r, int g, int b, int dropShadow )
{
    SDL_Surface*    destSurface;
	unsigned char*  src;
	unsigned char*  dst;
	int             srcRowBytes;
	int             dstRowBytes;
	int             index;
	int             rgbPremixed;
	MRect           destBounds;
	
	int height = font->surface->h;
	int width  = font->width[text];
	int across = font->across[text];
	
	destSurface = SDLU_GetCurrentSurface();
	SDLU_SDLRectToMRect( &destSurface->clip_rect, &destBounds );	
	
	if( (dPoint->h + width)  > destBounds.right           ||      // clipped?
	    (dPoint->v + height) > destBounds.bottom          || 
	     dPoint->h           < destBounds.left            ||
	     dPoint->v           < destBounds.top                )
	{
		dPoint->h += width;
		return;                                               // do nothing
	}
	
	srcRowBytes = font->surface->pitch;
	dstRowBytes = destSurface->pitch;
	
	src = (unsigned char*) font->surface->pixels + across;
	dst = (unsigned char*) destSurface->pixels + (dPoint->h * BYTES_PER_PIXEL) + (dPoint->v * dstRowBytes);
	rgbPremixed = (r << BITS_PER_2CHANNELS) | (g << BITS_PER_1CHANNEL) | b;
	
	switch( dropShadow )
	{
		case 0:
			while( height-- )
			{
				unsigned char *tSrc = src, *tDst = dst;
				
				for( index=0; index<width; index++ )
				{
					int workR, workG, workB, work, weightS, weightD;
					weightS = *src & CHANNEL_MASK;
					
					if( weightS == CHANNEL_MASK )
					{
						*(COLOR_T*)dst = rgbPremixed;
					}
					else if( weightS > 0 )
					{
						weightD = FULL_WEIGHT - weightS;

						work = *(COLOR_T*)dst;
						workB = ((((work                      ) & CHANNEL_MASK) * weightD) + (b * weightS)) >> BITS_PER_1CHANNEL;
						workG = ((((work >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weightD) + (g * weightS));
						workR = ((((work >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * weightD) + (r * weightS)) << BITS_PER_1CHANNEL;

						*(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
					}
					
					src++;
					dst+=BYTES_PER_PIXEL;
				}
				
				src = tSrc + srcRowBytes;
				dst = tDst + dstRowBytes;
			}
			break;
		
		default:
			{
				unsigned char *drp = dst + ((dstRowBytes + BYTES_PER_PIXEL) * dropShadow);
				
				while( height-- )
				{
					unsigned char *tSrc = src, *tDst = dst, *tDrp = drp;
					
					for( index=0; index<width; index++ )
					{
						int workR, workG, workB, work, weightS, weightD;
						weightS = *src & CHANNEL_MASK;
						
						if( weightS == CHANNEL_MASK )
						{
							*(COLOR_T*)drp = 0;
							*(COLOR_T*)dst = rgbPremixed;
						}
						else if( weightS > 0 )
						{
							weightD = FULL_WEIGHT - weightS;

							work = *(COLOR_T*)drp;
							workB = ((((work                    ) & CHANNEL_MASK) * weightD)) >> BITS_PER_1CHANNEL;
							workG = ((((work>>BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weightD));
							workR = ((((work>>BITS_PER_2CHANNELS) & CHANNEL_MASK) * weightD)) << BITS_PER_1CHANNEL;
							*(COLOR_T*)drp = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);

							work = *(COLOR_T*)dst;
							workB = ((((work                    ) & CHANNEL_MASK) * weightD) + (b * weightS)) >> BITS_PER_1CHANNEL;
							workG = ((((work>>BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weightD) + (g * weightS));
							workR = ((((work>>BITS_PER_2CHANNELS) & CHANNEL_MASK) * weightD) + (r * weightS)) << BITS_PER_1CHANNEL;
							*(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
						}
						
						src++;
						dst+=BYTES_PER_PIXEL;
						drp+=BYTES_PER_PIXEL;
					}
					
					src = tSrc + srcRowBytes;
					dst = tDst + dstRowBytes;
					drp = tDrp + dstRowBytes;
				}
				break;
			}
	}	
	dPoint->h += width;
}

void SurfaceBlitColorOver( SDL_Surface* source,     SDL_Surface* dest,
                           const MRect* sourceRect, const MRect* destRect,
                           int r, int g, int b, int weight )
{
    int            startX = 0, startY = 0, endX, endY, x, y, dstRowBytes, srcRowBytes;
	unsigned char* src;
	unsigned char* dst;
	MRect          destBounds;
	
	SDLU_SDLRectToMRect( &dest->clip_rect, &destBounds );
	
	endX = destRect->right - destRect->left;
	endY = destRect->bottom - destRect->top;
	
	if( destRect->left   > destBounds.right  ||			// completely clipped?
		destRect->right  < destBounds.left   ||
		destRect->top    > destBounds.bottom ||
		destRect->bottom < destBounds.top )
	{
		return; 												// do nothing
	}
	
	src         = (unsigned char*) source->pixels;
	dst         = (unsigned char*) dest->pixels;
	srcRowBytes = source->pitch;
	dstRowBytes = dest->pitch;
	
	src += (sourceRect->top * srcRowBytes) + (sourceRect->left * BYTES_PER_PIXEL);
	dst += (destRect->top * dstRowBytes) + (destRect->left * BYTES_PER_PIXEL);
	
	if( destRect->left   < destBounds.left   ) startX -= destRect->left - destBounds.left;
	if( destRect->right  > destBounds.right  ) endX   -= destRect->right - destBounds.right;
	if( destRect->top    < destBounds.top    ) startY -= destRect->top - destBounds.top;
	if( destRect->bottom > destBounds.bottom ) endY   -= destRect->bottom - destBounds.bottom;
	
	src += (srcRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	dst += (dstRowBytes * startY) + (startX * BYTES_PER_PIXEL);

	r *= weight;
	g *= weight;
	b *= weight;
	weight = FULL_WEIGHT - weight;
	
	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tDst = dst;
		int work, workB, workG, workR;
		
		for( x=startX; x<endX; x++ )
		{
			work = *(COLOR_T*)src;
			workB = ((((work                    ) & CHANNEL_MASK)*weight) + b) >> BITS_PER_1CHANNEL;
			workG = ((((work>>BITS_PER_1CHANNEL ) & CHANNEL_MASK)*weight) + g);
			workR = ((((work>>BITS_PER_2CHANNELS) & CHANNEL_MASK)*weight) + r) << BITS_PER_1CHANNEL;

			*(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
			
			src += BYTES_PER_PIXEL;
			dst += BYTES_PER_PIXEL;
		}
		
		src = tSrc + srcRowBytes;
		dst = tDst + dstRowBytes;
	}
}

// NOTE: due to rounding error, there should never be a case of transitioning all the way from 0 to 255.
// You'll overflow and get weird glitches on the edges.

void SurfaceBlitBlendOver(SDL_Surface* source,     SDL_Surface* dest,
                           const MRect* sourceRect, const MRect* destRect, 
                           int r1, int g1, int b1, 
                           int r2, int g2, int b2, 
                           int r3, int g3, int b3, 
                           int r4, int g4, int b4, 
                           int weight )
{
	int startX = 0, startY = 0, endX, endY, x, y;
	int rA, gA, bA;
	int rB, gB, bB;
	int rI, gI, bI;
	int vrLI, vgLI, vbLI;
	int vrRI, vgRI, vbRI;
	int weight12, ditherDiff;
	int width, height, dstRowBytes, srcRowBytes;
	unsigned char *src, *dst;
	MRect destBounds;
	MBoolean oddX = false;

	SDLU_SDLRectToMRect( &dest->clip_rect, &destBounds );
	
	if( destRect->left   > destBounds.right  ||			// completely clipped?
		destRect->right  < destBounds.left   ||
		destRect->top    > destBounds.bottom ||
		destRect->bottom < destBounds.top )
	{
		return; 												// do nothing
	}

	endX = destRect->right - destRect->left;
	endY = destRect->bottom - destRect->top;
	
	src         = (unsigned char*) source->pixels; 
	dst         = (unsigned char*) dest->pixels;
	srcRowBytes = source->pitch;
	dstRowBytes = dest->pitch;
	
	src += (sourceRect->top * srcRowBytes) + (sourceRect->left * BYTES_PER_PIXEL);
	dst += (destRect->top * dstRowBytes)   + (destRect->left * BYTES_PER_PIXEL);
	
	if( destRect->left   < destBounds.left   ) startX -= destRect->left - destBounds.left;
	if( destRect->right  > destBounds.right  ) endX   -= destRect->right - destBounds.right;
	if( destRect->top    < destBounds.top    ) startY -= destRect->top - destBounds.top;
	if( destRect->bottom > destBounds.bottom ) endY   -= destRect->bottom - destBounds.bottom;
	
	src += (srcRowBytes * startY) + (startX * BYTES_PER_PIXEL);
	dst += (dstRowBytes * startY) + (startX * BYTES_PER_PIXEL);

	height = endY - startY;
	width  = endX - startX;
	
	weight12 = weight << 12;
	
	r1 *= weight12;		g1 *= weight12;		b1 *= weight12;
	r2 *= weight12;		g2 *= weight12;		b2 *= weight12;
	r3 *= weight12;		g3 *= weight12;		b3 *= weight12;
	r4 *= weight12;		g4 *= weight12;		b4 *= weight12;
	ditherDiff = weight12 >> 1;
	
	vrLI = (r3 - r1) / height;
	vgLI = (g3 - g1) / height;
	vbLI = (b3 - b1) / height;

	vrRI = (r4 - r2) / height;
	vgRI = (g4 - g2) / height;
	vbRI = (b4 - b2) / height;

	weight = FULL_WEIGHT - weight;
    weight12 = weight << 12;

    if( (endX - startX) & 1 )
	{
		endX--;
		oddX = true;
	}
	
	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tDst = dst;
		int work, workB, workG, workR;
		
		if( y & 1 )
		{
			rA =  r1;
			gA =  g1;
			bA =  b1;
			rB =  r1 - ditherDiff; if( rB < 0 ) rB = 0;
			gB =  g1 - ditherDiff; if( gB < 0 ) gB = 0;
			bB =  b1 - ditherDiff; if( bB < 0 ) bB = 0;
		}
		else
		{
			rA =  r1 - ditherDiff; if( rA < 0 ) rA = 0;
			gA =  g1 - ditherDiff; if( gA < 0 ) gA = 0;
			bA =  b1 - ditherDiff; if( bA < 0 ) bA = 0;
			rB =  r1;
			gB =  g1;
			bB =  b1;
		}
		
		rI = (2 * (r2 - r1)) / width;
		gI = (2 * (g2 - g1)) / width;
		bI = (2 * (b2 - b1)) / width;
					
		for( x=startX; x<endX; x+=2 )
		{
			work = *(COLOR_T*)src;

            workB = ((((work                      ) & CHANNEL_MASK) * weight12) + bA) >> (12 + BITS_PER_1CHANNEL);
            workG = ((((work >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weight12) + gA) >> (12                    );
            workR = ((((work >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * weight12) + rA) >> (12 - BITS_PER_1CHANNEL);

            *(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
			
			src+= BYTES_PER_PIXEL;
			dst+= BYTES_PER_PIXEL;
			rA += rI; 
			gA += gI;
			bA += bI;

			work = *(COLOR_T*)src;

            workB = ((((work                      ) & CHANNEL_MASK) * weight12) + bB) >> (12 + BITS_PER_1CHANNEL);
            workG = ((((work >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weight12) + gB) >> (12                    );
            workR = ((((work >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * weight12) + rB) >> (12 - BITS_PER_1CHANNEL);

            *(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
			
            src+= BYTES_PER_PIXEL;
            dst+= BYTES_PER_PIXEL;
			rB += rI;
			gB += gI;
			bB += bI;
		}
		
		if( oddX )
		{
			work = *(COLOR_T*)src;
            
            workB = ((((work                      ) & CHANNEL_MASK) * weight12) + bA) >> (12 + BITS_PER_1CHANNEL);
            workG = ((((work >> BITS_PER_1CHANNEL ) & CHANNEL_MASK) * weight12) + gA) >> (12                    );
            workR = ((((work >> BITS_PER_2CHANNELS) & CHANNEL_MASK) * weight12) + rA) >> (12 - BITS_PER_1CHANNEL);

            *(COLOR_T*)dst = (workR & RED_MASK) | (workG & GREEN_MASK) | (workB & BLUE_MASK);
		}
		
		src = tSrc + srcRowBytes;
		dst = tDst + dstRowBytes;

		r1 += vrLI; r2 += vrRI;
		g1 += vgLI; g2 += vgRI;
		b1 += vbLI; b2 += vbRI;
	}
}

