/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

//! Effect used to rumble the gamepad.
class GamepadRumbleEffect: GamepadEffect
{
	//! Get fade-in time
	proto external int GetFadeInTimeMS();
	//! Get fade-out time
	proto external int GetFadeOutTimeMS();
	//! Get rumble amplitude. "saturate( Scale * (Base + Bias) )".
	proto external float GetAmplitude();
	//! Set Amplitude value
	proto external void SetAmplitudeBase(float a);
	//! Get rumble duration
	proto external int GetDurationMS();
	//! How many times the effect will loop
	proto external int GetLoopTimes();
	proto external bool IsInfiniteLooping();
}

/*!
\}
*/
