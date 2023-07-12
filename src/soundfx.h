// soundfx.h


void InitSound();
void ShutdownSound();
void PlayStereo( int player, short which );
void PlayStereoFrequency( int player, short which, short freq );
void PlayMono( int which );
void UpdateSound();

enum
{
	kShift = 0,
	kRotate,
	kPlace,
	kSquishy,
	kBounce,
	kSplop,
	kWhistle,
	kPause,
	kLoss,
	kVictory,
	kMagic,
	kWhomp,
	kChime,
	kClick,
	kLevelUp,
	kContinueSnd,
	kBatsuSnd,
	kNumSounds
};

extern MBoolean         soundOn;
