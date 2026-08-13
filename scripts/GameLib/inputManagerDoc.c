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

\section InputSystem_Description Description
InputManager provides an abstraction layer between physical input devices and abstract actions. In gameplay code only actions are used, never direct keys or mouse. The system is split into two layers: \ref ActionManager which is a universal mechanism for managing float-state actions, and \ref InputManager which specializes it for physical input devices.

\ref ActionManager is not bound only to input controllers. Action values can be updated by various ActionUpdaters (\ref InputManager is one of them). Action managers are organized hierarchically, meaning context or action definitions are searched in the parent action manager if they are not found in the current one. Contexts from a parent action manager can also block contexts in a child action manager with lower priority.

\dot
digraph InputSystemOverview {
	rankdir=LR;
	node [shape=box, style=filled, fillcolor="#E8E8E8", fontname="sans-serif", fontsize=10];
	edge [fontname="sans-serif", fontsize=9];

	ActionManager [fillcolor="#B0C4DE"];
	InputManager [fillcolor="#B0C4DE"];
	InputDevice [label="InputDevice"];
	InputFilter [label="InputFilter"];
	InputSource [label="InputSource"];
	Action [label="Action"];
	Game [label="Game"];

	InputManager -> ActionManager [arrowhead=empty, label="inherits"];
	InputDevice -> InputFilter [label="raw value"];
	InputFilter -> InputSource [label="filtered value"];
	InputSource -> Action [label="*  feeds  *"];
	InputManager -> Action [label="has *"];
	Game -> Action [label="reads value"];
}
\enddot

Key classes:
- \ref ActionManager - container holding ActionContexts and ActionInstances. ActionManagers are hierarchically organized. To be updated, an action manager must be registered in an ActionUpdater.
- **\ref InputSystem_ActionContexts "Context"** - groups actions which logically belong together. Has priority and flags.
- **Action** - runtime instance of an action. Contains a value (float), list of listeners and link to action definition.
- \ref InputManager - specialization of \ref ActionManager. Reads from input controllers (mouse, keyboard, gamepad, joystick) and updates action values.
- \ref InputBinding - handles user key binding configuration, capture and conflict detection.

\section InputSystem_Configuration Configuration
The input system is configured through action configuration files referenced in the project settings under **InputManagerSettings**. The configuration defines actions, their input sources and action contexts.

Each action has the following properties:
- **Type** - value type of the action: Digital, Analog, AnalogRelative or Motion.
- **InputSource** - tree of input sources that feed the action. Sources can be combined using InputSourceSum (alternatives) or InputSourceCombo (simultaneous keys).

Each input source can have a **Filter** that transforms the raw input value. See \ref InputSystem_InputFilter "InputFilter" for the list of available filters.

Example configuration:
\verbatim
ActionManager {
 Actions {
  Action MyAction {
   InputSource InputSourceSum "{00000000}" {
    Sources {
     InputSourceValue "{00000001}" {
      Input "keyboard:KC_G"
      Filter InputFilterDown "{00000002}" {
      }
     }
     InputSourceValue "{00000003}" {
      Input "gamepad0:a"
      Filter InputFilterDown "{00000004}" {
      }
     }
    }
   }
  }
 }
 Contexts {
  ActionContext MyContext {
   Priority 5
   ActionRefs {
    "MyAction"
   }
  }
 }
}
\endverbatim

\section InputSystem_InputSource InputSource
Each action has an InputSource. There are several source classes with different behavior:
- **InputSourceValue** - simply provides the value from a single physical input.
- **InputSourceSum** - sums values from any number of child input sources. Used when multiple keys should trigger the same action (alternatives).
- **InputSourceCombo** - provides combo functionality. All child sources must be activated for the combo to activate. When the first input from a combo is activated, all other inputs in the combo are blocked for other InputSources. This allows distinguishing between for example 'Ctrl + A' and 'A' alone.

InputSourceSum represents "any of these inputs" (OR logic), InputSourceCombo represents "all of these inputs at once" (AND logic):

\dot
digraph InputSourceSum {
	rankdir=LR;
	label="InputSourceSum: W or Up or Gamepad";
	labelloc=t;
	node [shape=box, style=filled, fillcolor="#E8E8E8", fontname="sans-serif", fontsize=10];
	edge [fontname="sans-serif", fontsize=9];

	W [label="keyboard:KC_W"];
	Up [label="keyboard:KC_UP"];
	Gp [label="gamepad0:left_thumb_vertical"];
	Sum [label="InputSourceSum", fillcolor="#B0C4DE"];
	Action [label="CharacterForward"];

	W -> Sum;
	Up -> Sum;
	Gp -> Sum;
	Sum -> Action;
}
\enddot

\dot
digraph InputSourceCombo {
	rankdir=LR;
	label="InputSourceCombo: Ctrl + A";
	labelloc=t;
	node [shape=box, style=filled, fillcolor="#E8E8E8", fontname="sans-serif", fontsize=10];
	edge [fontname="sans-serif", fontsize=9];

	Ctrl [label="keyboard:KC_LCONTROL"];
	A [label="keyboard:KC_A"];
	Combo [label="InputSourceCombo", fillcolor="#B0C4DE"];
	Action [label="SelectAll"];

	Ctrl -> Combo;
	A -> Combo;
	Combo -> Action;
}
\enddot

\section InputSystem_InputFilter InputFilter
Filter classes transform raw input values into output values. For example, a filter can detect just the rising edge of a signal (key down), transform input by a curve, or react only when input is active for some time (key hold). Available filters:
- **InputFilterValue** - passes through the normalized input value to the output.
- **InputFilterPressed** - returns 1.0 when input value is above threshold, otherwise returns 0.0.
- **InputFilterDown** - returns 1.0 only in the frame when input value hits the rising edge (key down).
- **InputFilterUp** - returns 1.0 only in the frame when input value hits the falling edge (key up).
- **InputFilterClick** - returns 1.0 only in the frame when input value hits the falling edge (key up) AND duration between the rising and falling edge is less than HOLD_DURATION.
- **InputFilterSingleClick** - same as InputFilterClick but will not fire if a double click is detected within DOUBLE_CLICK_DURATION.
- **InputFilterDoubleClick** - returns 1.0 only in the frame when input value hits the rising edge (key down) a second time within DOUBLE_CLICK_DURATION.
- **InputFilterHold** - returns 1.0 when input value is above threshold for a minimal amount of time HOLD_DURATION. Value rises gradually from 0.0 to 1.0 during HOLD_DURATION.
- **InputFilterHoldOnce** - returns 1.0 once when input value is above threshold for a minimal amount of time HOLD_DURATION. Value rises gradually from 0.0 to 1.0 during HOLD_DURATION.
- **InputFilterToggle** - has value 0.0 or 1.0, switched by the rising edge (key down) of input value.
- **InputFilterRepeat** - similar to InputFilterDown, returns 1.0 on the rising edge (key down), then after INITIAL_INTERVAL repeats every REPEAT_INTERVAL for one frame, until the value drops below threshold.

\section InputSystem_ActionContexts Action Contexts
All actions are organized in **contexts**. A context groups actions which logically belong together. One action can be in multiple contexts. Because many actions share the same inputs but not all actions are used at the same time, the game must activate contexts which are used each frame (or activate them once for some amount of time).

Each context has a **priority** and some **flags**. During update, \ref InputManager iterates through contexts. If a context was activated by the game in the current frame and there is no active context with higher priority, it processes its actions. Otherwise values of actions are zeroed or unchanged (depending on action type).

Context flags:
- **Overlay** - allows contexts with lower priority to also be updated when this context is active.
- **CursorVisible** - tells \ref InputManager that the context uses the mouse cursor.

Contexts are managed through \ref ActionManager :
- \ref ActionManager.ActivateContext - activates a context by name, with an optional duration in milliseconds.
- \ref ActionManager.IsContextActive - checks whether a context is currently active.

\remark Menus automatically activate their associated action context when opened and deactivate it when closed. See \ref Page_Menu for details.

\subsection InputSystem_Context_UseCase Context use case
For example, consider these four contexts:

| Name | Description | Priority | Flags |
|---|---|---|---|
| CharacterLookContext | mouse look actions | 1 | |
| CharacterMovementContext | W,A,S,D movement actions | 2 | Overlay |
| InventoryContext | mouse UI interaction actions | 2 | CursorVisible |
| InGameMenuContext | mouse UI interaction actions | 3 | CursorVisible |

- **Regular gameplay:** CharacterLookContext and CharacterMovementContext are activated by gameplay code. CharacterMovementContext has priority 2, but it has the Overlay flag, so it also allows CharacterLookContext with lower priority to be updated. Result: character can both walk and look around, cursor is not visible.
- **Inventory open:** InventoryContext starts to be activated too. It has priority 2 but no Overlay flag, so only contexts with priority 2 or higher are updated (InventoryContext and CharacterMovementContext). Result: character can still walk, but mouse movement is used for interaction with UI in inventory, cursor is now visible because of CursorVisible flag on InventoryContext.
- **In-game menu open:** InGameMenuContext is activated by the game. It has priority 3, so the first three contexts are inactive. Result: character does not react to any input, the player can only interact with the in-game menu, cursor is visible because of CursorVisible flag on InGameMenuContext.

\section InputSystem_Actions Actions and Listeners
In gameplay code only **actions** are used, never direct keys or mouse. Each action has a name and a type. The action type tells \ref InputManager how to use the value and how to deal with different types of inputs.

There are four action value types defined by EActionValueType:
- **DIGITAL** - binary on/off input (keyboard keys, mouse buttons, gamepad buttons).
- **ANALOG** - continuous value (thumbstick axes, trigger axes on gamepads).
- **ANALOG_RELATIVE** - relative motion value (mouse wheel, relative mouse position).
- **ANALOG_MOTION** - motion sensor input (motion controllers for VR, absolute mouse position).

Various combinations of action type vs input type behave differently:

| | Input: Digital | Input: Analog | Input: AnalogRelative | Input: Motion |
|---|---|---|---|---|
| **Action: Digital** | value | value | **sum** | **diff** |
| **Action: Analog** | value | value | **sum** | **diff** |
| **Action: AnalogRelative** | value | value | value | **diff** |
| **Action: Motion** | **sum** | **sum** | **sum** | value |

To query an action value directly, use \ref ActionManager.GetActionValue or \ref ActionManager.GetActionTriggered. To react to input events, register a listener using \ref ActionManager.AddActionListener with one of the EActionTrigger modes:
- **DOWN** - fires once when the input is pressed.
- **UP** - fires once when the input is released.
- **PRESSED** - fires every frame while the input is held.
- **VALUE** - fires every frame with the current value.

\snippet this Input System Listener Example

\section InputSystem_InputBinding Input Binding
\ref InputBinding provides runtime access to user key binding configuration. Create an instance through \ref InputManager.CreateUserBinding.

Key operations:
- **Querying bindings** - use GetBindings to retrieve current key bindings for an action and device type.
- **Modifying bindings** - use AddBinding, RemoveBinding and InsertCombo to change key assignments.
- **Capture mode** - use StartCapture to listen for user input and automatically assign it as a new binding. Use SaveCapture to confirm or CancelCapture to abort.
- **Conflict detection** - use GetConflicts to find actions that share the same key bindings.
- **Persistence** - use Save to write binding changes to persistent storage.

\remark Use ResetDefault to restore the original binding for a specific action.

\snippet this Input System Binding Example

\section InputSystem_DeviceHandlers Device Handlers
InputManager provides access to device-specific handlers through dedicated getter methods. Each handler inherits from \ref InputDeviceHandler.

| Handler | Getter | Purpose |
|---------|--------|---------|
| \ref KeyboardDeviceHandler | GetKeyboardDeviceHandler | Keyboard input. |
| \ref MouseDeviceHandler | GetMouseDeviceHandler | Mouse input and cursor position. |
| \ref GamepadDeviceHandler | GetGamepadDeviceHandler | Gamepad input, rumble and trigger effects. |
| \ref JoystickDeviceHandler | GetJoystickDeviceHandler | Joystick/HOTAS input and force feedback. |
| \ref GyroDeviceHandler | GetGyroDeviceHandler | Gyroscope and motion sensor input. |
| \ref TrackIRDeviceHandler | GetTrackIRDeviceHandler | TrackIR head tracking. |

The currently dominant input device can be queried with \ref InputManager.GetLastUsedInputDevice, which returns an EInputDeviceType value. Use \ref InputManager.IsUsingMouseAndKeyboard to check whether the player is using mouse and keyboard as opposed to a gamepad or other device.

\section InputSystem_Forcefeedback Force Feedback
Force feedback is available on joysticks and gamepads through their respective device handlers.

For joysticks, \ref JoystickDeviceHandler provides four types of force feedback effects:
- **Constant force** - sustained force in one direction.
- **Ramp** - force that transitions from a start to an end magnitude.
- **Periodic** - repeating waveforms (square, sine, triangle, sawtooth).
- **Condition** - reactive effects (spring, damper, inertia, friction).

Each joystick supports up to 4 simultaneous effect slots (indices 0 to 3).

For gamepads, \ref GamepadDeviceHandler provides rumble through StartRumble and StopRumble, as well as trigger effects through SetTriggerEffect. The gamepad light color can be set with SetLightColor.

\snippet this Input System FFB Example
*/

#ifdef DOXYGEN

//! [Input System Listener Example]
class MyInputComponent
{
	// Context must be re-activated each frame to stay active
	void OnUpdate()
	{
		InputManager inputManager = g_Game.GetInputManager();
		inputManager.ActivateContext("MyContext");

		UpdateActions(inputManager);
	}

	// --- Listener - callback approach ---

	void Init()
	{
		InputManager inputManager = g_Game.GetInputManager();

		inputManager.AddActionListener("MyAction", EActionTrigger.DOWN, OnActionDown);
		inputManager.AddActionListener("MyAction", EActionTrigger.UP, OnActionUp);
	}

	void ~MyInputComponent()
	{
		InputManager inputManager = g_Game.GetInputManager();

		inputManager.RemoveActionListener("MyAction", EActionTrigger.DOWN, OnActionDown);
		inputManager.RemoveActionListener("MyAction", EActionTrigger.UP, OnActionUp);
	}

	void OnActionDown(float value, EActionTrigger reason, string actionName)
	{
		Print("Action pressed: " + actionName);
	}

	void OnActionUp(float value, EActionTrigger reason, string actionName)
	{
		Print("Action released: " + actionName);
	}

	// --- Polling approach ---

	void UpdateActions(InputManager inputManager)
	{
		bool isTriggered = inputManager.GetActionTriggered("MyAction");
		float fValue = inputManager.GetActionValue("MyAction");

		if (isTriggered)
		{
			Print("MyAction is triggered");
		}

		// Display current action value in debug UI
		DbgUI.Text("MyAction value: " + fValue);
	}
}
//! [Input System Listener Example]

//! [Input System Binding Example]
void ShowAndModifyBindings()
{
	InputBinding binding = g_Game.GetInputManager().CreateUserBinding();

	array<string> bindings = {};
	binding.GetBindings("MyAction", bindings, EInputDeviceType.KEYBOARD);
	foreach (string key : bindings)
	{
		Print("Current binding: " + key);
	}

	binding.AddBinding("MyAction", "", "keyboard:KC_F");
	binding.Save();
}

void CaptureNewBinding()
{
	InputBinding binding = g_Game.GetInputManager().CreateUserBinding();

	binding.StartCapture("MyAction", EInputDeviceType.KEYBOARD);
}
//! [Input System Binding Example]

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
