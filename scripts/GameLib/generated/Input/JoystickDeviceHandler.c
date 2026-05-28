/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Input
\{
*/

sealed class JoystickDeviceHandler: InputDeviceHandler
{
	/*
	\brief Checks if a joystick is connected in the specified slot.
	\param iSlotIndex Index of the joystick slot to check.
	\return true if a joystick is connected, false otherwise.
	*/
	proto external bool IsJoystickConnected(int iSlotIndex);
	/*!
	\brief Retrieves the USB vendor ID of the joystick on the given slot.
	\param iSlotIndex Index of the joystick slot.
	\return USB Vendor ID.
	*/
	proto external int GetJoystickVendorId(int iSlotIndex);
	/*!
	\brief Retrieves the USB product ID of the joystick on the given slot.
	\param iSlotIndex Index of the joystick slot.
	\return USB Product ID.
	*/
	proto external int GetJoystickProductId(int iSlotIndex);
	/*!
	\brief Returns the human-readable name of the joystick.
	\param iSlotIndex Index of the joystick slot.
	\return Product name as a string.
	*/
	proto external string GetJoystickProductName(int iSlotIndex);
	/*!
	\brief Starts a constant force feedback effect on the joystick.
	\param iSlotIndex Index of the joystick slot.
	\param iEffectIndex Effect slot. Valid values are \<0, 3\>.
	\param eEffectType Type of force feedback effect.
										\note Only \ref CONSTANT_FORCE is supported.
	\param fMagnitude Force magnitude.
	\param iDurationMs Duration in milliseconds. -1 for infinite duration.
	\return true if successful, false otherwise.
	*/
	proto external bool StartFFEffectConstantForce(int iSlotIndex, int iEffectIndex, JoystickFFEffectType eEffectType, float fMagnitude, int iDurationMs = -1);
	/*!
	\brief Starts a ramp force feedback effect on the joystick.
	\param iSlotIndex Index of the joystick slot.
	\param iEffectIndex Effect slot. Valid values are \<0, 3\>.
	\param eEffectType Type of force feedback effect.
									\note Only \ref RAMP_FORCE is supported.
	\param fStart Start magnitude.
	\param fEnd End magnitude.
	\param iDurationMs Duration in milliseconds. -1 for infinite duration.
	\return true if successful, false otherwise.
	*/
	proto external bool StartFFEffectRamp(int iSlotIndex, int iEffectIndex, JoystickFFEffectType eEffectType, float fStart, float fEnd, int iDurationMs = -1);
	/*!
	\brief Starts a periodic force feedback effect on the joystick.
	\param iSlotIndex Index of the joystick slot.
	\param iEffectIndex Effect slot. Valid values are \<0, 3\>.
	\param eEffectType Type of force feedback periodic effect.
									 \note Supported values: \ref SQUARE, \ref SINE, \ref TRIANGLE, \ref SAWTOOTH_UP, \ref SAWTOOTH_DOWN.
	\param fMagnitude Magnitude of the force.
	\param fFrequencyHz Frequency in Hz (default 50Hz).
	\param fOffset Offset value.
	\param fPhase Phase shift value.
	\param iDurationMs Duration in milliseconds. -1 for infinite duration.
	\return true if successful, false otherwise.
	*/
	proto external bool StartFFEffectPeriodic(int iSlotIndex, int iEffectIndex, JoystickFFEffectType eEffectType, float fMagnitude, float fFrequencyHz = 50, float fOffset = 0, float fPhase = 0, int iDurationMs = -1);
	/*!
	\brief Starts a condition-based force feedback effect on the joystick.
	\param iSlotIndex Index of the joystick slot.
	\param iEffectIndex Effect slot. Valid values are \<0, 3\>.
	\param eEffectType Type of force feedback condition effect.
									 \note Supported values: \ref SPRING, \ref DAMPER, \ref INERTIA, \ref FRICTION.
	\param fPositiveCoefficient Coefficient for positive force direction.
	\param fNegativeCoefficient Coefficient for negative force direction.
	\param fOffset Center offset.
	\param fDeadBand Deadband amount.
	\param iDurationMs Duration in milliseconds. -1 for infinite duration.
	\return true if successful, false otherwise.
	*/
	proto external bool StartFFEffectCondition(int iSlotIndex, int iEffectIndex, JoystickFFEffectType eEffectType, float fPositiveCoefficient, float fNegativeCoefficient, float fOffset = 0, float fDeadBand = 0, int iDurationMs = -1);
	/*!
	\brief Stops a force feedback effect.
	\param iSlotIndex Index of the joystick slot.
	\param iEffectIndex Effect slot. Valid values are \<0, 3\>. Use -1 to stop all effects.
	*/
	proto external void StopForceFeedbackEffect(int iSlotIndex, int iEffectIndex = -1);
}

/*!
\}
*/
