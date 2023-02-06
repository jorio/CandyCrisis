/////
///  MTypes.h
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///

#pragma once

#include <cstdint>

typedef signed char MBoolean;

typedef uint32_t MTicks;


struct MRGBColor
{
	unsigned short red;
	unsigned short green;
	unsigned short blue;
};


struct MRect
{
	short top;
	short left;
	short bottom;
	short right;	
};


struct MPoint
{
	short v;
	short h;
};


void UnionMRect( const MRect* a, const MRect* b, MRect* u );
void OffsetMRect( MRect* r, int x, int y );
unsigned char MPointInMRect( MPoint p, const MRect* r );

