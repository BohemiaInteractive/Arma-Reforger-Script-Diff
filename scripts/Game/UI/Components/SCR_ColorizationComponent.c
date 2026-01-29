class SCR_ColorizationComponent: ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.EditBox, "Name of the widget to colorize. Leave empty for colorize root widget")]
	string m_sWidgetName;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	ref Color m_ColorDefault;

	[Attribute("true")]
	bool m_bSetHoverColor;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	ref Color m_ColorHover;

	[Attribute("true")]
	bool m_bSetFocusColor;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	ref Color m_ColorFocus;

	[Attribute("true")]
	bool m_bSetClickColor;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	ref Color m_ColorClick;

	[Attribute("true")]
	bool m_bAnimateWidgets;

	[Attribute("0.2", UIWidgets.EditBox, "")]
	protected float m_fAnimationLength;

	private Widget m_Widget;
	private float m_wAnimationSpeed;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (m_sWidgetName != string.Empty)
			m_Widget = w.FindAnyWidget(m_sWidgetName);
		else
			m_Widget = w;
		
		if (m_fAnimationLength > 0)
			m_wAnimationSpeed = 1 / m_fAnimationLength;
		else
			m_wAnimationSpeed = 1000; // Instant animation
		
		if (m_Widget)
			m_Widget.SetColor(m_ColorDefault);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (!m_bSetClickColor)
			return false;

		Colorize(m_ColorClick);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (!m_bSetClickColor)
			return false;

		if (w == GetGame().GetWorkspace().GetFocusedWidget())
			Colorize(m_ColorFocus);
		else
			Colorize(m_ColorHover);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (!m_bSetFocusColor)
			return false;

		Colorize(m_ColorFocus);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (!m_bSetFocusColor)
			return false;

		if (w == WidgetManager.GetWidgetUnderCursor())
			Colorize(m_ColorHover);
		else
			Colorize(m_ColorDefault);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (!m_bSetHoverColor)
			return false;

		if (w != GetGame().GetWorkspace().GetFocusedWidget())
			Colorize(m_ColorHover);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_bSetHoverColor)
			return false;

		if (w != GetGame().GetWorkspace().GetFocusedWidget())
			Colorize(m_ColorDefault);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Colorize(Color color)
	{
		if (!m_Widget)
			return;

		if (m_bAnimateWidgets)
			AnimateWidget.Color(m_Widget, color, m_wAnimationSpeed);
		else
			m_Widget.SetColor(color);
	}
};