//! Effect which sets a widget visible (SetVisible).
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Visibility", "m_sWidgetName")]
class SCR_ButtonEffectVisibility : SCR_ButtonEffectWidgetBase
{	
	[Attribute()]
	bool m_bDefault;
	
	[Attribute()]
	bool m_bHovered;
	
	[Attribute()]
	bool m_bActivated;
	
	[Attribute()]
	bool m_bActivatedHovered;

	[Attribute()]
	bool m_bDisabled;
	
	[Attribute()]
	bool m_bDisabledActivated;	
		
	[Attribute()]
	bool m_bClicked;
	
	[Attribute()]
	bool m_bFocusGained;
	
	[Attribute()]
	bool m_bFocusLost;
	
	[Attribute()]
	bool m_bToggledOn;
	
	[Attribute()]
	bool m_bToggledOff;
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		Apply(m_bDefault, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		Apply(m_bHovered, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		Apply(m_bActivated, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(m_bActivatedHovered, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnClicked(bool instant)
	{
		Apply(m_bClicked, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(m_bDisabled, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_bDisabledActivated, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		Apply(m_bFocusGained, instant);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		Apply(m_bFocusLost, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		Apply(m_bToggledOn, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		Apply(m_bToggledOff, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when effect is disabled. Here you should stop all running effects.
	override void OnDisabled()
	{
		AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationOpacity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Apply(bool visible, bool instant)
	{
		m_wTarget.SetVisible(visible);
	}
}
