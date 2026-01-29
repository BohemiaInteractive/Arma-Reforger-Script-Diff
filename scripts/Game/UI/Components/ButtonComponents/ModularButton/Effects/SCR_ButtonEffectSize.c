[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Size", "m_sWidgetName")]
class SCR_ButtonEffectSize : SCR_ButtonEffectWidgetBase
{
	[Attribute(defvalue: "0.2", UIWidgets.EditBox, "Duration of animation")]
	protected float m_vAnimationTime;
	
	[Attribute()]
	vector m_vDefault;
	
	[Attribute()]
	vector m_vHovered;
	
	[Attribute()]
	vector m_vActivated;
	
	[Attribute()]
	vector m_vActivatedHovered;
	
	[Attribute()]
	vector m_vDisabled;
	
	[Attribute()]
	vector m_vDisabledActivated;
	
	[Attribute()]
	vector m_vClicked;
	
	[Attribute()]
	vector m_vFocusGained;
	
	[Attribute()]
	vector m_vFocusLost;
	
	[Attribute()]
	vector m_vToggledOn;
	
	[Attribute()]
	vector m_vToggledOff;
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		Apply(m_vDefault, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		Apply(m_vHovered, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		Apply(m_vActivated, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(m_vActivatedHovered, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(m_vDisabled, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_vDisabledActivated, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked(bool instant)
	{
		Apply(m_vClicked, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		Apply(m_vFocusGained, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		Apply(m_vFocusLost, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		Apply(m_vToggledOn, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		Apply(m_vToggledOff, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDisabled()
	{
		AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationFrameSize);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Apply(vector value, bool instant)
	{
		if (m_wTarget)
		{
			if (!instant && m_vAnimationTime != 0)
			{
				float size[2] = {value[0], value[1]};
				AnimateWidget.Size(m_wTarget, size, 1 / m_vAnimationTime);
			}
			else
			{
				AnimateWidget.StopAnimation(m_wTarget, WidgetAnimationFrameSize);
				FrameSlot.SetSize(m_wTarget, value[0], value[1]);
			}
		}
	}
}
