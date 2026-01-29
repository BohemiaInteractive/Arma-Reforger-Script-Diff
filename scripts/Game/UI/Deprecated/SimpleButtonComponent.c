// Simple button provides 4 states in which it can be:
// Default, focused, pressed and disabled.
// Hovering over the button will focus it instantly.
// Use it in places where you don't need onHover functionality.

// TODO:
// Need events OnDisable and onEnable to recolorize widgets

class SimpleButtonComponent : ScriptedWidgetComponent
{
	[Attribute("0 0 0 0.7", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonDefault;
	
	[Attribute("1 1 1 0.25", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonFocused;
	
	[Attribute("1 1 1 0.3", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonPressed;

	[Attribute("0.8 0.8 0.8 0.1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorButtonDisabled;
	
	[Attribute("false", UIWidgets.CheckBox, "")]
	protected bool m_bColorizeChildren;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildDefault;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildFocused;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildPressed;

	[Attribute("1 1 1 0.6", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildDisabled;
	
	[Attribute("0.2", UIWidgets.EditBox, "")]
	protected float m_fAnimationLength;

	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSoundClicked;
	
	[Attribute(SCR_SoundEvent.FOCUS, UIWidgets.EditBox, "")]
	protected string m_sSoundFocused;

	
	protected Widget m_wChild;
	protected bool m_bIsFocused = false;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wChild = w.GetChildren();
		m_fAnimationLength = 1 / m_fAnimationLength;
		
		if (!w.IsEnabled())
		{
			w.SetColor(m_ColorButtonDisabled);
			if (m_bColorizeChildren && m_wChild)
				m_wChild.SetColor(m_ColorChildDisabled);
		}
		else
		{
			w.SetColor(m_ColorButtonDefault);
			if (m_bColorizeChildren && m_wChild)
				m_wChild.SetColor(m_ColorChildDefault);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_UISoundEntity.SoundEvent(m_sSoundClicked);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnClickSound(string newOnClickedSound)
	{
		m_sSoundClicked = newOnClickedSound;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		AnimateWidget.Color(w, m_ColorButtonPressed, m_fAnimationLength);
		if (m_bColorizeChildren && m_wChild)
			AnimateWidget.Color(m_wChild, m_ColorChildPressed, m_fAnimationLength);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (w.IsEnabled())
			AnimateWidget.Color(w, m_ColorButtonFocused, m_fAnimationLength);
		else
			AnimateWidget.Color(w, m_ColorButtonDisabled, m_fAnimationLength);
		
		if (m_bColorizeChildren && m_wChild)
		{
			if (w.IsEnabled())
				AnimateWidget.Color(m_wChild, m_ColorChildFocused, m_fAnimationLength);
			else
				AnimateWidget.Color(m_wChild, m_ColorChildDisabled, m_fAnimationLength);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_bIsFocused = true;
		SCR_UISoundEntity.SoundEvent(m_sSoundFocused);
		
		if (w.IsEnabled())
			AnimateWidget.Color(w, m_ColorButtonFocused, m_fAnimationLength);
		else
			AnimateWidget.Color(w, m_ColorButtonDisabled, m_fAnimationLength);
		
		if (m_bColorizeChildren && m_wChild)
		{
			if (w.IsEnabled())
				AnimateWidget.Color(m_wChild, m_ColorChildFocused, m_fAnimationLength);
			else
				AnimateWidget.Color(m_wChild, m_ColorChildDisabled, m_fAnimationLength);
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bIsFocused = false;
		
		if (w.IsEnabled())
			AnimateWidget.Color(w, m_ColorButtonDefault, m_fAnimationLength);
		else
			AnimateWidget.Color(w, m_ColorButtonDisabled, m_fAnimationLength);
		
		if (m_bColorizeChildren && m_wChild)
		{
			if (w.IsEnabled())
				AnimateWidget.Color(m_wChild, m_ColorChildDefault, m_fAnimationLength);
			else
				AnimateWidget.Color(m_wChild, m_ColorChildDisabled, m_fAnimationLength);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w.GetFlags() & WidgetFlags.NOFOCUS)
			OnFocus(w, x, y);
		else
			GetGame().GetWorkspace().SetFocusedWidget(w);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w.GetFlags() & WidgetFlags.NOFOCUS)
			OnFocusLost(w, x, y);
		else
			GetGame().GetWorkspace().SetFocusedWidget(enterW);
		
		return false;
	}
};