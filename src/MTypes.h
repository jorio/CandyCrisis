/////
///  MTypes.h
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef signed char MBoolean;

typedef uint32_t MTicks;


typedef struct MRGBColor
{
	unsigned short red;
	unsigned short green;
	unsigned short blue;
} MRGBColor;


typedef struct MRect
{
	short top;
	short left;
	short bottom;
	short right;
} MRect;


typedef struct MPoint
{
	short v;
	short h;
} MPoint;


void UnionMRect( const MRect* a, const MRect* b, MRect* u );
void OffsetMRect( MRect* r, int x, int y );
unsigned char MPointInMRect( MPoint p, const MRect* r );


static inline short MinShort(short a, short b) { return a < b ? a : b; }
static inline short MaxShort(short a, short b) { return a < b ? b : a; }
static inline int MinInt(int a, int b) { return a < b ? a : b; }
static inline int MaxInt(int a, int b) { return a < b ? b : a; }
