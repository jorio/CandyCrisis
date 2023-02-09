///
///  MTypes.c
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///


#include "MTypes.h"


void UnionMRect( const MRect* a, const MRect* b, MRect* u )
{
	u->top    = MinShort( a->top, b->top );
	u->left   = MinShort( a->left, b->left );
	u->bottom = MaxShort( a->bottom, b->bottom );
	u->right  = MaxShort( a->right, b->right );
}


void OffsetMRect( MRect* r, int x, int y )
{
	r->top += y;
	r->left += x;
	r->bottom += y;
	r->right += x;
}


unsigned char MPointInMRect( MPoint p, const MRect* r )
{
	return (p.h >= r->left) &&
	       (p.h <  r->right) &&
	       (p.v >= r->top) &&
	       (p.v <  r->bottom);
}
