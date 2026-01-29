// Base buttton that changes visuals on being hovered over, but no further functionality

//------------------------------------------------------------------------------------------------

class HoverButtonComponent : ScriptedWidgetComponent
{
	[Attribute("false", UIWidgets.CheckBox, "")]
	protected bool m_bColorizeChildren;
	
	[Attribute("1 1 1 0", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonDefault;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonHovered;

	[Attribute("1 1 1 0", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonDisabled;

	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildDefault;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildHovered;

	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildDisabled;
	
	[Attribute("0.2", UIWidgets.EditBox, "")]
	protected float m_fAnimationLength;
	
	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSoundClicked;

	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	protected string m_sSoundHovered;
	
	protected Widget m_wChild;
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		
		m_fAnimationLength = 1 / m_fAnimationLength;

		if (!w.IsEnabled())
		{
			w.SetColor(m_ColorButtonDisabled);
			if (m_wChild)
				w.SetColor(m_ColorChildDisabled);
		}
		else
		{
			w.SetColor(m_ColorButtonDefault);
			if (m_wChild)
			{
				AnimateWidget.Color(m_wChild, m_ColorChildDefault, m_fAnimationLength);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_UISoundEntity.SoundEvent(m_sSoundClicked);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_UISoundEntity.SoundEvent(m_sSoundHovered);
		AnimateWidget.Color(w, m_ColorButtonHovered, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, m_ColorChildHovered, m_fAnimationLength);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		AnimateWidget.Color(w, m_ColorButtonDefault, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, m_ColorChildDefault, m_fAnimationLength);

		return false;
	}
};