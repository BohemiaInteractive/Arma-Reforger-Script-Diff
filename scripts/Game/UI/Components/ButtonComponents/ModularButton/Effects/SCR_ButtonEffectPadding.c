//! Effect which changes all padding values of a target widget.
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Padding", "m_sWidgetName")]
class SCR_ButtonEffectPadding : SCR_ButtonEffectWidgetBase
{
	[Attribute(defvalue: "0.2", UIWidgets.EditBox, "Duration of animation")]
	protected float m_fAnimationTime;
	
	[Attribute()]
	float m_fDefault;
	
	[Attribute()]
	float m_fHovered;
	
	[Attribute()]
	float m_fActivated;
	
	[Attribute()]
	float m_fActivatedHovered;
	
	[Attribute()]
	float m_fDisabled;
	
	[Attribute()]
	float m_fDisabledActivated;
	
	[Attribute()]
	float m_fFocusGained;
	
	[Attribute()]
	float m_fFocusLost;
	
	[Attribute()]
	float m_fToggledOn;
	
	[Attribute()]
	float m_fToggledOff;
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		Apply(m_fDefault, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		Apply(m_fHovered, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		Apply(m_fActivated, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(m_fActivatedHovered, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(m_fDisabled, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_fDisabled, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		Apply(m_fFocusGained, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		Apply(m_fFocusLost, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		Apply(m_fToggledOn, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		Apply(m_fToggledOff, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when effect is disabled. Here you should stop all running effects.
	override void OnDisabled()
	{
		AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationPadding);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Apply(float value, bool instant)
	{
		if (m_wTarget)
		{
			if (!instant && m_fAnimationTime != 0)
			{
				float padding[4] = {value, value, value, value};
				AnimateWidget.Padding(m_wTarget, padding, 1/m_fAnimationTime);
			}
			else
			{
				AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationPadding);
				AlignableSlot.SetPadding(m_wTarget, value, value, value, value);
			}
		}
	}
}
