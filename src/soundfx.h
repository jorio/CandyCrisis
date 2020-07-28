// soundfx.h


void FMOD_ERRCHECK(int result);

void InitSound( void );
void PlayStereo( short player, short which );
void PlayStereoFrequency( short player, short which, short freq );
void PlayMono( short which );
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

namespace FMOD { class System; }

extern MBoolean         soundOn;
extern FMOD::System    *g_fmod;
