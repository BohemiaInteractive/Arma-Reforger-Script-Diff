/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Input
\{
*/

sealed class GamepadDeviceHandler: InputDeviceHandler
{
	/*!
	\brief Start to rumble a gamepad for specific user (if available). Intensity for low/high frequency is in range from 0 to 1
	*/
	proto external void StartRumble(int userIdx, float fLeftMotorSpeed, float fRightMotorSpeed, float fLeftTriggerSpeed, float fRightTriggerSpeed, int iDurationMs = -1, int iFadeInMs = 0, int iFadeOutMs = 0);
	/*!
	*/
	//! use userIdx = -1 to stop rumble for all controllers
	proto external void StopRumble(int userIdx = -1);
	/*!
	\brief Set gamepad trigger effect
	Set gamepad trigger effect. Functionality can vary across platforms.

	\param userIdx user index
	\param eTriggerIndex can be left or right trigger
	\param eEffectType can be rumble or force-feedback
	\param frequency used just with rumble effect
	\param curve parameter curve, array of 10 integer values. Range is (0, 8) where 0 is minimum, 8 maximum force of effect.
	*/
	proto external void SetTriggerEffect(int userIdx, GamepadTrigger eTriggerIndex, GamepadTriggerEffect eEffectType, int frequency, notnull array<int> curve);
	/*!
	\brief Set gamepad light color
	*/
	proto external void SetLightColor(int userIdx, notnull Color color);
}

/*!
\}
*/
