/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

class GamepadEffect: ScriptAndConfig
{
	proto external IEntity GetOwner();
	proto external bool IsEnabled();
	//! Disabled GamepadEffects are ignored.
	proto external void SetEnabled(bool val, float autoDisableTimeout = 0.0, bool immediate = true);
	//! Name of GamepadEffectContext on which this GamepadEffect depends on. Empty string means no context depended-upon.
	//! The context's IsEnabled() is ANDed with this GamepadEffect's IsEnabled().
	proto external owned string GetContext();
	proto external owned string GetEffectName();
	//! GamepadEffect with higher priority is preferred.
	proto external int GetPriority();
	//! Apply this effect immediately. Normally it would be applied at next update of GamepadIOHandlerComponent.
	proto external void ApplyImmediate();
}

/*!
\}
*/
