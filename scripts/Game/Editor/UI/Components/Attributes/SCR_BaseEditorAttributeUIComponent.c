/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_BaseEditorAttributeUIComponent: ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sUiComponentName;
	
	[Attribute("TickboxHolder")]
	protected string m_sTickBoxAttributeName;
	
	[Attribute("AttributeHolder")]
	protected string m_sAttributeHolder;
	
	[Attribute("GamePadLockedSelector")]
	protected string m_sGamePadLockedSelectorName;
	
	[Attribute("SubAttributeIndicator", desc: "Shown when attribute is a subattribute")]
	protected string m_sSubAttributeIndicatorName;
	
	[Attribute("0.25", desc: "Subattribute indicator disabled alpha")]
	protected float m_fSubAttributeDisabledAlphaColor;
	
	protected ref SCR_EditorAttributeUIInfo m_ButtonDescriptionUIInfo = new SCR_EditorAttributeUIInfo;
	protected SCR_AttributeButtonUIComponent m_ActiveButtonDescription;
	
	protected SCR_ChangeableComponentBase m_UIComponent;
	private SCR_BaseEditorAttribute m_Attribute;
	protected float m_fBottomPadding = 1;
	protected bool m_bIsSubAttribute;
	protected SCR_AttributeTickboxUIComponent m_TickBoxAttribute;
	protected Widget m_GamePadLockedSelector;
	protected SCR_AttributesManagerEditorComponent m_AttributeManager;
	protected InputManager m_InputManager;
	protected Widget m_SubAttributeIndicator;
	protected Widget m_wAttributeHolder;
	
	protected bool m_bEnabledByAttribute;
	protected bool m_bEnabledByTickbox;
	protected bool m_bIsFocused;
	
	//Attribute desciption
	protected bool m_bIsShowingDescription;
	protected bool m_bShowButtonDescription;
	protected string m_sButtonDescription;
	protected string m_sButtonDescriptionParam1;
	
	protected ref ScriptInvoker Event_OnAttributeChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnEnabledByAttribute = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnAttributeUIFocusChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnInputDeviceChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnMouseLeave = new ScriptInvoker;
	
	
	//============================ Getters ============================\\
	/*!
	Get attribute this component represents.
	\return Editor attribute
	*/
	SCR_BaseEditorAttribute GetAttribute()
	{
		return m_Attribute;
	}
	
	//============================ Set from var ============================\\
	protected void SetFromVarExternal(SCR_BaseEditorAttributeVar var, bool isReset)
	{		
		//If Reset
		if (isReset)
		{
			GetAttribute().SetConflictingAttributeWasReset(false);
			if (GetAttribute().GetHasConflictingValues())
			{		
				SetVariableToDefaultValue(m_Attribute.GetVariableOrCopy());
				
				if (m_TickBoxAttribute.GetToggled())
					m_TickBoxAttribute.ToggleTickbox(false);
			}
		}
		
		SetFromVar(var);
	}
	
	/*!
	Update GUI from attribute variable. Called when attributes are initialized in a dialog or when they are reset based on user request.
	\param var Attribute variable
	*/
	void SetFromVar(SCR_BaseEditorAttributeVar var)
	{	
		SetFromVarOrDefault();
	}
	
	protected void SetFromVarOrDefault()
	{
		array<SCR_BaseAttributeDynamicDescription> dynamicDescriptionArray = {};
		m_Attribute.GetDynamicDescriptionArray(dynamicDescriptionArray);
		
		//~ Init dynamic descriptions
		foreach(SCR_BaseAttributeDynamicDescription discription: dynamicDescriptionArray)
		{
			discription.InitDynamicDescription(m_Attribute, this);
		}
		
		//~ Update discription if showing
		if (m_bIsShowingDescription)
			ShowAttributeDescription();
	}
	
	//============================ Init ============================\\
	/*!
	Initialize GUI from attribute.
	To be overridden by inherited classes.
	\param w Widget this component is attached to
	\param attribute Editor attribute this component represents
	*/
	void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		m_Attribute = attribute;
		m_AttributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		
		if (m_Attribute.GetOnExternalnChange())
			m_Attribute.GetOnExternalnChange().Insert(SetFromVarExternal);
		m_Attribute.GetOnVarChanged().Insert(SetFromVar);
		m_Attribute.GetOnToggleEnable().Insert(ToggleEnableAttribute);
		m_Attribute.GetOnToggleButtonSelected().Insert(ToggleButtonSelected);
		m_Attribute.GetOnSetAsSubAttribute().Insert(SetAsSubAttribute);
		
		m_SubAttributeIndicator = w.FindAnyWidget(m_sSubAttributeIndicatorName);
		
		m_InputManager = GetGame().GetInputManager();
		
		Widget tickbox = w.FindAnyWidget(m_sTickBoxAttributeName);
		if (!tickbox)
		{
			Print(string.Format("SCR_BaseEditorAttributeUIComponent could not find tickbox: %1!", m_sTickBoxAttributeName), LogLevel.ERROR);
			return;
		}
			
		m_TickBoxAttribute = SCR_AttributeTickboxUIComponent.Cast(tickbox.FindHandler(SCR_AttributeTickboxUIComponent));
		
		m_GamePadLockedSelector = w.FindAnyWidget(m_sGamePadLockedSelectorName);
		
		if (m_GamePadLockedSelector)
		{
			SCR_OnFocusUIComponent focusComponent = SCR_OnFocusUIComponent.Cast(m_GamePadLockedSelector.FindHandler(SCR_OnFocusUIComponent));
			
			if (focusComponent)
				focusComponent.GetOnFocusChanged().Insert(GamePadLockedSelectorFocusChanged);
		}
		
		if (m_TickBoxAttribute)
			m_TickBoxAttribute.GetOnToggleChanged().Insert(OnTickboxToggleChanged);
		
		//m_AttributeSizeLayout = SizeLayoutWidget.Cast(w.FindAnyWidget(m_sAttributeSizeLayoutName));
		
		Widget uiComponentWidget = w.FindAnyWidget(m_sUiComponentName);
		if (uiComponentWidget)
		{
			m_UIComponent = SCR_ChangeableComponentBase.Cast(uiComponentWidget.FindHandler(SCR_ChangeableComponentBase));
			
			if (m_UIComponent)
				attribute.GetUIInfo().SetNameTo(m_UIComponent.GetLabel());
		}
		
		//Check if is disabled by means of overriding varriables
		m_bEnabledByTickbox = (!attribute.GetIsMultiSelect()) || (attribute.GetIsMultiSelect() && !attribute.GetHasConflictingValues() || (attribute.GetHasConflictingValues() && attribute.GetIsOverridingValues()));
		
		//If not multi select it can be disabled and enabled
		ToggleEnableAttribute(attribute.IsEnabled());
		
		if (attribute.GetIsSubAttribute())
			SetAsSubAttribute();
		
		//Padding
		LayoutSlot.SetPadding(w,0,0,0,m_fBottomPadding);
		
		//Override attributes link
		typename linkedOverrideAttributeType = typename.Empty;		
		
		if (attribute.GetHasConflictingValues() && !GetAttribute().GetInitCalled())
		{
			GetGame().OnInputDeviceIsGamepadInvoker().Insert(SetGamepadLockSelectorActive);
		}

		//Multiselect
		if (attribute.GetIsMultiSelect())
		{	
			//If attribute init or reset was called make sure that attribute settings are reset and default values are set
			if (attribute.GetHasConflictingValues() && (!GetAttribute().GetInitCalled() || GetAttribute().GetConflictingAttributeWasReset()))
			{
				if (!attribute.GetInitCalled())
					attribute.SetInitCalled(true);
				else if (attribute.GetConflictingAttributeWasReset())
					attribute.SetConflictingAttributeWasReset(false);
				
				attribute.ResetAttribute();
				SetVariableToDefaultValue(m_Attribute.GetVariableOrCopy());			
			}
		}
		//Override
		else
		{			
			array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
			attribute.GetEntries(entries);
			
			foreach (SCR_BaseEditorAttributeEntry entry: entries)
			{
				SCR_EditorAttributeEntryOverride overrideEntry = SCR_EditorAttributeEntryOverride.Cast(entry);
				
				if (overrideEntry)
				{
					//attribute.SetHasConflictingValues(true);
					bool isOverride;
					overrideEntry.GetToggleStateAndTypename(isOverride, linkedOverrideAttributeType);
					attribute.SetIsOverridingValues(isOverride);
					
					break;
				}
			}
		}
		
		if (attribute.GetIsMultiSelect())
		{
			if (attribute.GetHasConflictingValues())
				m_TickBoxAttribute.InitTickbox(attribute.GetIsOverridingValues(), this);	
			else 
				m_TickBoxAttribute.InitDisabled();
		}
		//If values are overriden by another attribute
		else if (attribute.GetHasConflictingValues())
		{
			m_TickBoxAttribute.InitTickbox(attribute.GetIsOverridingValues(), this, linkedOverrideAttributeType);	
		}
		
		// Setup attribut holder 
		m_wAttributeHolder = w.FindAnyWidget(m_sAttributeHolder);
		if (m_wAttributeHolder)
		{
			Widget child = m_wAttributeHolder.GetChildren();
			if (child)
			{
				// Setup event handler and callbacks 
				SCR_EventHandlerComponent eventHandler = new SCR_EventHandlerComponent();
				eventHandler.GetOnFocus().Insert(OnFocusAttributeWidget);
				child.AddHandler(eventHandler);
				
			}
		}
	}
	
	//============================ Set button states ============================\\
	//Button boxs only, toggle the slection state of buttons
	protected void ToggleButtonSelected(bool selected, int index, bool animated = true)
	{
	}
	
	//============================ UI Logics ============================\\
	//Sets an indent on the label if it is a child of another attribute
	protected void SetAsSubAttribute()
	{
		m_bIsSubAttribute = true;
		m_Attribute.GetOnSetAsSubAttribute().Remove(SetAsSubAttribute);
		
		if (!m_SubAttributeIndicator) 
		{
			Print("SCR_BaseEditorAttributeUIComponent is lacking subattribute indicator and can never be shown as a subattribute", LogLevel.WARNING);
			return;
		}
		
		m_SubAttributeIndicator.SetVisible(true);
		
		if (GetAttribute().GetHasConflictingValues())
			m_TickBoxAttribute.ToggleEnableByAttribute(false);
	}

	//============================ Varriable changed ============================\\
	override bool OnChange(Widget w, bool finished)
	{
		return OnChangeInternal(w, 0, 0, finished);
		
	}

	bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{			
		AttributeValueChanged();
		return false;
	}
	
	//On attribute value changed
	protected void AttributeValueChanged()
	{
		m_Attribute.UpdateInterlinkedVariables(m_Attribute.GetVariable(), m_AttributeManager);
		m_Attribute.PreviewVariable(true, m_AttributeManager);
		
		//~ Update discription
		ShowAttributeDescription();
		Event_OnAttributeChanged.Invoke();
	}
	
	//============================ Set tooltip ============================\\
	protected void ShowAttributeDescription()
	{
		if (!m_AttributeManager)
			return;
		
		m_bIsShowingDescription = true;
		
		//~ Always show tickbox description first
		if (!m_TickBoxAttribute.GetToggled() && m_TickBoxAttribute.IsVisibleAndEnabled())
		{
			m_AttributeManager.SetAttributeDescription(m_AttributeManager.GetConflictingAttributeUIInfo(), m_AttributeManager.GetConflictingAttributeUIInfo().GetDescription());
			return;
		}
		
		array<SCR_BaseAttributeDynamicDescription> dynamicDescriptionArray = {};
		m_Attribute.GetDynamicDescriptionArray(dynamicDescriptionArray);
		
		//~ Get first valid description
		SCR_BaseAttributeDynamicDescription dynamicDescription;
		foreach(SCR_BaseAttributeDynamicDescription discription: dynamicDescriptionArray)
		{
			if (discription.IsValid(m_Attribute, this))
			{
				dynamicDescription = discription;
				break;
			}
		}
		
		//~ Get button dynamic
		SCR_BaseButtonAttributeDynamicDescription buttonDynamicDescription;
		if (dynamicDescription)
		{
			buttonDynamicDescription = SCR_BaseButtonAttributeDynamicDescription.Cast(dynamicDescription);
		}
		
		//~ Button description
		if (m_bShowButtonDescription && (!buttonDynamicDescription || !buttonDynamicDescription.HasPriorityOverButton()))
		{
			m_AttributeManager.SetAttributeDescription(m_ButtonDescriptionUIInfo, string.Empty, m_sButtonDescriptionParam1);
			return;
		}
		
		//~ Dynamic description	
		if (dynamicDescription)
		{
			SCR_EditorAttributeUIInfo uiInfo;
			string param1, param2, param3;
			
			dynamicDescription.GetDescriptionData(m_Attribute, this, uiInfo, param1, param2, param3);
			
			if (!uiInfo)
			{
				Print("SCR_BaseEditorAttributeUIComponent 'dynamicDescription' is missing UIInfo this means a dynamic description was added but the UIInfo was not set!", LogLevel.WARNING);
				m_AttributeManager.SetAttributeDescription(GetAttribute().GetUIInfo());
				return;
			}
			
			string descr = uiInfo.GetDescription();
			
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(descr))
			{
				Print("'SCR_BaseEditorAttributeUIComponent' dynamicDescription does not have a description assigned! This means the description is not set!", LogLevel.WARNING);
				m_AttributeManager.SetAttributeDescription(GetAttribute().GetUIInfo());
				return;
			}

			m_AttributeManager.SetAttributeDescription(uiInfo, descr, param1, param2, param3);
			return;
		}
			
		//~ Default attribute description
		m_AttributeManager.SetAttributeDescription(GetAttribute().GetUIInfo());
	}
	
	protected void HideAttributeDescription()
	{
		if (!m_AttributeManager)
			return;
		
		m_bIsShowingDescription = false;
		m_AttributeManager.SetAttributeDescription(null);
	}	
	
	void ShowButtonDescription(SCR_AttributeButtonUIComponent button, bool showButtonDescription, string buttonDescription = string.Empty)
	{				
		//~ If trying to hide description but that description is not active ignore it
		if (showButtonDescription)
			m_ActiveButtonDescription = button;
		else if (m_ActiveButtonDescription != button && button != null)
			return;
			
		if (buttonDescription.IsEmpty())
			m_bShowButtonDescription = false;
		else 
			m_bShowButtonDescription = showButtonDescription;
		
		m_sButtonDescriptionParam1 = buttonDescription;
		
		//~ Override description to that of the button
		ShowAttributeDescription();
	}
	
	//============================ Script Invokers ============================\\
	/*!
	Called on attribute changed via UI.
	\return Event_OnAttributeChanged ScriptInvoker
	*/
	ScriptInvoker GetOnAttributeChanged()
	{
		return Event_OnAttributeChanged;
	}
	
	/*!
	Called on focus changes. For gamepad to know if the attribute is being focused on
	\return Event_OnAttributeUIFocusChanged ScriptInvoker
	*/
	ScriptInvoker GetOnAttributeUIFocusChanged()
	{
		return Event_OnAttributeUIFocusChanged;
	}	
	
	/*!
	Called when another attribute enables/disables the attribute
	\return Event_OnEnabledByAttribute ScriptInvoker when on enabled by attribute happens
	*/
	ScriptInvoker GetOnEnabledByAttribute()
	{
		return Event_OnEnabledByAttribute;
	}	
	
	/*!
	Called when the mouse leaves the attribute
	\return Event_OnMouseLeave ScriptInvoker on mouse leave
	*/
	ScriptInvoker GetOnMouseLeave()
	{
		return Event_OnMouseLeave;
	}	
	
	//============================ Enabling and Disabling UI and Tickbox ============================\\
	//Sets a default state for the UI and var value if conflicting attribute
	protected void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{
		SetFromVarOrDefault();
	}
	
	protected void ToggleEnableAttribute(bool enabled)
	{
		m_bEnabledByAttribute = enabled;
		ToggleEnable(m_bEnabledByAttribute);
		Event_OnEnabledByAttribute.Invoke(m_bEnabledByAttribute);
		
		if (GetAttribute().GetHasConflictingValues())
			m_TickBoxAttribute.ToggleEnableByAttribute(enabled);
	}
	
	
	/*!
	Toggle UI enabled. Disabled will delete the var if is conflicting var
	/param bool enable true or false
	*/
	protected void ToggleEnable(bool enabled)
	{		
		if (m_SubAttributeIndicator)
		{
			Color color = Color.FromInt(m_SubAttributeIndicator.GetColor().PackToInt());
			if (!enabled)
				color.SetA(m_fSubAttributeDisabledAlphaColor);
			else 
				color.SetA(1);
		
			m_SubAttributeIndicator.SetColor(color);
		}
		
		//If Multiselect
		if (GetAttribute().GetHasConflictingValues())
		{
			//~ Create var from copy var if enabled
			if (enabled)
			{
				//~ Only do this if copy existis
				if (m_Attribute.GetCopyVariable())
				{
					GetAttribute().SetVariable(m_Attribute.GetCopyVariable());
					GetAttribute().ClearCopyVar();
				}
			}
			//~ Create copy var and Delete var if disabled
			else 
			{
				//~ Only do this if var existis
				if (GetAttribute().GetVariable(false))
				{
					GetAttribute().CreateCopyVariable();
					GetAttribute().ClearVar();
				}
			}
		}
		
		if (m_bEnabledByAttribute && m_bEnabledByTickbox)
			m_UIComponent.SetEnabled(true);
		else
			m_UIComponent.SetEnabled(false);

	}
	
	/*!
	Get if tickbox is enabled or not. 
	\return Tickbox is enabled true or false
	*/
	bool GetTickboxEnabled()
	{
		return m_TickBoxAttribute && m_TickBoxAttribute.GetEnabled();
	}
	
	/*!
	Toggle tickbox UI, flipping the tickbox from true to false and viceversa
	*/
	void ToggleEnableAttributeTickbox()
	{
		if (GetAttribute().GetHasConflictingValues())
			m_TickBoxAttribute.ToggleTickbox(!GetAttribute().GetIsOverridingValues());
		
		if (GetAttribute().GetIsOverridingValues())
			GetGame().GetWorkspace().SetFocusedWidget(m_UIComponent.GetRootWidget());
		else
			GetGame().GetWorkspace().SetFocusedWidget(m_GamePadLockedSelector);
	}
	
	//Listens to tickbox if the state changed
	protected void OnTickboxToggleChanged(bool toggle)
	{
		if (GetAttribute().GetHasConflictingValues())
		{
			GetAttribute().SetIsOverridingValues(toggle);
		}
		
		m_bEnabledByTickbox = toggle;
		ToggleEnable(toggle);
		
		SetGamepadLockSelectorActive(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());

		if (toggle)
			AttributeValueChanged();
		
		if (m_bIsShowingDescription)
			ShowAttributeDescription();
	}
	
	//If Disabled attribute is focused or not with gamepad
	protected void GamePadLockedSelectorFocusChanged(bool newFocus)
	{
		if (!GetAttribute().GetHasConflictingValues())
			return;
		
		if (newFocus)
		{
			ShowAttributeDescription();
			Event_OnAttributeUIFocusChanged.Invoke(this);
		}
			
		else
		{
			HideAttributeDescription();
			Event_OnAttributeUIFocusChanged.Invoke(null);
		}			
	}
	
	/*!
	Get if attribute is focused
	/return if attribute is focused
	*/
	bool GetIsFocused()
	{
		return m_bIsFocused;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for focus of widget used
	protected void OnFocusAttributeWidget(Widget w)
	{
		OnFocus(w, 0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//If enabled UI is focused
	override bool OnFocus(Widget w, int x, int y)
	{
		m_bIsFocused = true;
		
		if (!m_InputManager.IsUsingMouseAndKeyboard())
			ShowButtonDescription(null, false);
		
		if (GetAttribute().GetHasConflictingValues())
			Event_OnAttributeUIFocusChanged.Invoke(this);
		
		ShowAttributeDescription();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//If enabled UI is lost focus
	override bool OnFocusLost(Widget w, int x, int y)
	{		
		m_bIsFocused = false; 
		
		if (GetAttribute().GetHasConflictingValues())
			Event_OnAttributeUIFocusChanged.Invoke(null);
		
		return false;
	}	
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		return OnFocus(w, x, y);
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{	
		Event_OnMouseLeave.Invoke();
		return false;
	}
	
	//For tickbox
	protected void SetGamepadLockSelectorActive(bool isGamepad)
	{
		m_GamePadLockedSelector.SetVisible(isGamepad && !m_TickBoxAttribute.GetToggled());
	}
	
	//============================ On Destroy ============================\\
	override void HandlerDeattached(Widget w)
	{
		if (m_Attribute)
		{
			if (m_Attribute.GetOnExternalnChange())
				m_Attribute.GetOnExternalnChange().Remove(SetFromVarExternal);
		
			m_Attribute.GetOnVarChanged().Remove(SetFromVar);
			m_Attribute.GetOnToggleEnable().Remove(ToggleEnableAttribute);
			m_Attribute.GetOnToggleButtonSelected().Remove(ToggleButtonSelected);
			
			if (!m_bIsSubAttribute)
				m_Attribute.GetOnSetAsSubAttribute().Remove(SetAsSubAttribute);
			
			if (m_Attribute.GetHasConflictingValues())
				GetGame().OnInputDeviceIsGamepadInvoker().Remove(SetGamepadLockSelectorActive);
		} 
		
		if (m_TickBoxAttribute)
			m_TickBoxAttribute.GetOnToggleChanged().Remove(OnTickboxToggleChanged);	
		
		
		if (m_GamePadLockedSelector)
		{
			SCR_OnFocusUIComponent focusComponent = SCR_OnFocusUIComponent.Cast(m_GamePadLockedSelector.FindHandler(SCR_OnFocusUIComponent));
			
			if (focusComponent)
				focusComponent.GetOnFocusChanged().Remove(GamePadLockedSelectorFocusChanged);
		}
	}
};