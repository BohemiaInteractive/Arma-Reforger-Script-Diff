/*!
\defgroup Input Input System
\addtogroup Input
\{
\see \ref Page_Input
\}
*/

/*!
\page Page_Input Input System

\tableofcontents

\section InputSystem_description Description
InputManager provides an abstraction layer between physical input devices and abstract actions. Also supplies an API for configuring action key bindings.

It offers interfaces for various devices:
- \ref KeyboardDeviceHandler
- \ref MouseDeviceHandler
- \ref GamepadDeviceHandler
- \ref JoystickDeviceHandler
- \ref GyroDeviceHandler
- \ref TrackIRDeviceHandler

\section InputSystem_Forcefeedback Force feedback
- For force feedback on joysticks use \ref JoystickDeviceHandler
- For rumble and trigger effects on gamepads use \ref GamepadDeviceHandler

Example:
\snippet this Input System FFB Example
*/

#ifdef DOXYGEN

//! [Input System FFB Example]
void OnUpdate()
{
	JoystickDeviceHandler handler = g_Game.GetInputManager().GetJoystickDeviceHandler();
		
	if (DbgUI.Button("FFB Force"))
	{
		handler.StartFFEffectConstantForce(0, 0, JoystickFFEffectType.CONSTANT_FORCE, 0.2, 1000);
	}
	
	if (DbgUI.Button("FFB Spring"))
	{
		handler.StartFFEffectCondition(0, 0, JoystickFFEffectType.SPRING, 1.0, 0.1);
	}
	
	if (DbgUI.Button("FFB Damper"))
	{
		handler.StartFFEffectCondition(0, 0, JoystickFFEffectType.DAMPER, 1.0, 0.1);
	}
	
	if (DbgUI.Button("FFB Inertia"))
	{
		handler.StartFFEffectCondition(0, 0, JoystickFFEffectType.INERTIA, 1.0, 0.1);
	}
	
	if (DbgUI.Button("FFB Friction"))
	{
		handler.StartFFEffectCondition(0, 0, JoystickFFEffectType.FRICTION, 1.0, 0.1);
	}
	
	if (DbgUI.Button("FFB Ramp"))
	{
		handler.StartFFEffectRamp(0, 0, JoystickFFEffectType.RAMP_FORCE, 1.0, 0.1);
	}
	
	if (DbgUI.Button("FFB Square"))
	{
		handler.StartFFEffectPeriodic(0, 0, JoystickFFEffectType.SQUARE, 0.3, 100);
	}
	
	if (DbgUI.Button("FFB Sine"))
	{
		handler.StartFFEffectPeriodic(0, 0, JoystickFFEffectType.SINE, 0.3, 100);
	}

	if (DbgUI.Button("FFB Combined"))
	{
		handler.StartFFEffectPeriodic(0, 0, JoystickFFEffectType.SQUARE, 0.2, 10);
		handler.StartFFEffectCondition(0, 1, JoystickFFEffectType.SPRING, 0.3, 0.3);
		handler.StartFFEffectCondition(0, 2, JoystickFFEffectType.SPRING, 0.5, 0.5, fDeadBand: 0.15);
	}
	
	if (DbgUI.Button("FFB Stop"))
	{
		handler.StopForceFeedbackEffect(0, -1);
	}
}
//! [Input System FFB Example]
#endif