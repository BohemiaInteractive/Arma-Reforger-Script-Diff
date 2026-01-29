/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

class GamepadIOHandlerComponentClass: ScriptComponentClass
{
}

class GamepadIOHandlerComponent: ScriptComponent
{
	proto external GamepadTriggerEffect GetActiveLeftTriggerEffect();
	proto external GamepadTriggerEffect GetActiveRightTriggerEffect();
	proto external GamepadLightEffect GetActiveLightEffect();
	proto external void Reset();
	proto external void RegisterEffectsManager(GamepadEffectsManagerComponent manager);
	proto external void UnregisterEffectsManager(GamepadEffectsManagerComponent manager);
	proto external void ApplyGamepadEffectImmediate(GamepadEffect effect);
	//!------------------------------------------------------------------------
	//! Gyro acceleration bias
	static proto void SetGyroAccelerationBias(vector min, vector max);
	static proto void GetGyroAccelerationBias(out vector min, out vector max);
	//!------------------------------------------------------------------------
	//! Gyro angular velocity bias
	static proto void SetGyroAngularVelocityBias(vector min, vector max);
	static proto void GetGyroAngularVelocityBias(out vector min, out vector max);
}

/*!
\}
*/
