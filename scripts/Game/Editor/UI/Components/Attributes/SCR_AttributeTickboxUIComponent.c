//! @ingroup Editor_UI Editor_UI_Components

//! Using a controller will show/hide the Select button hint if the attribute with this script is focused on
class SCR_AttributeTickboxUIComponent : ScriptedWidgetComponent
{	
	
	[Attribute()]
	protected string m_sTickBoxImageName;

	[Attribute()]
	protected string m_sTickBoxButtonName;
		
	// Visuals
	[Attribute()]
	protected ResourceName m_sTickBoxImageToggled;

	[Attribute()]
	protected ResourceName m_sTickBoxImageUntoggled;
	
	// Reference
	protected SCR_BaseEditorAttributeUIComponent m_AttributeUI;
	protected typename m_LinkedOverrideAttributeType = typename.Empty;
	protected Widget m_Root;
	protected ImageWidget m_TickBoxImage;
	protected Widget m_TickBoxButton;
	
	protected float m_fDisabledAlphaColor = 0.25;
	
	// States
	protected bool m_bToggled;
	protected bool m_bEnabled = true;
	
	// Events
	protected ref ScriptInvokerBool m_OnToggleChanged;
	
	//------------------------------------------------------------------------------------------------
	//! Set if tickbox is toggled or not.
	//! \param toggled If toggled editing the attribute is enabled (unless the attribute itself is disabled)
	void ToggleTickbox(bool toggled)
	{		
		if (!m_bEnabled)
			return;
		
		m_bToggled = toggled;
		
		if (m_OnToggleChanged)
			m_OnToggleChanged.Invoke(toggled);
		
		if (toggled)
		{
			m_TickBoxImage.LoadImageTexture(0, m_sTickBoxImageToggled);
			m_TickBoxImage.SetImage(0);
		}
		else 
		{
			m_TickBoxImage.LoadImageTexture(0, m_sTickBoxImageUntoggled);
			m_TickBoxImage.SetImage(0);
		}
		
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (!attributesManager)
			return;
				
		if (m_LinkedOverrideAttributeType != typename.Empty)
		{
			SCR_BaseEditorAttributeVar overideValue = new SCR_BaseEditorAttributeVar();
			overideValue.SetBool(toggled);
			attributesManager.SetAttributeVariable(m_LinkedOverrideAttributeType, overideValue);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! If subAttribute toggles the tickbox if the 'parent' attribute disabled the attribute
	//! \param enabled, If true sets itself as enabled. Else disables it
	void ToggleEnableByAttribute(bool enabled)
	{
		SetEnabled(enabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonToggle()
	{
		ToggleTickbox(!m_bToggled);
	}

	//------------------------------------------------------------------------------------------------
	//! Get if tickbox is toggled or not.
	//! \return m_bToggled, If toggled editing the attribute is enabled (unless the attribute itself is disabled)
	bool GetToggled()
	{		
		return m_bToggled;
	}

	//------------------------------------------------------------------------------------------------
	//! Get if tickbox is visible and enabled
	//! \return false if the tickbox is either disabled or invisible, true otherwise
	bool IsVisibleAndEnabled()
	{
		return m_TickBoxButton.IsEnabled() && m_Root.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnToggleChanged()
	{
		if (!m_OnToggleChanged)
			m_OnToggleChanged = new ScriptInvokerBool();

		return m_OnToggleChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Set tickbox's visibility
	//! \param frameVisible
	//! \param buttonVisible
	protected void SetVisible(bool frameVisible, bool buttonVisible)
	{	
		m_Root.SetVisible(frameVisible);
		m_TickBoxButton.SetVisible(buttonVisible);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEnabled(bool enabled)
	{		
		m_bEnabled = enabled;
		
		//m_TickBoxImage.LoadImageTexture(0, m_sNonConfictingTickBoxImage);
		Color color = Color.FromInt(m_TickBoxImage.GetColor().PackToInt());
		if (!enabled)
			color.SetA(m_fDisabledAlphaColor);
		else 
			color.SetA(1);
		
		m_TickBoxImage.SetColor(color);
		
		m_TickBoxButton.SetEnabled(enabled);
	}

	//------------------------------------------------------------------------------------------------
	//! Get if tickbox is enabled or not.
	//! \return Tickbox is enabled true or false
	bool GetEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! On init as overriding attributes (multiple entities with the same attribute) and one or more of the values are conflicting with each other
	//! This will enable the tickbox and allow the value to be overridden for all
	//! \param toggleState
	//! \param attributeUI
	//! \param linkedOverrideAttributeType
	void InitTickbox(bool toggleState, SCR_BaseEditorAttributeUIComponent attributeUI, typename linkedOverrideAttributeType = typename.Empty)
	{
		m_LinkedOverrideAttributeType = linkedOverrideAttributeType;
		
		SetVisible(true, true);
		ToggleTickbox(toggleState);
		m_AttributeUI = attributeUI;
		
		if (m_AttributeUI)
		{
			m_AttributeUI.GetOnEnabledByAttribute().Insert(OnAttributeEnabledByAttribute);
			SetEnabled(m_AttributeUI.GetAttribute().IsEnabled());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! On init as overriding attributes (multiple entities with the same attribute) but non of them are conflicting dus show the tickbox but disable it
	void InitDisabled()
	{
		SetVisible(true, true);
		SetEnabled(false);
		m_bToggled = true;
		m_TickBoxImage.LoadImageTexture(0, m_sTickBoxImageToggled);
		m_TickBoxImage.SetImage(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAttributeEnabledByAttribute(bool enabled)
	{
		SetEnabled(enabled);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		
		m_TickBoxImage = ImageWidget.Cast(w.FindAnyWidget(m_sTickBoxImageName));
		if (!m_TickBoxImage)
			return;
		
		m_TickBoxButton = w.FindAnyWidget(m_sTickBoxButtonName);
		if (!m_TickBoxButton)
			return;
		
		ScriptInvoker onButton = ButtonActionComponent.GetOnAction(w, m_sTickBoxButtonName);
		if (onButton) onButton.Insert(OnButtonToggle);	
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_AttributeUI)
			m_AttributeUI.GetOnEnabledByAttribute().Remove(OnAttributeEnabledByAttribute);
	}
}
