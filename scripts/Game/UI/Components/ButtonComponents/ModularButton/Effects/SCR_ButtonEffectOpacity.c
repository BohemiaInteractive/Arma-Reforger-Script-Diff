//! Effect which animated opacity of the target widget.
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Opacity", "m_sWidgetName")]
class SCR_ButtonEffectOpacity : SCR_ButtonEffectWidgetBase
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
	float m_fClicked;
	
	[Attribute()]
	float m_fFocusGained;
	
	[Attribute()]
	float m_fFocusLost;
	
	[Attribute()]
	float m_fToggledOn;
	
	[Attribute()]
	float m_fToggledOff;
	
	[Attribute()]
	float m_fMouseEnter;
	
	[Attribute()]
	float m_fMouseLeave;
	
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
	override void OnClicked(bool instant)
	{
		Apply(m_fClicked, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(m_fDisabled, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_fDisabledActivated, instant);
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
	override void OnMouseEnter(bool instant)
	{
		Apply(m_fMouseEnter, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMouseLeave(bool instant)
	{
		Apply(m_fMouseLeave, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when effect is disabled. Here you should stop all running effects.
	override void OnDisabled()
	{
		AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationOpacity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Apply(float value, bool instant)
	{
		if (m_wTarget)
		{
			if (!instant && m_fAnimationTime != 0)
			{
				AnimateWidget.Opacity(m_wTarget, value, 1 / m_fAnimationTime);
			}
			else
			{
				AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationOpacity);
				m_wTarget.SetOpacity(value);
			}
		}
	}
}
