//#define DEBUG_MODULAR_BUTTON


//! These enum values correspond to change of button state,
//! A button can only be in one state at a time
enum EModularButtonState
{
	STATE_DEFAULT 				= 1 << 0,	//!< Default state
	STATE_HOVERED				= 1 << 1,	//!< Hovered
	STATE_ACTIVATED				= 1 << 2,	//!< Activated
	STATE_ACTIVATED_HOVERED		= 1 << 3,	//!< Activated and hovered
	STATE_DISABLED				= 1 << 4,	//!< Disabled
	STATE_DISABLED_ACTIVATED	= 1 << 5	//!< Disabled and activated
}

//! Enum corresponding Enfusion native event types
enum EModularButtonEventHandler
{
	CLICK			= 1 << 0,
	DOUBLE_CLICK	= 1 << 1,
	FOCUS_GAINED	= 1 << 2,
	FOCUS_LOST		= 1 << 3,
	MOUSE_ENTER		= 1 << 4,
	MOUSE_LEAVE		= 1 << 5,
}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Completely separate hierarchy from other buttons and ui components

//! Modular button implements all logic of a button.
//! It can be customised by various SCR_ButtonEffectBase objects attached to it.
class SCR_ModularButtonComponent : ScriptedWidgetComponent
{
	
//---- REFACTOR NOTE END ----
	
	// ---- Public and attributes ----
	
	// Attributes - effects

	[Attribute()]
	protected ref array<ref SCR_ButtonEffectBase> m_aEffects;
	
	// Attributes - other properties
	
	[Attribute("false", UIWidgets.CheckBox, "Can the button be only clicked, or also toggled?")]
	protected bool m_bCanBeToggled;
	
	[Attribute("false", UIWidgets.CheckBox, "If the button can be toggled, toggle it automatically on click or only through external API?")]
	protected bool m_bToggledOnlyThroughApi;
	
	[Attribute("false")]
	protected bool m_bToggledAtStart;
	
	[Attribute("2147483647", UIWidgets.Flags, "Value which will be returned from event handlers. Useful in button-over-button cases: tiles with buttons inside or similar.", "", ParamEnumArray.FromEnum(EModularButtonEventHandler))]
	protected EModularButtonEventHandler m_eEventReturnValue;
	
	[Attribute("false", UIWidgets.CheckBox, "The button will be focused on mouse enter events")]
	protected bool m_bFocusOnMouseEnter;
	
	[Attribute("false", UIWidgets.CheckBox, "Mouse over, focus, clicks, etc, will be completely ignored. Useful to create a button driven by SlaveButton effect.")]
	protected bool m_bIgnoreStandardInputs;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Old untyped invokers
		
	// Script invokers
	
	ref ScriptInvoker m_OnClicked = new ScriptInvoker();		// (SCR_ModularButtonComponent comp)
	ref ScriptInvoker m_OnDoubleClicked = new ScriptInvoker();	// (SCR_ModularButtonComponent comp)
	ref ScriptInvoker m_OnToggled = new ScriptInvoker();		// (SCR_ModularButtonComponent comp, bool newToggled)
	ref ScriptInvoker m_OnFocus = new ScriptInvoker();			// (SCR_ModularButtonComponent comp)
	ref ScriptInvoker m_OnFocusLost = new ScriptInvoker();		// (SCR_ModularButtonComponent comp)
	ref ScriptInvoker m_OnMouseEnter = new ScriptInvoker();		// (SCR_ModularButtonComponent comp, bool mouseInput) - mouseEvent - when true, last input was from a mouse, not keyboard/gamepad.
	ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();		// (SCR_ModularButtonComponent comp, bool mouseInput)
	
//---- REFACTOR NOTE END ----
	
	// ---- Protected ----
	
	// Widgets
	protected Widget m_wRoot;
		
	// Internal state
	protected bool m_bFocus;
	protected bool m_bMouseOver;
	protected bool m_bToggled;
	protected EModularButtonState m_eState;
	
	// Other
	protected ref Managed m_UserData; // User data - can be accessed with SetData, GetData
	
	// ------------------------------- Public -----------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Finds SCR_ModularButtonComponent on a widget
	//! \param[in] w
	//! \return
	static SCR_ModularButtonComponent FindComponent(Widget w)
	{
		return SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetEnabled(bool enabled)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("SetEnabled: %1", enabled));
		#endif
		
		m_wRoot.SetEnabled(enabled);
		UpdateCurrentState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetEnabled()
	{
		return m_wRoot.IsEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] toggled
	//! \param[in] invokeOnToggled
	void SetToggled(bool toggled, bool invokeOnToggled = true, bool instant = false)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("SetToggled: %1", toggled));
		#endif
		
		if (!m_bCanBeToggled)
			return;
		
		Internal_SetToggled(toggled, invokeOnToggled, instant);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetToggled()
	{
		return m_bToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetFocused()
	{
		return m_bFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetMouseOver()
	{
		return m_bMouseOver;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] toggleable
	void SetToggleable(bool toggleable)
	{
		m_bCanBeToggled = toggleable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] newValue
	void SetTogglableOnlyThroughApi(bool newValue)
	{
		m_bToggledOnlyThroughApi = newValue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	void SetData(Managed data)
	{
		m_UserData = data;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Managed GetData()
	{
		return m_UserData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] visible
	//! \return
	bool SetVisible(bool visible)
	{
		if (!m_wRoot)
			return false;
		
		m_wRoot.SetVisible(visible);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns first effect with given tag
	//! \param[in] tag
	//! \return
	SCR_ButtonEffectBase FindEffect(string tag)
	{
		foreach (SCR_ButtonEffectBase e : m_aEffects)
		{
			if (e.m_aTags.Contains(tag))
				return e;
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] tag
	//! \return all effects with given tag
	array<SCR_ButtonEffectBase> FindAllEffects(string tag)
	{
		array<SCR_ButtonEffectBase> effects = {};
		foreach (SCR_ButtonEffectBase e : m_aEffects)
		{
			if (e.m_aTags.Contains(tag))
				effects.Insert(e);
		}

		return effects;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return all effects
	array<SCR_ButtonEffectBase> GetAllEffects()
	{
		array<SCR_ButtonEffectBase> effects = {};
		foreach (SCR_ButtonEffectBase e : m_aEffects)
		{
			effects.Insert(e);
		}

		return effects;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables or disables all effects with given tag
	//! \param[in] tag
	//! \param[in] enable
	void SetEffectsEnabled(string tag, bool enable)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("SetEffectsEnabled: %1, %2", tag, enable));
		#endif
		
		foreach (SCR_ButtonEffectBase e : m_aEffects)
		{
			if (e.m_aTags.Contains(tag))
				e.SetEnabled(enable);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Effects with at least one tag are enabled. Other effects are disabled.
	//! \param[in] tags
	void SetEffectsWithAnyTagEnabled(notnull array<string> tags)
	{		
		foreach (SCR_ButtonEffectBase e : m_aEffects)
		{
			bool found = false;
			foreach (string tag : tags)
			{
				if (e.m_aTags.Contains(tag))
				{
					found = true;
					break;
				}
			}

			e.SetEnabled(found);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables ar disables all effects
	//! \param[in] enable
	void SetAllEffectsEnabled(bool enable)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("SetAllEffectsEnabled: %1", enable));
		#endif
		
		foreach (SCR_ButtonEffectBase e : m_aEffects)
		{
			e.SetEnabled(enable);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Applies all enabled effects. 
	//! Param instant should be true for changes to an effect's setting while it is active (eg: change the focused color while element is focused)
	//! If instant == false, it's like retriggering the event, eg element simulates being focused again
	//! \param[in] instant
	void InvokeAllEnabledEffects(bool instant)
	{
		EModularButtonState state = GetCurrentState();
		InvokeEffectsEvent(state, instant);
		
		if (m_bCanBeToggled)
		{
			if (m_bToggled)
				InvokeEffectsEvent(EModularButtonEventFlag.EVENT_TOGGLED_ON, instant);
			else
				InvokeEffectsEvent(EModularButtonEventFlag.EVENT_TOGGLED_OFF, instant);				
		}
		
		if (m_bFocus)
			InvokeEffectsEvent(EModularButtonEventFlag.EVENT_FOCUS_GAINED, instant);
		else
			InvokeEffectsEvent(EModularButtonEventFlag.EVENT_FOCUS_LOST, instant);
	}
	
	// ------------------------ Protected -------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Checks current state, invokes state change effects if state has changed.
	protected void UpdateCurrentState()
	{
		EModularButtonState newState = GetCurrentState();
		EModularButtonState oldState = m_eState;
		m_eState = newState;
		
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("UpdateCurrentState: old: %1, new: %2", typename.EnumToString(EModularButtonState, oldState), typename.EnumToString(EModularButtonState, newState)));
		#endif
		
		if (newState != oldState)
			InvokeEffectsEvent(newState);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Enum with current state value
	protected EModularButtonState GetCurrentState()
	{	
		if (m_wRoot.IsEnabled())
		{
			// Enabled
			
			if (m_bCanBeToggled)
			{
				if (m_bToggled)
				{
					if (m_bMouseOver)
						return EModularButtonState.STATE_ACTIVATED_HOVERED;
					else
						return EModularButtonState.STATE_ACTIVATED;
				}
				else
				{
					if (m_bMouseOver)
						return EModularButtonState.STATE_HOVERED;
					else
						return EModularButtonState.STATE_DEFAULT;
				}
			}
			else
			{
				// Non toggleable
				if (m_bMouseOver)
					return EModularButtonState.STATE_HOVERED;
				else
					return EModularButtonState.STATE_DEFAULT;
			}
		}
		else
		{
			// Disabled
			
			if (m_bCanBeToggled)
			{
				if (m_bToggled)
					return EModularButtonState.STATE_DISABLED_ACTIVATED;
				else
					return EModularButtonState.STATE_DISABLED;
			}
			else
				return EModularButtonState.STATE_DISABLED;
		}
		
		return EModularButtonState.STATE_DEFAULT;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls _OnEvent of all enabled effects.
	//! \param[in] eventFlag
	//! \param[in] instant
	protected void InvokeEffectsEvent(EModularButtonEventFlag eventFlag, bool instant = false)
	{
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("InvokeEffectsEvent: %1, %2", typename.EnumToString(EModularButtonEventFlag, eventFlag), typename.EnumToString(EInputDeviceType, deviceType)));
		#endif
		
		foreach (SCR_ButtonEffectBase effect : m_aEffects)
		{
			if (effect.GetEnabled())
				effect.Internal_OnEvent(eventFlag, deviceType, instant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used for passing events from master button / slave button
	//! This is called on a slave button by SCR_ButtonEffectSlaveButton
	//! \param[in] eventFlag
	//! \param[in] instant
	void Internal_OnMasterButtonEvent(EModularButtonEventFlag eventFlag, bool instant)
	{
		InvokeEffectsEvent(eventFlag, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Internal_SetToggled(bool newToggled, bool invokeOnToggled = true, bool instant = false)
	{
		bool oldToggled = m_bToggled;
		m_bToggled = newToggled;
		
		if (newToggled != oldToggled)
		{
			if (newToggled)
				InvokeEffectsEvent(EModularButtonEventFlag.EVENT_TOGGLED_ON, instant);
			else
				InvokeEffectsEvent(EModularButtonEventFlag.EVENT_TOGGLED_OFF, instant);
		}
		
		UpdateCurrentState();
		
		if (invokeOnToggled && newToggled != oldToggled)
			m_OnToggled.Invoke(this, newToggled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Must only be called by the effect class
	//! \param[in] effect
	void Internal_OnEffectEnabled(SCR_ButtonEffectBase effect)
	{
		EModularButtonState state = GetCurrentState();
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		effect.Internal_OnEvent(state, deviceType, true);
		
		if (m_bCanBeToggled)
		{
			if (m_bToggled)
				effect.Internal_OnEvent(EModularButtonEventFlag.EVENT_TOGGLED_ON, deviceType, true);
			else
				effect.Internal_OnEvent(EModularButtonEventFlag.EVENT_TOGGLED_OFF, deviceType, true);				
		}
		
		if (m_bFocus)
			effect.Internal_OnEvent(EModularButtonEventFlag.EVENT_FOCUS_GAINED, deviceType, true);
		else
			effect.Internal_OnEvent(EModularButtonEventFlag.EVENT_FOCUS_LOST, deviceType, true);
	}
	
	// --------- Event Handlers ------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w; 
		
		// Initialize effects
		foreach (SCR_ButtonEffectBase effect : m_aEffects)
		{
			effect.Init(this);
			effect.OnHandlerAttached(w);
		}
			
		m_bToggled = m_bToggledAtStart;
		
		m_bFocus = GetGame().GetWorkspace().GetFocusedWidget() == m_wRoot;
		
		// Iterate all effects, make sure that there are no effects which are affecting same widget,
		// have same type and receive same event types
		array<ref Tuple3<Widget, typename, int>> widgetEffectEventMasks = {};
		foreach (SCR_ButtonEffectBase effect : m_aEffects)
		{
			SCR_ButtonEffectWidgetBase widgetEffect = SCR_ButtonEffectWidgetBase.Cast(effect);
			if (effect.GetEnabled() && widgetEffect)
			{
				// Find entry with this widget, effect type, and event mask which intersects with event mask of this effect
				bool disabled = false;
				foreach (Tuple3<Widget, typename, int> i : widgetEffectEventMasks)
				{
					if (i.param1 == widgetEffect.GetTargetWidget() &&
						i.param2 == widgetEffect.Type() &&
						(i.param3 & widgetEffect.m_eEvents))
					{
						disabled = true;
						widgetEffect.SetEnabled(false);
						break;
					}
				}
				
				// If this effect is enabled, record what events it listens to.
				if (!disabled)
				{
					// Find entry with this widget and effect type
					Tuple3<Widget, typename, int> entry;
					foreach (Tuple3<Widget, typename, int> i : widgetEffectEventMasks)
					{
						if (i.param1 == widgetEffect.GetTargetWidget() &&
							i.param2 == widgetEffect.Type())
						{
							entry = i;
							break;
						}
					}
					
					if (!entry)
					{
						entry = new Tuple3<Widget, typename, int>(widgetEffect.GetTargetWidget(), widgetEffect.Type(), 0);
						widgetEffectEventMasks.Insert(entry);
					}
						
					entry.param3 = entry.param3 | widgetEffect.m_eEvents;
				}
			}
		}
		
		// Invoke effects at start
		InvokeAllEnabledEffects(instant : true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print("OnClick");
		#endif
		
		// Auto focus is focusable
		if (m_wRoot.IsFocusable())
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			Widget currentFocus = workspace.GetFocusedWidget();
			if (currentFocus != m_wRoot)
				workspace.SetFocusedWidget(m_wRoot);
		}
		
		bool eventReturnValue = m_eEventReturnValue & EModularButtonEventHandler.CLICK;
		
		if (m_bIgnoreStandardInputs)
			return eventReturnValue;
		
		if (m_bCanBeToggled && !m_bToggledOnlyThroughApi)
			Internal_SetToggled(!m_bToggled);
		
		InvokeEffectsEvent(EModularButtonEventFlag.EVENT_CLICKED);
		
		EModularButtonState state = GetCurrentState();
		InvokeEffectsEvent(state);
		
		m_OnClicked.Invoke(this);
			
		return eventReturnValue;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (m_bIgnoreStandardInputs)
			return m_eEventReturnValue & EModularButtonEventHandler.DOUBLE_CLICK;
		
		m_OnDoubleClicked.Invoke(this);
		return m_eEventReturnValue & EModularButtonEventHandler.DOUBLE_CLICK;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print("OnMouseEnter");
		#endif
		
		if (m_bIgnoreStandardInputs)
			return m_eEventReturnValue & EModularButtonEventHandler.MOUSE_ENTER;
		
		m_bMouseOver = true;
		
		UpdateCurrentState();
		
		EInputDeviceType lastInput = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool mouseInput = lastInput == EInputDeviceType.MOUSE;
		
		if (m_bFocusOnMouseEnter)
		{
			if (mouseInput)
				GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
		}
		
		InvokeEffectsEvent(EModularButtonEventFlag.EVENT_MOUSE_ENTER);
		
		m_OnMouseEnter.Invoke(this, mouseInput);
			
		return m_eEventReturnValue & EModularButtonEventHandler.MOUSE_ENTER;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print("OnMouseLeave");
		#endif
		
		if (m_bIgnoreStandardInputs)
			return m_eEventReturnValue & EModularButtonEventHandler.MOUSE_LEAVE;
		
		m_bMouseOver = false;
		
		UpdateCurrentState();
		
		EInputDeviceType lastInput = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool mouseInput = lastInput == EInputDeviceType.MOUSE;
		
		InvokeEffectsEvent(EModularButtonEventFlag.EVENT_MOUSE_LEAVE);
		
		m_OnMouseLeave.Invoke(this, mouseInput);
		
		return m_eEventReturnValue & EModularButtonEventHandler.MOUSE_LEAVE;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print("%1 OnFocus");
		#endif
		
		if (m_bIgnoreStandardInputs)
			return m_eEventReturnValue & EModularButtonEventHandler.FOCUS_GAINED;
		
		m_bFocus = true;
		
		m_OnFocus.Invoke(this);
		
		InvokeEffectsEvent(EModularButtonEventFlag.EVENT_FOCUS_GAINED);
		
		return m_eEventReturnValue & EModularButtonEventHandler.FOCUS_GAINED;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print("%1 OnFocusLost");
		#endif
		
		if (m_bIgnoreStandardInputs)
			return m_eEventReturnValue & EModularButtonEventHandler.FOCUS_LOST;
		
		m_bFocus = false;
		
		m_OnFocusLost.Invoke(this);
		
		InvokeEffectsEvent(EModularButtonEventFlag.EVENT_FOCUS_LOST);
		
		return m_eEventReturnValue & EModularButtonEventHandler.FOCUS_LOST;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] s
	void _print(string s)
	{
		Print(string.Format("[SCR_ModularButtonComponent] %1: %2", GetRootWidget().GetName(), s), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsFocusOnMouseEnter()
	{
		return m_bFocusOnMouseEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] focus
	void SetIsFocusOnMouseEnter(bool focus)
	{
		m_bFocusOnMouseEnter = focus;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetCurrentStateName()
	{
		return typename.EnumToString(EModularButtonState, m_eState);
	}
}
