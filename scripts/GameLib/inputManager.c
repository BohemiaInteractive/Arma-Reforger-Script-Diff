enum EActionValueType
{
	DIGITAL,
	ANALOG,
	ANALOG_RELATIVE,
	ANALOG_MOTION,
}

enum EInputDeviceType
{
	KEYBOARD,
	MOUSE,
	GAMEPAD,
	JOYSTICK,
	TRACK_IR,
	INVALID
}

typedef func ActionListenerCallback;
void ActionListenerCallback(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty); 

