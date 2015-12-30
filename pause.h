// pause.h

void HandleDialog( int type );
void SurfaceGetEdges( SDL_Surface* edgeSurface, const MRect *rect );
void SurfaceCurveEdges( SDL_Surface* edgeSurface, const MRect *rect );

enum
{
	kPauseDialog = 0,
	kHiScoreDialog,
	kContinueDialog,
	kControlsDialog,
	kNumDialogs
};
