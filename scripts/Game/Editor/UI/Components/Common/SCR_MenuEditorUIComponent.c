class SCR_MenuEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sAlwaysShownWidgetName;
	
	[Attribute()]
	protected string m_sHideableWidgetName;
	
	protected Widget m_AlwaysShownWidget;
	protected Widget m_HideableWidget;
	
	//------------------------------------------------------------------------------------------------
	//! \return Parent widget for layouts that should *not* be hidden when user hides editor interface.
	Widget GetAlwaysShownWidget()
	{
		return m_AlwaysShownWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Parent widget for layouts that should be hidden when user hides editor interface.
	Widget GetHideableWidget()
	{
		return m_HideableWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_AlwaysShownWidget = w.FindWidget(m_sAlwaysShownWidgetName);
		m_HideableWidget = w.FindWidget(m_sHideableWidgetName);
	}
}
