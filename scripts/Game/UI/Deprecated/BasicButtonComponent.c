class BasicButtonComponent : ScriptedWidgetComponent
{
	static const ref Color COLOR_DEFAULT = new Color(0, 0, 0, 0.6);
	static const ref Color COLOR_FOCUSED = new Color(1, 1, 1, 0.06);
	static const ref Color COLOR_HOVERED = new Color(1, 1, 1, 0.06);
	static const ref Color COLOR_PRESSED = new Color(0.8, 0.8, 0.8, 0.06);
	
	[Attribute(SCR_SoundEvent.FOCUS, UIWidgets.EditBox, "")]
	protected string m_sSoundFocused;
	
	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSoundClicked;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		w.SetColor(COLOR_DEFAULT);
		
		if (!w.IsEnabled())
			w.SetOpacity(0.5);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_UISoundEntity.SoundEvent(m_sSoundClicked);
		AnimateWidget.Color(w, COLOR_PRESSED, UIConstants.FADE_RATE_DEFAULT);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_UISoundEntity.SoundEvent(m_sSoundFocused);
		AnimateWidget.Color(w, COLOR_FOCUSED, UIConstants.FADE_RATE_DEFAULT);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		AnimateWidget.Color(w, COLOR_DEFAULT, UIConstants.FADE_RATE_DEFAULT);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
		return false;
	}
};