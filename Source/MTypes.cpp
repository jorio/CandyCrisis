///
///  MTypes.c
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///


#include "MTypes.h"


static inline short min( short a, short b )
{	
	return (a < b)? a: b;
}


static inline short max( short a, short b )
{	
	return (a > b)? a: b;
}


void UnionMRect( const MRect* a, const MRect* b, MRect* u )
{
	u->top    = min( a->top, b->top );
	u->left   = min( a->left, b->left );
	u->bottom = max( a->bottom, b->bottom );
	u->right  = max( a->right, b->right );
}


void OffsetMRect( MRect* r, int x, int y )
{
	r->top += y;
	r->left += x;
	r->bottom += y;
	r->right += x;
}


unsigned char MPointInMRect( MPoint p, MRect* r )
{
	return (p.h >= r->left) &&
	       (p.h <  r->right) &&
	       (p.v >= r->top) &&
	       (p.v <  r->bottom);
}
