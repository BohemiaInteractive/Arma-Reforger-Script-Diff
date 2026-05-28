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
	proto external bool PlayAudioInstanceByPos(vector pos, AudioSourceConfig conf, array<string> signalNames = null, array<float> signalValues = null, array<string> variableNames = null, array<float> variableValues = null);
	// Plays a "fire and forget" sound effect instance. If more control is needed, use different solution
	proto external bool PlayAudioInstance(IEntity owner, AudioSourceConfig conf, array<string> names = null, array<float> values = null, array<string> variableNames = null, array<float> variableValues = null);
}

/*!
\}
*/
