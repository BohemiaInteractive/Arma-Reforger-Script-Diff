/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Sound
\{
*/

class SoundManagerModule: SndBaseModule
{
	private void SoundManagerModule();

	// Plays a "fire and forget" sound effect instance. If more control is needed, use different solution
	proto external bool PlayAudioInstance(IEntity owner, AudioSourceConfig conf, array<string> names = null, array<float> values = null);
}

/*!
\}
*/
