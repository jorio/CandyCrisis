// players.c

#include "stdafx.h"
#include "SDLU.h"

#include "main.h"
#include "players.h"
#include "gworld.h"
#include "moving.h"
#include "graphics.h"
#include "control.h"
#include "grays.h"
#include "soundfx.h"
#include "next.h"
#include "random.h"
#include "victory.h"
#include "tweak.h"
#include "zap.h"
#include "level.h"
#include "opponent.h"
#include "gameticks.h"
#include "blitter.h"
#include "music.h"
#include "score.h"
#include "hiscore.h"

#include <string.h>
#include <stdlib.h>

unsigned int  boredTime[2], hintTime[2], fadeCharTime[2], animTime[2], shadowDepth[2], hintGlow, messageTime;
int emotions[2];
int glowColors[][3] = { { 0,  0,  0},
						{_5TO8(13), _5TO8(26), _5TO8(31)},
						{_5TO8(13), _5TO8(29), _5TO8(13)},
						{_5TO8(31), _5TO8(18), _5TO8(31)},
						{_5TO8(31), _5TO8(14), _5TO8(18)},
						{_5TO8(31), _5TO8(31), _5TO8(15)},
						{_5TO8(31), _5TO8(21), _5TO8(13)},
						{_5TO8(30), _5TO8(22), _5TO8(30)},
						{_5TO8(20), _5TO8(20), _5TO8(20)} };

void HandlePlayers( void )
{
	int player;
	
	for( player = 0; player<=1; player++ )
	{
		UpdateScore( player );
		UpdateNext( player );
		BlinkGrays( player );
		GlowBlobs( player );
		BlinkBored( player );
		FadeCharred( player );

		switch( role[player] )
		{
			case kWaitForRetrieval:
				if( control[player] == kAutoControl )
				{
					AutoControl( player );
					break;
				}
				
				role[player] = kRetrieveBlobs;
				// fallthrough
				
			case kRetrieveBlobs:
				LockGrays( 1-player );
				RetrieveBlobs( player );
				break;
				
			case kFalling:
				Falling( player );
				break;
			
			case kLockdownBlobs:
				LockdownBlobs( player );
				break;
				
			case kJiggleBlobs:
				JiggleBlobs( player );
				break;
				
			case kFastJiggleBlobs:
				FastJiggleBlobs( player );
				break;
				
			case kPlaceBlobs:
				PlaceBlobs( player );
				break;
			
			case kDropBlobs:
				DropBlobs( player );
				break;
			
			case kZapBlobs:
				ZapBlobs( player );
				break;
			
			case kKillBlobs:
				KillBlobs( player );
				break;
			
			case kDropGrays:
				DropGrays( player );
				break;
			
			case kLosing:
				Lose( player );
				break;
			
			case kWinning:
				Win( player );
				break;
				
			case kChooseDifficulty:
				ChooseDifficulty( player );
				break;
			
			case kWaitingToStart:
				if( role[1-player] == kWaitingToStart )
					role[0] = role[1] = kZapBlobs;
				break;
			
			case kIdlePlayer:
				break;
		}
		
		UpdatePlayerWindow( player );
	}
}

void LockdownBlobs( int player )
{
	const int shadowList[] = { 
								 kBlobShadowDepth, 
								 kBlobShadowDepth*5/6, 
								 kBlobShadowDepth*5/6, 
								 kBlobShadowDepth*4/6, 
								 kBlobShadowDepth*3/6, 
								 kBlobShadowDepth*1/6, 
								};

	int frame = (GameTickCount( ) - blobTime[player]) / 2;

	if( frame >= arrsize(shadowList) )
	{
		role[player] = kPlaceBlobs;
		return;
	}
	
	shadowDepth[player] = shadowList[frame];
	UpdateTweak( player, blobJiggleAnimation );
}

void FastJiggleBlobs( int player )
{
	const int shadowList[] = { 
								 kBlobShadowDepth, 
								 kBlobShadowDepth*5/6, 
								 kBlobShadowDepth*5/6, 
								 kBlobShadowDepth*4/6, 
								 kBlobShadowDepth*3/6, 
								 kBlobShadowDepth*2/6, 
								 kBlobShadowDepth*1/6,
								 0 
								};

	int frame = (GameTickCount( ) - blobTime[player]) / 2;

	if( frame >= 8 )
	{
		role[player] = kPlaceBlobs;
		return;
	}

	switch( control[player] )
	{
		case kPlayerControl:
			PlayerControl( player );
			break;
		
		case kAIControl:
			AIControl( player );
			break;
		
		case kAutoControl:
			AutoControl( player );
			break;
	}
	
	shadowDepth[player] = shadowList[frame];
	UpdateTweak( player, blobJiggleAnimation );

	if( CanFall( player ) )
		role[player] = kFalling;
}


void JiggleBlobs( int player )
{
	const int shadowList[] = { 
								 kBlobShadowDepth, kBlobShadowDepth,
								 kBlobShadowDepth*5/6, kBlobShadowDepth*5/6,
								 kBlobShadowDepth*5/6, kBlobShadowDepth*4/6,
								 kBlobShadowDepth*4/6, kBlobShadowDepth*3/6,
								 kBlobShadowDepth*3/6, kBlobShadowDepth*2/6,
								 kBlobShadowDepth*2/6, kBlobShadowDepth*1/6,
								 kBlobShadowDepth*1/6, 0
								};

	int frame = (GameTickCount( ) - blobTime[player]) / 2;
	
	if( frame >= 14 )
	{
		role[player] = kPlaceBlobs;
		return;
	}

	switch( control[player] )
	{
		case kPlayerControl:
			PlayerControl( player );
			break;
		
		case kAIControl:
			AIControl( player );
			break;
			
		case kAutoControl:
			AutoControl( player );
			break;
	}

	shadowDepth[player] = shadowList[frame];
	UpdateTweak( player, kJiggleAnimation );
	
	if( CanFall( player ) )
		role[player] = kFalling;
}

void PlaceGrenade( int player )
{
	MRect myRect;
	int x, y, atX, atY, color, delay = -6;
	int currentX = blobX[player], currentY = blobY[player];
	int charTypes[3][5] = { { kDarkChar | kChar11, kDarkChar | kChar12, kDarkChar | kChar13, kDarkChar | kChar14, kNoCharring         },
	                        { kNoCharring,         kNoCharring,         kNoCharring,         kNoCharring,         kDarkChar | kChar24 },
	                        { kDarkChar | kChar31, kDarkChar | kChar32, kDarkChar | kChar33, kDarkChar | kChar34, kNoCharring         } };
	
	int amount = ((level <= kLevels)? level: 1) * 100;
	int multiplier = 0;
	
	grenadeFrame[player] = 0;
	grenadeRect [player].top    = (currentY * kBlobVertSize ) - (kBlastHeight/2);
	grenadeRect [player].left   = (currentX * kBlobHorizSize) + (kBlobHorizSize/2) - (kBlastWidth/2);
	grenadeRect [player].bottom = grenadeRect[player].top  + kBlastHeight;
	grenadeRect [player].right  = grenadeRect[player].left + kBlastWidth ;

	SDLU_AcquireSurface( playerSurface[player] );
	
	for( x=-1; x<=1; x++ )
	{
		atX = currentX + x;
		
		for( y=-2; y<=2; y++ )
		{
			atY = currentY + y;

			if( atX >= 0 && atX < kGridAcross &&
			    atY >= 0 && atY < kGridDown )
			{
				if( charTypes[x+1][y+2] != kNoCharring   &&
				    grid[player][atX][atY] >= kFirstBlob &&
					grid[player][atX][atY] <= kLastBlob     )
				{
					charred[player][atX][atY] =  charTypes[x+1][y+2];

					CalcBlobRect( atX, atY, &myRect );
					SurfaceDrawBlob( player, &myRect, grid[player][atX][atY], suction[player][atX][atY], charred[player][atX][atY] );
					CleanSpriteArea( player, &myRect );
				}
			}
		}
	}

	SDLU_ReleaseSurface( playerSurface[player] );
	
	if( currentY < (kGridDown-1) && 
		( grid[player][currentX][currentY+1] >= kFirstBlob && 
		  grid[player][currentX][currentY+1] <= kLastBlob    ) )
	{
		color = grid[player][currentX][currentY+1];
		
		for( x=0; x<kGridAcross; x++ )
		{
			for( y=0; y<kGridDown; y++ )
			{
				if( grid[player][x][y] == color )
				{
					suction[player][x][y] = kInDeath;
					death[player][x][y] = -abs( x - currentX + y - currentY );
					multiplier++;
					
					if( (x <= (kGridAcross-2)) && (grid[player][x+1][y] == kGray) )
					{
						suction[player][x+1][y] = kGrayBlink1;
						death[player][x+1][y] = delay;
					}

					if( (x >= 1) && (grid[player][x-1][y] == kGray) )
					{
						suction[player][x-1][y] = kGrayBlink1;
						death[player][x-1][y] = delay;
					}

					if( (y <= (kGridDown-2)) && (grid[player][x][y+1] == kGray) )
					{
						suction[player][x][y+1] = kGrayBlink1;
						death[player][x][y+1] = delay;
					}

					if( (y >= 1) && (grid[player][x][y-1] == kGray) )
					{
						suction[player][x][y-1] = kGrayBlink1;
						death[player][x][y-1] = delay;
					}
				}
			}
		}
	}
	else
	{
		for( x=currentX-1; x<=currentX+1; x++ )
		{
			for( y=currentY-1; y<=currentY+1; y++ )
			{
				if( x>=0 && x<kGridAcross && y>=0 && y<kGridDown )
				{
					if( grid[player][x][y] == kGray )
					{
						suction[player][x][y] = kGrayBlink1;
						death[player][x][y] = delay;
					}
					else if( grid[player][x][y] >= kFirstBlob &&
					         grid[player][x][y] <= kLastBlob     )
					{
						suction[player][x][y] = kInDeath;
						death[player][x][y] = -abs( x - currentX + y - currentY );
						multiplier++;
					}
				}
			}
		}
	}
	
	PlayStereo( player, kSplop );

	if( multiplier > 0 )
	{
		score[player] += amount * multiplier;	
		ZapScoreDisplay( player, amount, multiplier, blobX[player], blobY[player], 8 );
	}
	
	blobTime[player] = GameTickCount( );
	role[player] = kKillBlobs;
}

void PlaceBlobs( int player )
{
	MRect myRect;
	int x, y, height;
	int currentX = blobX[player], currentY = blobY[player];
	
	potentialCombo[player].x = currentX;
	potentialCombo[player].r = blobR[player];

	SDLU_AcquireSurface( playerSurface[player] );

	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( glow[player][x][y] )
			{
				glow[player][x][y] = false;
				CalcBlobRect( x, y, &myRect );
				SurfaceDrawBlob( player, &myRect, grid[player][x][y], suction[player][x][y], charred[player][x][y] );
				CleanSpriteArea( player, &myRect );
			}
		}
	}

	SDLU_ReleaseSurface( playerSurface[player] );

	EraseSpriteBlobs( player );
	
	CalcBlobRect( currentX, currentY, &myRect );
	CalcSecondBlobOffset( player, &x, &y );
	
	if( grenade[player] )
	{
		PlaceGrenade( player );	
		return;
	}
	
	if( magic[player] )
	{
		switch( blobR[player] )
		{
			case upRotate:
				height = GetRowHeight(player, currentX)-1;
				grid[player][currentX][height] = colorB[player];
				colorA[player] = BestColor( player, currentX, height+1 );
				grid[player][currentX][height] = kEmpty;
				break;
			
			case downRotate:
				height = GetRowHeight(player, currentX);
				grid[player][currentX][height] = colorB[player];
				colorA[player] = BestColor( player, currentX, height-1 );
				grid[player][currentX][height] = kEmpty;
				break;
			
			case rightRotate:
				height = GetRowHeight(player, currentX+1);
				grid[player][currentX+1][height] = colorB[player];
				colorA[player] = BestColor( player, currentX, GetRowHeight(player, currentX) );
				grid[player][currentX+1][height] = kEmpty;
				break;

			case leftRotate:
				height = GetRowHeight(player, currentX-1);
				grid[player][currentX-1][height] = colorB[player];
				colorA[player] = BestColor( player, currentX, GetRowHeight(player, currentX) );
				grid[player][currentX-1][height] = kEmpty;
				break;
		}
	}
	
	SDLU_AcquireSurface( playerSurface[player] );

	if( currentX >= 0 && currentX < kGridAcross &&
		currentY >= 0 && currentY < kGridDown )
	{	
		grid[player][currentX][currentY] = colorA[player];
		suction[player][currentX][currentY] = kNoSuction;
		charred[player][currentX][currentY] = kNoCharring;

		SurfaceDrawBlob( player, &myRect, colorA[player], kNoSuction, kNoCharring );
		CleanSpriteArea( player, &myRect );
	}
	
	OffsetMRect( &myRect, x * kBlobHorizSize, y * kBlobVertSize );
	currentX += x; 
	currentY += y;
	
	if( currentX >= 0 && currentX < kGridAcross &&
		currentY >= 0 && currentY < kGridDown )
	{
		grid[player][currentX][currentY] = colorB[player];
		suction[player][currentX][currentY] = kNoSuction;
		charred[player][currentX][currentY] = kNoCharring;
		
		SurfaceDrawBlob( player, &myRect, colorB[player], kNoSuction, kNoCharring );
		CleanSpriteArea( player, &myRect );
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
	
	blobTime[player] = GameTickCount( );
	halfway[player] = false;
	role[player] = kDropBlobs;
}

void DropBlobs( int player )
{
	MBoolean busy = false;
	signed char tempG[kGridDown], tempC[kGridDown];
	const int jiggleList[] = { kNoSuction, kSquish,
					      	   kNoSuction, kSquash,
						  	   kNoSuction, kSquish,
						  	   kNoSuction, kSquash };
	MRect myRect;
	int x, y;
	
	if( GameTickCount( ) < blobTime[player] )
		return;
	
	blobTime[player] += 1;
	
	halfway[player] = !halfway[player];
	
	SDLU_AcquireSurface( playerSurface[player] );
	
	if( halfway[player] )
	{
		for( x=0; x<kGridAcross; x++ )
		{
			for( y = 0; y<kGridDown; y++ )
			{
				tempG[y] = grid[player][x][y];
				tempC[y] = charred[player][x][y];
			}
			
			for( y = kGridDown-1; y; y-- )
			{
				if( tempG[y] == kEmpty && tempG[y-1] != kEmpty )
				{
					CalcBlobRect( x, y, &myRect );
					OffsetMRect( &myRect, 0, -kBlobVertSize/2 );
					
					if( tempG[y-1] == kGray )
					{
						SurfaceDrawBoard( player, &myRect );
						SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
					}
					else
					{
						SurfaceDrawBlob( player, &myRect, tempG[y-1], kNoSuction, tempC[y-1] );
					}
					
					tempG[y]   = tempG[y-1];
					tempG[y-1] = kEmpty;
					tempC[y]   = tempC[y-1];
					tempC[y-1] = kNoCharring;

					CleanSpriteArea( player, &myRect );
					
					OffsetMRect( &myRect, 0, -kBlobVertSize );
					SurfaceDrawBoard( player, &myRect );
					CleanSpriteArea( player, &myRect );
				}
			}
		}
	}
	else
	{
		for( x=0; x<kGridAcross; x++ )
		{
			for( y = kGridDown-1; y; y-- )
			{
				if( suction[player][x][y] >= kJiggle1 &&
					suction[player][x][y] <  kInDoubt )
				{
					CalcBlobRect( x, y, &myRect );
					SurfaceDrawBlob( player, &myRect, grid[player][x][y],
						jiggleList[ suction[player][x][y] - kJiggle1 ],
						charred[player][x][y] );
					CleanSpriteArea( player, &myRect );
					
					suction[player][x][y]++;
					
					busy = true;
				}
				else if( grid[player][x][y] == kEmpty && grid[player][x][y-1] != kEmpty )
				{
					grid[player][x][y] = grid[player][x][y-1];
					grid[player][x][y-1] = kEmpty;
					charred[player][x][y] = charred[player][x][y-1];
					charred[player][x][y-1] = kNoCharring;
					suction[player][x][y-1] = kNoSuction;
					
					CalcBlobRect( x, y, &myRect );
					if( grid[player][x][y] == kGray )
					{
						SurfaceDrawBoard( player, &myRect );
						SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
					}
					else
					{
						SurfaceDrawBlob( player, &myRect, grid[player][x][y], kNoSuction, charred[player][x][y] );
					}

					CleanSpriteArea( player, &myRect );
					
					OffsetMRect( &myRect, 0, -kBlobVertSize );
					SurfaceDrawBoard( player, &myRect );
					CleanSpriteArea( player, &myRect );
					
					if( grid[player][x][y] >= kFirstBlob && grid[player][x][y] <= kLastBlob )
					{
						if( y >= (kGridDown-1) || grid[player][x][y+1] != kEmpty )
						{
							suction[player][x][y] = kJiggle1;
						}
					}
					else
					{
						suction[player][x][y] = kNoSuction;
					}
					
					busy = true;
				}
			}
		}
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
	
	if( !busy && !halfway[player] )
	{
		ResolveSuction( player );
		role[player] = kZapBlobs;
	}
}

void RedrawBoardContents( int player )
{
	int x, y;
	MRect myRect;
	
	SDLU_AcquireSurface( playerSurface[player] );
	
	for( y=0; y<kGridDown; y++ )
	{
		for( x=0; x<kGridAcross; x++ )
		{
			CalcBlobRect( x, y, &myRect );
			if( grid[player][x][y] == kGray )
			{
				SurfaceDrawBoard( player, &myRect );
				SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
			}
			else
			{
				SurfaceDrawBlob( player, &myRect, grid[player][x][y], suction[player][x][y], charred[player][x][y] );
			}
			
			CleanSpriteArea( player, &myRect );
		}
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
}

void ResolveSuction( int player )
{
	int x, y, suck, actualSuck, color;
	MRect myRect;
	
	SDLU_AcquireSurface( playerSurface[player] );
	
	for( x=0; x<kGridAcross; x++ )
	{
		for( y=1; y<kGridDown; y++ )
		{
			suck = kNoSuction;
			color = grid[player][x][y];
			
			if( color >= kFirstBlob && color <= kLastBlob )
			{
				if( x > 0 )
					if( grid[player][x-1][y] == color ) suck |= kLeft;
				
				if( x < (kGridAcross-1) )
					if( grid[player][x+1][y] == color ) suck |= kRight;
					
				if( y > 1 )
					if( grid[player][x][y-1] == color ) suck |= kUp;
					
				if( y < (kGridDown-1) )
					if( grid[player][x][y+1] == color ) suck |= kDown;
				
				actualSuck = suction[player][x][y];
				if( actualSuck == kBlinkBlob || actualSuck == kSobBlob ) actualSuck = kNoSuction;
				
				if( actualSuck != suck )
				{
					suction[player][x][y] = suck;
					
					CalcBlobRect( x, y, &myRect );
					SurfaceDrawBlob( player, &myRect, grid[player][x][y], suck, charred[player][x][y] );
					CleanSpriteArea( player, &myRect );
				}
			}
			else
			{
				suction[player][x][y] = kNoSuction;
			}
		}
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
}

void HandleMagic( int player )
{
	if( magic[player] )
	{
		colorA[player]++;
		if( colorA[player] > kBlobTypes ) colorA[player] = 1;
	}
}

void Falling( int player )
{	
	if( role[1-player] == kLosing )
	{
		BeginVictory( player );
		return;
	}
	
	shadowDepth[player] = kBlobShadowDepth;
	
	UpdateTweak( player, kNoSuction );

	if( GameTickCount( ) >= blobTime[player] )
	{
		if( CanFall( player ) )
		{
			blobTime[player] += dropping[player]? kDropSpeed: speed[player];
			DoFall( player );
		}
		else
		{
			blobTime[player] = animTime[player] = GameTickCount( );
			role[player] = dropping[player]? kFastJiggleBlobs: kJiggleBlobs;
			anim[player] = 0;
			PlayStereoFrequency( player, kPlace, player );
			return;
		}
	}

	switch( control[player] )
	{
		case kPlayerControl:
			PlayerControl( player );
			break;
		
		case kAIControl:
			AIControl( player );
			break;

		case kAutoControl:
			AutoControl( player );
			break;
	}
}

void RetrieveBlobs( int player )
{
	if( (role[1-player] != kLosing) && (grid[player][2][1] != kEmpty) )
	{
		EndRound( player );
		return;
	}
	
	// See if it's time to update levels in Solitaire mode
	if( control[1] == kNobodyControl )
	{
		int levelClear[] = { 0,
		                     2500,
		                     7000,
		                     14000,
		                     35000,
		                     50000,
		                     70000,
		                     90000,
		                     120000,
		                     160000,
		                     200000,
		                     500000,
		                     1000000 };
		
		if( (level <= kLevels) && (score[player] > levelClear[level]) )
		{
			FreezeGameTickCount( );
			
			PlayMono( kLevelUp );
			
			level++;
			if( InitCharacter( 1, level ) )
			{
				InitCharacter( 1, level );
				character[0] = character[1];
				character[0].zapStyle = 0;
			}
			else
			{
				UnfreezeGameTickCount( );
				TotalVictory( );
				return;
			}

			if( level == 2 || level == 4 || level == 7 || level == 9 ) AddExtraPiece( );
			
			PrepareStageGraphics( character[1].picture );
			ChooseMusic( character[1].music );

			UnfreezeGameTickCount( );
		}		                     
	}
	
	PullNext( player );
	
	dropping[player] = false;
	anim[player] = 0;
	magic[player] = nextM[player];
	grenade[player] = nextG[player];
	zapIteration[player] = 0;
	chain[player] = 1;
	
	emotions[player] = DetermineEmotion(player);
	if( players == 1 && player == 0 ) 
	{
		if( emotions[0] == kEmotionPanic ) FastMusic(); else SlowMusic();
	}
	
	if( magic[player] || grenade[player] )
	{
		PlayStereoFrequency( player, kMagic, player );
	}
	
	colorA[player] = nextA[player];
	colorB[player] = nextB[player];
	
	nextG[player] = GetGrenade( player );

	if( nextG[player] )
	{
		nextA[player] = kBombBottom;
		nextB[player] = kBombTop;
		nextM[player] = false;
	}
	else
	{
		nextA[player] = GetPiece( player );
		nextB[player] = GetPiece( player );
		nextM[player] = GetMagic( player );
	}

	ChooseGlowingBlobs( player );
	
	if( control[player] == kPlayerControl )
	{
		memcpy( potentialCombo[player].grid, grid[player], kGridAcross * kGridDown );
		potentialCombo[player].a = colorA[player];
		potentialCombo[player].b = colorB[player];
		potentialCombo[player].m = magic[player];
		potentialCombo[player].g = grenade[player];
		potentialCombo[player].lv = level;
		potentialCombo[player].x = 0;
		potentialCombo[player].r = 0;
		potentialCombo[player].player = player;
		potentialCombo[player].value = 0;
		potentialCombo[player].name[0] = 0;
	}
	
	blobX[player] = 2;
	blobY[player] = 0;
	blobR[player] = upRotate;
	blobSpin[player] = 0;
	halfway[player] = false;
	
	DrawSpriteBlobs( player, kNoSuction );
	
	speed[player] = (signed char)(character[player].dropSpeed);
	blobTime[player] = animTime[player] = GameTickCount( );
	role[player] = kFalling;
	
	if( control[player] == kAIControl )
		ChooseAIDestination( player );
}

void ChooseGlowingBlobs( int player )
{
	int x, height[kGridAcross];
	
    if( character[player].hints && !grenade[player] && !magic[player] )
    {
		for( x=0; x<kGridAcross; x++ )
		{
			height[x] = GetRowHeight( player, x );
		}
				
		for( x=0; x<kGridAcross; x++ )
		{
			if( x>0 && height[x]>1 && height[x-1]>1 ) 
			{
				// left

				grid[player][x  ][height[x  ]] = colorA[player];
				grid[player][x-1][height[x-1]] = colorB[player];
				ConsiderGlow( player, colorA[player], x, height[x  ] );
				ConsiderGlow( player, colorB[player], x, height[x-1] );
				grid[player][x  ][height[x  ]] = kEmpty;
				grid[player][x-1][height[x-1]] = kEmpty;
			}

			if( x<(kGridAcross-1) && height[x]>1 && height[x+1]>1 ) 
			{
				// right

				grid[player][x  ][height[x  ]] = colorA[player];
				grid[player][x+1][height[x+1]] = colorB[player];
				ConsiderGlow( player, colorA[player], x, height[x  ] );
				ConsiderGlow( player, colorB[player], x, height[x+1] );
				grid[player][x  ][height[x  ]] = kEmpty;
				grid[player][x+1][height[x+1]] = kEmpty;
			}

			if( height[x]>2 ) 
			{
				// up

				grid[player][x][height[x]  ] = colorA[player];
				grid[player][x][height[x]-1] = colorB[player];
				ConsiderGlow( player, colorA[player], x, height[x]   );
				ConsiderGlow( player, colorB[player], x, height[x]-1 );
				grid[player][x][height[x]  ] = kEmpty;
				grid[player][x][height[x]-1] = kEmpty;

				// down

				grid[player][x][height[x]  ] = colorB[player];
				grid[player][x][height[x]-1] = colorA[player];
				ConsiderGlow( player, colorB[player], x, height[x]   );
				ConsiderGlow( player, colorA[player], x, height[x]-1 );
				grid[player][x][height[x]  ] = kEmpty;
				grid[player][x][height[x]-1] = kEmpty;
			}
		}
	}
}

void ConsiderGlow( int player, int color, int x, int y )
{
	if( GetChainSize( grid[player], x, y, color ) >= kBlobClusterSize )
	{
		CleanWithPolish( grid[player], glow[player], x, y, color );					
	}
	else
	{
		CleanSize( grid[player], x, y, color );
	}
}

void GlowBlobs( int player )
{
	int x, y, color, suck;
	MRect myRect;
	
	if( (!character[player].hints) || (GameTickCount() < hintTime[player]) )
		return;
		
	hintTime[player] += 3;
	if( ++hintGlow >= kGlowArraySize ) hintGlow = 0;

	SDLU_AcquireSurface( playerSurface[player] );

	for( x=0; x<kGridAcross; x++ )
	{
		if( rowBounce[player][x] == -1 )
		{
			for( y=0; y<kGridDown; y++ )
			{
				if( glow[player][x][y] && grid[player][x][y] != kEmpty )
				{
					CalcBlobRect( x, y, &myRect );	

					color = grid[player][x][y];
					suck = suction[player][x][y];
					
					SurfaceDrawBlob( player, &myRect, color, suck, charred[player][x][y] );
					SurfaceDrawColor( &myRect, color, suck, glowColors[color][0], glowColors[color][1], glowColors[color][2], glowArray[hintGlow] );
					CleanSpriteArea( player, &myRect );
				}
			}
		}
	}
	
	SDLU_ReleaseSurface( playerSurface[player] );
}

void FadeCharred( int player )
{
	int x, y;
	MRect myRect;
	
	if( GameTickCount() < fadeCharTime[player] || role[player] != kFalling ) return;
	
	fadeCharTime[player] += 135;
	
	SDLU_AcquireSurface( playerSurface[player] ); 
	
	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( charred[player][x][y] & 0xF0 )
			{
				charred[player][x][y] = ( charred[player][x][y] & 0x0F ) | ( ( charred[player][x][y] & 0xF0 ) - 0x10 );
				
				if( rowBounce[player][x] == -1 )
				{
					CalcBlobRect( x, y, &myRect );
					SurfaceDrawBlob( player, &myRect, grid[player][x][y], suction[player][x][y], charred[player][x][y] );
					CleanSpriteArea( player, &myRect );
				}
			}
		}
	}

	SDLU_ReleaseSurface( playerSurface[player] ); 
}

void BlinkBored( int player )
{
	MRect myRect;
	int which, x, y, count;
	
	if( GameTickCount() < boredTime[player] )
		return;
	
	SDLU_AcquireSurface( playerSurface[player] ); 
	
	// undo any previously bored blobs
	
	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( ( rowBounce[player][x] == -1 ) &&
				( suction[player][x][y] == kBlinkBlob || suction[player][x][y] == kSobBlob ) )
			{
				suction[player][x][y] = kNoSuction;
				
				CalcBlobRect( x, y, &myRect );
				SurfaceDrawBlob( player, &myRect, grid[player][x][y],
								   suction[player][x][y],
								   charred[player][x][y] );
				CleanSpriteArea( player, &myRect );
			}				
		}
	}
		
	// make some boredom

	which = RandomBefore( 2 );
	boredTime[player] += which? 15: 80;
	which = which? kBlinkBlob: kSobBlob;

	for( count=0; count<5; count++ )
	{
		x = RandomBefore( kGridAcross );
		y = RandomBefore( kGridDown );
		
		if( rowBounce[player][x] == -1 &&
			suction[player][x][y] == kNoSuction &&
			grid[player][x][y] >= kFirstBlob &&
			grid[player][x][y] <= kLastBlob   
		  ) 
		{
			suction[player][x][y] = which;
			
			CalcBlobRect( x, y, &myRect );
			SurfaceDrawBlob( player, &myRect,
					    grid[player][x][y],
					    suction[player][x][y],
					    charred[player][x][y] );
			CleanSpriteArea( player, &myRect );
		}
	}

	SDLU_ReleaseSurface( playerSurface[player] ); 
}

void InitPlayers( void )
{
	const double windowLoc[ ] = { kLeftPlayerWindowCenter, kRightPlayerWindowCenter };
	
	playerWindowZRect.top = playerWindowZRect.left = 0;
	playerWindowZRect.bottom = 288; playerWindowZRect.right = 144;
	
	playerWindowRect[0] = playerWindowRect[1] = playerWindowZRect;
	CenterRectOnScreen( &playerWindowRect[0], windowLoc[0], 0.5 );
	CenterRectOnScreen( &playerWindowRect[1], windowLoc[1], 0.5 );
}
