/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

class GamepadTriggerEffect: GamepadEffect
{
	//! Get TriggerId (Left, Right)
	proto external GamepadTrigger GetTriggerId();
	//! Get TriggerEffectId (Rumble, Feedback)
	proto external GamepadTriggerEffect GetEffectId();
	//! Get trigger effect amplitude/strength curve
	proto external void GetCurve(out notnull array<int> outCurve);
	//! Get base amplitude/strength curve (without variable effects)
	proto external void GetBaseCurve(out notnull array<int> outCurve);
	//! Get Rumble frequency
	proto external int GetFrequency();
	//! Get base Rumble frequency (without variable effects)
	proto external int GetBaseFrequency();
	proto external bool IsFrequencyVariable();
	proto external bool IsAmplitudeVariable();
	//! Sets 'm_fFreqDeltaAmount'. Applicable only when m_bVariableFreq is set to true.
	//! \param amount must be in the range [0.0, 1.0].
	proto external void SetFrequencyDeltaAmount(float amount);
	//! Sets 'm_fAmplDeltaAmount'. Applicable only when m_bVariableAmplitude is set to true.
	//! \param amount must be in the range [0.0, 1.0].
	proto external void SetAmplitudeDeltaAmount(float amount);
	proto external float GetFrequencyDeltaAmount();
	proto external float GetAmplitudeDeltaAmount();
}

/*!
\}
*/
