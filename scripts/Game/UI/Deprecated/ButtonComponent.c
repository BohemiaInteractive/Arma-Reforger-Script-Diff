// More complex button handling 6 states:
// Default, hovered, focused, hovered+focused, clicked, disabled.
// Used for more complex buttons types requiring hovering over a button

class ButtonComponent : SimpleButtonComponent
{
	[Attribute("1 1 1 0.05", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonHovered;
	
	[Attribute("1 1 1 0.25", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonHoveredFocused; 
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildHovered;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildHoveredFocused;
	
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	protected string m_sSoundHovered;

	protected bool m_bIsHovered = false;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		m_ColorButtonHovered = null;
		m_ColorChildHovered = null;
		m_ColorButtonHoveredFocused = null;
		m_ColorChildHoveredFocused = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		AnimateWidget.Color(w, m_ColorButtonHoveredFocused, m_fAnimationLength);

		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, m_ColorChildHoveredFocused, m_fAnimationLength);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_bIsFocused = true;
		SCR_UISoundEntity.SoundEvent(m_sSoundFocused);

		Color buttonColor;
		Color childColor;
		if (m_bIsHovered)
		{
			buttonColor = m_ColorButtonHoveredFocused;
			childColor = m_ColorChildHoveredFocused;
		}
		else
		{
			buttonColor = m_ColorButtonFocused;
			childColor = m_ColorChildFocused;
		}
		
		AnimateWidget.Color(w, buttonColor, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, childColor, m_fAnimationLength);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bIsFocused = false;
		
		Color buttonColor;
		Color childColor;
		if (m_bIsHovered)
		{
			buttonColor = m_ColorButtonHovered;
			childColor = m_ColorChildHovered;
		}
		else
		{
			buttonColor = m_ColorButtonDefault;
			childColor = m_ColorChildDefault;
		}
		
		AnimateWidget.Color(w, buttonColor, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, childColor, m_fAnimationLength);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bIsHovered = true;
		SCR_UISoundEntity.SoundEvent(m_sSoundHovered);

		Color buttonColor;
		Color childColor;
		if (m_bIsFocused)
		{
			buttonColor = m_ColorButtonHoveredFocused;
			childColor = m_ColorChildHoveredFocused;
		}
		else
		{
			buttonColor = m_ColorButtonHovered;
			childColor = m_ColorChildHovered;
		}
		
		AnimateWidget.Color(w, buttonColor, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, childColor, m_fAnimationLength);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bIsHovered = false;
		
		Color buttonColor;
		Color childColor;
		if (m_bIsFocused)
		{
			buttonColor = m_ColorButtonFocused;
			childColor = m_ColorChildFocused;
		}
		else
		{
			buttonColor = m_ColorButtonDefault;
			childColor = m_ColorChildDefault;
		}

		AnimateWidget.Color(w, buttonColor, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, childColor, m_fAnimationLength);

		return false;
	}
};