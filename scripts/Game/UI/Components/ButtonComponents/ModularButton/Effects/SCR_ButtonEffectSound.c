//! Plays a sound.
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Sound", "")]
class SCR_ButtonEffectSound : SCR_ButtonEffectBase
{
	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnClicked(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMouseEnter(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMouseLeave(bool instant)
	{
		Apply(instant);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Apply(bool instant)
	{
		if (!instant && !m_sSound.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sSound);
	}
}
