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
	//! Set the gamepad device index for this effect only; Index must be between 0 and 3
	proto external void SetGamepadDeviceIndex(int idx);
	proto external int GetGamepadDeviceIndex();
	//! Set the default gamepad device index for all gamepad effects (unless overriden individually); Index must be between 0 and 3
	static proto void SetDefaultGamepadDeviceIndex(int idx);
	static proto int GetDefaultGamepadDeviceIndex();
	//! Set the gamepad device index for this effect only; Index must be between 0 and 3
	proto external void SetJoystickDeviceIndex(int idx);
	proto external int GetJoystickDeviceIndex();
	//! Set the default gamepad device index for all gamepad effects (unless overriden individually); Index must be between 0 and 3
	static proto void SetDefaultJoystickDeviceIndex(int idx);
	static proto int GetDefaultJoystickDeviceIndex();
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
