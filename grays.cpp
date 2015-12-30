// grays.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "grays.h"
#include "players.h"
#include "graphics.h"
#include "gworld.h"
#include "control.h"
#include "soundfx.h"
#include "score.h"
#include "random.h"
#include "graymonitor.h"
#include "gameticks.h"
#include "blitter.h"
#include "zap.h"
#include "level.h"
#include "opponent.h"

int grays[2][kGridAcross], grayAir[2][kGridAcross];
int unallocatedGrays[2], lockGrays[2], rowBounce[2][kGridAcross], splat[2][kGridAcross];
int blinkTime[2], blinkStage[2];
const int blinkList[] = { kGrayNoBlink, kGrayBlink1,  kGrayBlink2, 
  						  kGrayBlink3,  kGrayBlink2,  kGrayBlink1   };

void InitGrays( void )
{
	int player, gridX;
	
	blinkTime[0] = GameTickCount( );
	blinkTime[1] = GameTickCount( ) + 60;
	
	for( player=0; player<=1; player++ )
	{
		blinkStage[player] = 0;
		unallocatedGrays[player] = 0;
		
		for( gridX=0; gridX<kGridAcross; gridX++ )
		{
			grays[player][gridX] = 0;
			rowBounce[player][gridX] = -1;
		}
	}	
}

void BlinkGrays( int player )
{
	int x, y, currentBlinkGraphic;
	MRect myRect;
	
	if( blinkTime[player] > GameTickCount( ) )
		return;
	
	blinkTime[player] += 2;
	blinkStage[player]++;
	
	if( blinkStage[player] > 5 )
	{
		blinkStage[player] = 0;
		blinkTime[player] = GameTickCount( ) + kTimeBetweenBlinks;
	}
	
    currentBlinkGraphic = blinkList[ blinkStage[player] ];

    SDLU_AcquireSurface( playerSurface[player] );
    
    for( x=0; x<kGridAcross; x++ )
    {
        if( rowBounce[player][x] == -1 )
        {
            for( y=kGridDown-1; y>=0; y-- )
            {			
                if( (grid[player][x][y] == kGray) &&
                    (suction[player][x][y] == kGrayNoBlink) )
                {
                    CalcBlobRect( x, y, &myRect );
                    SurfaceDrawBoard( player, &myRect );
                    SurfaceDrawAlpha( &myRect, kGray, kLight, currentBlinkGraphic );
                    CleanSpriteArea( player, &myRect );
                }					
            }
        }
    }
    
    SDLU_ReleaseSurface( playerSurface[player] );
}

void CalculateGrays( int player, int blobsToDrop )
{
	if( blobsToDrop < unallocatedGrays[1-player] )
	{
		unallocatedGrays[1-player] -= blobsToDrop;
		lockGrays[1-player] -= blobsToDrop;
		if( lockGrays[1-player] < 0 ) lockGrays[1-player] = 0;
		
		blobsToDrop = 0;
		ShowGrayMonitor( 1-player );
	}
	else
	{
		blobsToDrop -= unallocatedGrays[1-player];
		unallocatedGrays[1-player] = 0;
		lockGrays[1-player] = 0;
		ShowGrayMonitor( 1-player );
		
		unallocatedGrays[player] += blobsToDrop;
		ShowGrayMonitor( player );
	}
}

void LockGrays( int player )
{
	lockGrays[player] = unallocatedGrays[player];
}

void SetupGrays( int player )
{
	int grayX, change;
	MBoolean onlyOnce[kGridAcross];
	int rowFree[kGridAcross];
	
	if( role[player] == kDropGrays ) return; // next time around
	
	for( grayX=0; grayX < kGridAcross; grayX++ )
	{
		grayAir[player][grayX] = -RandomBefore(kBlobVertSize*3/2);
		rowFree[grayX] = -1;
		
		while( grid[player][grayX][rowFree[grayX]+1] == kEmpty && 
			   rowFree[grayX] < (kGridDown-1) )
			rowFree[grayX]++;
	}
	
	while( lockGrays[player] >= kGridAcross )
	{
		change = 0;
		
		for( grayX=0; grayX < kGridAcross; grayX++ )
		{
			if( rowFree[grayX] >= 0 )
			{
				grays[player][grayX]++;
				//grayAir[player][grayX] -= kBlobVertSize;
				rowFree[grayX]--;
				change++;
			}
		}
		
		lockGrays[player] -= change;
		unallocatedGrays[player] -= change;
		
		if( change == 0 ) break;
	}
	
	if( lockGrays[player] > 0 )
	{
		for( grayX = 0; grayX < kGridAcross; grayX++ )
		{
			onlyOnce[grayX] = rowFree[grayX] > 0;
		}
		
		while( (onlyOnce[0] || onlyOnce[1] || onlyOnce[2] ||
			    onlyOnce[3] || onlyOnce[4] || onlyOnce[5]) &&
			   (lockGrays[player] > 0) )
		{
			grayX = RandomBefore(kGridAcross);
			
			if( onlyOnce[grayX] )
			{
				grays[player][grayX]++;
				//grayAir[player][grayX] -= kBlobVertSize;
				lockGrays[player]--;
				unallocatedGrays[player]--;
				onlyOnce[grayX] = false;
			}
		}
	}
}

void DropGrays( int player )
{
	MRect myRect;
	int count, grayX;
	
	if( blobTime[player] > GameTickCount( ) )
		return;
		
	blobTime[player]++;
	
	for( grayX = 0; grayX < kGridAcross; grayX++ )
	{
		if( grays[player][grayX] > 0 )
		{
			myRect.bottom = grayAir[player][grayX];
			myRect.left = kBlobHorizSize * grayX;
			myRect.top = myRect.bottom - (kBlobVertSize * grays[player][grayX]);
			myRect.right = myRect.left + kBlobHorizSize;
			CleanSpriteArea( player, &myRect );
			
			grayAir[player][grayX] += 4;
			if( grayAir[player][grayX] > 0 ) grayAir[player][grayX] += grayAir[player][grayX] / 12;
			
			if( ( grayAir[player][grayX] / kBlobVertSize ) >= GetRowHeight( player, grayX ) )
			{
				PlaceGrayRow( player, grayX );
				ShowGrayMonitor( player );
			}
			else
			{
				myRect.top = grayAir[player][grayX];
				myRect.bottom = myRect.top + kBlobVertSize;
				
				SDLU_AcquireSurface( playerSpriteSurface[player] );

				for( count=0; count<grays[player][grayX]; count++ )
				{
					OffsetMRect( &myRect, 0, -kBlobVertSize );
					CleanSpriteArea( player, &myRect );
					SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );					
				}

				SDLU_ReleaseSurface( playerSpriteSurface[player] );
			}
		}
	}
	
	Bounce( player );
		
	if( !BusyDroppingGrays( player ) )
	{
		role[player] = kWaitForRetrieval;
	}
}

MBoolean BusyDroppingGrays( int player )
{
	int grayX;
	
	for( grayX = 0; grayX<kGridAcross; grayX++ )
	{
		if( rowBounce[player][grayX] >= 0 ||
			grays[player][grayX] > 0 ) 
		{
			return true;
		} 
	}

	return false;
}

void PlaceGrayRow( int player, int grayX )
{
	int grayY;
	MRect myRect;

	if( player == 1 ) OpponentPissed( );
	
	rowBounce[player][grayX] = 0;
	splat[player][grayX] = GetRowHeight( player, grayX )+1;
	
	SDLU_AcquireSurface( playerSurface[player] );
	
	while( grays[player][grayX] > 0 )
	{
		grayY = GetRowHeight( player, grayX );
				
		grid[player][grayX][grayY] = kGray;
		suction[player][grayX][grayY] = kGrayNoBlink;
		
		CalcBlobRect( grayX, grayY, &myRect );
		SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
		CleanSpriteArea( player, &myRect );
		
		grays[player][grayX]--;
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
	
	PlayStereoFrequency( player, kBounce, player );
}

void Bounce( int player )
{
	int x, y, bounce, suck, blob, rows, currentBlinkGraphic;
	MRect blobRect;
	double blobTop, compress;
	const double compressList[kZapFrames+1] = { 
							 -kBlobVertSize + 0.83,
							 -kBlobVertSize + 1.58,
							 -kBlobVertSize + 2.22,
							 -kBlobVertSize + 2.76,
							 -kBlobVertSize + 3.20,
							 -kBlobVertSize + 3.55, 
							 -kBlobVertSize + 3.80,
							 -kBlobVertSize + 3.95,  
							 -kBlobVertSize + 4.00,
							 -kBlobVertSize + 3.95,  
							 -kBlobVertSize + 3.80,
							 -kBlobVertSize + 3.55, 
							 -kBlobVertSize + 3.20,
							 -kBlobVertSize + 2.76,
							 -kBlobVertSize + 2.22,
							 -kBlobVertSize + 1.58,
							 -kBlobVertSize + 0.83,
							 -kBlobVertSize,
							 -kBlobVertSize,
							 -kBlobVertSize,
							 -kBlobVertSize
						   };
	
    currentBlinkGraphic = blinkList[ blinkStage[player] ];
    
    SDLU_AcquireSurface( playerSpriteSurface[player] );
    
    for( x=0; x<kGridAcross; x++ )
    {
        if( rowBounce[player][x] >= 0 ) CleanSplat( player, x, splat[player][x], rowBounce[player][x] );
    }
    
    for( x=0; x<kGridAcross; x++ )
    {
        bounce = rowBounce[player][x];
        if( bounce >= 0 )
        {
            compress = compressList[bounce];
            rows = GetRowHeight( player, x );
            
            CalcBlobRect( x, rows, &blobRect );
            blobRect.bottom = kBlobVertSize * kGridDown;
            SurfaceDrawBoard( player, &blobRect );
            SetUpdateRect( player, &blobRect );
            
            blobRect.top = kBlobVertSize * (kGridDown-1);
            blobTop = kBlobVertSize * (kGridDown-1);
            
            for( y=kGridDown-1; y>=rows; y-- )
            {
                suck = suction[player][x][y];
                blob = grid[player][x][y];
                if( suck == kNoSuction && blob >= kFirstBlob &&
                    blob <= kLastBlob && compress > (-kBlobVertSize + 3) )
                {
                    suck = kSquish;
                }
                
                if( blob == kGray )	SurfaceDrawAlpha( &blobRect, kGray, kLight, currentBlinkGraphic );
                else SurfaceDrawBlob( player, &blobRect, blob, suck, charred[player][x][y] );
                
                blobTop += compress;
                blobRect.top = (short) blobTop;
                blobRect.bottom = blobRect.top + kBlobVertSize;
            }
        }
    }

    for( x=0; x<kGridAcross; x++ )
    {
        if( rowBounce[player][x] >= 0 ) DrawSplat( player, x, splat[player][x], rowBounce[player][x] );
    }
    
    SDLU_ReleaseSurface( playerSpriteSurface[player] );
	
	for( x=0; x<kGridAcross; x++ )
	{
		if( rowBounce[player][x] >= 0 && rowBounce[player][x] < (kZapFrames) )
			rowBounce[player][x]++;
		else
			rowBounce[player][x] = -1;
	}
}
