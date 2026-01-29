class SCR_ToolMenuButtonComponent : SCR_ButtonImageComponent
{
	protected TextWidget m_wTextLimit;
	protected Widget m_wToolBackground;
	
	protected const string m_sTextLimitWidget = "TextLimit";
	protected const string m_sBackgroundWidget = "BackgroundNew";
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wTextLimit = TextWidget.Cast(w.FindAnyWidget(m_sTextLimitWidget));
		m_wToolBackground = w.FindAnyWidget(m_sBackgroundWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetText(string text)
	{
		m_wTextLimit.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetTextVisible(bool state)
	{
		m_wTextLimit.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets color of the button background.
	//! \param[in] color of the background.
	void SetBackgroundColor(notnull Color color)
	{
		m_wToolBackground.SetColor(color);
	}
}
