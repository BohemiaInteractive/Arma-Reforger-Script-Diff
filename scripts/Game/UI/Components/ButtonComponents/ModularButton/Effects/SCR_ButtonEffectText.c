//! Effect which changes text of a text or rich text widget.
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Text", "m_sWidgetName")]
class SCR_ButtonEffectText : SCR_ButtonEffectWidgetBase
{
	[Attribute()]
	string m_sDefault;
	
	[Attribute()]
	string m_sHovered;
	
	[Attribute()]
	string m_sActivated;
	
	[Attribute()]
	string m_sActivatedHovered;
	
	[Attribute()]
	string m_sDisabled;
	
	[Attribute()]
	string m_sDisabledActivated;
	
	[Attribute()]
	string m_sFocusGained;
	
	[Attribute()]
	string m_sFocusLost;
	
	[Attribute()]
	string m_sToggledOn;
	
	[Attribute()]
	string m_sToggledOff;
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		Apply(m_sDefault);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		Apply(m_sHovered);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		Apply(m_sActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(m_sActivatedHovered);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(m_sDisabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_sDisabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		Apply(m_sFocusGained);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		Apply(m_sFocusLost);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		Apply(m_sToggledOn);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		Apply(m_sToggledOff);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Apply(string text)
	{
		TextWidget tw = TextWidget.Cast(m_wTarget);
		
		if (!tw)
			return;
		
		tw.SetText(text);
	}
}
