//#define DEBUG_MODULAR_BUTTON

//! Flags for events
//! We inherit state flags in this enum, because state transitions are also used for events
enum EModularButtonEventFlag : EModularButtonState
{
	// These events can coexist, unlike states in EModularButtonState
	EVENT_CLICKED				= 1 << 10, //!< Start with bit 10 to reserve some space for prev. enum
	EVENT_FOCUS_GAINED			= 1 << 11,
	EVENT_FOCUS_LOST			= 1 << 12,
	EVENT_TOGGLED_ON			= 1 << 13,
	EVENT_TOGGLED_OFF			= 1 << 14,
	EVENT_MOUSE_ENTER			= 1 << 15,
	EVENT_MOUSE_LEAVE			= 1 << 16	// !!! This must be the last flag! Base Container formatting relies on this.
}

//! Flags for device input types
enum EModularButtonInputDevice
{
	KEYBOARD	= 1 << 0,
	MOUSE		= 1 << 1,
	GAMEPAD		= 1 << 2,
	JOYSTICK	= 1 << 3
}

//! Base class for modular button effects.
//! Override methods in inherited classes with specific effect implementations.
[BaseContainerProps()]
class SCR_ButtonEffectBase
{
	[Attribute("true", UIWidgets.CheckBox, "Enables this effect")]
	protected bool m_bEnabled;
	
	[Attribute("", UIWidgets.EditBox, "Array of tags, can be used to find this effect or manipulate many effects at a time")]
	ref array<string> m_aTags;
	
	[Attribute("0", UIWidgets.Flags, "Events this effect will listen to.", "", ParamEnumArray.FromEnum(EModularButtonEventFlag) )]
	EModularButtonEventFlag m_eEvents;
	
	[Attribute("63", UIWidgets.Flags, "Input devices which can trigger this effect.", "", ParamEnumArray.FromEnum(EModularButtonInputDevice)) ]
	EModularButtonInputDevice m_eInputDevices;
	
	protected SCR_ModularButtonComponent m_Button; // Parent button which owns this effect
	
	// Public API
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetEnabled(bool enabled)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("SetEnabled: %1", enabled));
		#endif
		
		bool enabledPrev = m_bEnabled;
		m_bEnabled = enabled;
		
		if (enabled != enabledPrev)
		{
			if (enabled)
				m_Button.Internal_OnEffectEnabled(this);
			else
				OnDisabled();
		}
	}
	
	//---------------------------------------------------------------------------------------------
	//! Call it if properties have changed and they must be reapplied.
	void PropertiesChanged()
	{
		if (m_bEnabled)
		{
			m_Button.Internal_OnEffectEnabled(this);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	//! Returns true when the effect contains a given tag.
	bool HasTag(string tag)
	{
		return m_aTags.Contains(tag);
	}
	
	//---------------------------------------------------------------------------------------------
	// Override these methods in inherited classes
	// Instant - when true, means that the effect must be applied immediately (without animations).
	// Typically instant = true at start of the menu.
	protected void OnStateDefault(bool instant);
	protected void OnStateHovered(bool instant);
	protected void OnStateActivated(bool instant);
	protected void OnStateActivatedHovered(bool instant);
	protected void OnStateDisabled(bool instant);
	protected void OnStateDisabledActivated(bool instant);
	protected void OnClicked(bool instant);
	protected void OnFocusGained(bool instant);	
	protected void OnFocusLost(bool instant);
	protected void OnToggledOn(bool instant);
	protected void OnToggledOff(bool instant);
	protected void OnMouseEnter(bool instant);
	protected void OnMouseLeave(bool instant);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] w
	// Called when parent of this effect is attached to a widget
	void OnHandlerAttached(Widget w);
	
	//------------------------------------------------------------------------------------------------
	// Called when effect is disabled. Here you should stop all running effects.
	void OnDisabled();
	
	// Internal methods
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] eventFlag
	//! \param[in] deviceType
	//! \param[in] instant
	//! \return true when effect was consumed
	bool Internal_OnEvent(EModularButtonEventFlag eventFlag, EInputDeviceType deviceType, bool instant = false)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print(string.Format("Internal_OnEvent: %1, %2, %3", typename.EnumToString(EModularButtonEventFlag, eventFlag), typename.EnumToString(EInputDeviceType, deviceType), instant));
		#endif
		
		// Filter out input device
		// But we ignore this check if instant flag is set
		if (!instant)
		{
			EModularButtonInputDevice deviceFlag;
			switch (deviceType)
			{
				case EInputDeviceType.KEYBOARD: deviceFlag = EModularButtonInputDevice.KEYBOARD; break;
				case EInputDeviceType.MOUSE: deviceFlag = EModularButtonInputDevice.MOUSE; break;
				case EInputDeviceType.GAMEPAD: deviceFlag = EModularButtonInputDevice.GAMEPAD; break;
				case EInputDeviceType.JOYSTICK: deviceFlag = EModularButtonInputDevice.JOYSTICK; break;
			}
			
			// Bail if this effect does not listen to this input type
			if ((deviceFlag & m_eInputDevices) == 0)
			{
				#ifdef DEBUG_MODULAR_BUTTON
				_print("Event ignored: device input type doesn't match.");
				#endif
				
				return false;
			}
		}
		
		if (eventFlag & m_eEvents)
		{
			#ifdef DEBUG_MODULAR_BUTTON
			_print("Event passed");
			#endif
			switch (eventFlag)
			{
				case EModularButtonEventFlag.STATE_DEFAULT				: { OnStateDefault(instant); break; }
				case EModularButtonEventFlag.STATE_HOVERED				: { OnStateHovered(instant); break; }
				case EModularButtonEventFlag.STATE_ACTIVATED			: { OnStateActivated(instant); break; }
				case EModularButtonEventFlag.STATE_ACTIVATED_HOVERED	: { OnStateActivatedHovered(instant); break; }
				case EModularButtonEventFlag.STATE_DISABLED				: { OnStateDisabled(instant); break; }		
				case EModularButtonEventFlag.STATE_DISABLED_ACTIVATED	: { OnStateDisabledActivated(instant); break; }		
				case EModularButtonEventFlag.EVENT_CLICKED				: { OnClicked(instant); break; }
				case EModularButtonEventFlag.EVENT_FOCUS_GAINED			: { OnFocusGained(instant); break; }
				case EModularButtonEventFlag.EVENT_FOCUS_LOST			: { OnFocusLost(instant); break; }
				case EModularButtonEventFlag.EVENT_TOGGLED_ON			: { OnToggledOn(instant); break; }
				case EModularButtonEventFlag.EVENT_TOGGLED_OFF			: { OnToggledOff(instant); break; }
				case EModularButtonEventFlag.EVENT_MOUSE_ENTER			: { OnMouseEnter(instant); break; }
				case EModularButtonEventFlag.EVENT_MOUSE_LEAVE			: { OnMouseLeave(instant); break; }
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] parentButton
	void Init(SCR_ModularButtonComponent parentButton)
	{
		m_Button = parentButton;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void _print(string s)
	{
		string tagsStr;
		
		foreach (string tag : m_aTags)
		{
			tagsStr = tagsStr + string.Format("'%1' ", tag);
		}
		
		m_Button._print(string.Format("Effect %1 %2: %3", this, tagsStr, s));
	}
}

//! Button effect which does something with a specific widget.
[BaseContainerProps()]
class SCR_ButtonEffectWidgetBase : SCR_ButtonEffectBase
{
	[Attribute("", UIWidgets.Auto, "Widget name to apply the effect on")]
	protected string m_sWidgetName;
	
	protected  Widget m_wTarget;
	
	//------------------------------------------------------------------------------------------------
	override void OnHandlerAttached(Widget w)
	{
		if (!m_sWidgetName.IsEmpty())
			m_wTarget = w.FindAnyWidget(m_sWidgetName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetTargetWidget()
	{
		return m_wTarget;
	}
}

//! Class for custom title for effects.
//! It will set a custom title to effect objects in workbench in format "m_sTypeStr" or "m_sTypeStr: WIdgetName".
class SCR_ButtonEffectTitleAttribute : BaseContainerCustomTitle
{
	protected string m_sTypeStr;
	protected string m_sVarName;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] typeStr
	//! \param[in] varName
	void SCR_ButtonEffectTitleAttribute(string typeStr, string varName)
	{
		m_sTypeStr = typeStr;
		m_sVarName = varName;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Generate string according to enabled state
		string strEnabled;
		bool enabled;
		source.Get("m_bEnabled", enabled);
		if (enabled)
			strEnabled = "[X]";
		else
			strEnabled = "[  ]";
		
		// Generate string with event flags: "(ON_SOMETHING, ON_SOMETHING_ELSE)"
		string strFlags;
		int events;
		source.Get("m_eEvents", events);
		int flag = 1;
		int endFlag = EModularButtonEventFlag.EVENT_MOUSE_LEAVE << 1;
		int nFlagsAdded = 0;
		while (flag != endFlag)
		{
			if (events & flag)
			{
				if (nFlagsAdded > 0)
					strFlags = strFlags + ", ";
				
				string enumStr = typename.EnumToString(EModularButtonEventFlag, flag);
				enumStr.ToLower();
				strFlags = strFlags + enumStr;
				nFlagsAdded++;
			}
			
			flag = flag << 1;
		}
		
		// Generate string from tags
		string strTags;
		array<string> tags;
		source.Get("m_aTags", tags);
		if (tags)
		{
			foreach (string t : tags)
			{
				strTags = strTags + string.Format("'%1' ", t);
			}
		}

		string targetWidgetName;
		if (!m_sVarName.IsEmpty())
		{
			// Variable name was provided, try to get value of a variable with name m_sVarName
			
			source.Get(m_sVarName, targetWidgetName);
			
			string strOut;
			
			if (!targetWidgetName.IsEmpty())
				strOut = string.Format("%1 %2: %3 %4     (%5)", strEnabled, m_sTypeStr, strTags, targetWidgetName, strFlags);
			else
				strOut = string.Format("%1 (E) %2: %3 Widget name not specified!", strEnabled, m_sTypeStr, strTags);
			
			title = strOut;
			
			return true;
		}
		else
		{
			title = string.Format("%1 %2: %3     (%4)", strEnabled, m_sTypeStr, strTags, strFlags);
			return true;
		}
		
		return true;
	}
}

/*
Template effect:

[BaseContainerProps()]
class SCR_ModularButtonEffect_Example : SCR_ModularButtonEffectBase
{
	override void OnHandlerAttached(Widget w)
	{
	}

	// Events below are called according to the m_eEvents flags!
	// Make sure you set them up!

	override void OnStateDefault(bool instant)
	{
	}

	override void OnStateHovered(bool instant)
	{
	}

	override void OnStateActivated(bool instant)
	{
	}

	override void OnStateActivatedHovered(bool instant)
	{
	}

	override void OnStateDisabled(bool instant)
	{
	}

	override void OnStateDisabledActivated(bool instant)
	{
	}

	override void OnClicked(bool instant)
	{
	}

	override void OnFocusGained(bool instant)
	{
	}
	
	override void OnFocusLost(bool instant)
	{
	}

	override void OnToggledOn(bool instant)
	{
	}

	override void OnToggledOff(bool instant)
	{
	}

	// Called when effect is disabled. Here you should stop all running effects.
	override void OnDisabled()
	{
	}
}
*/
