class SCR_PlayersListEntryEditorUIComponent : SCR_EditableEntityLinkUIComponent
{
	[Attribute()]
	protected string m_sNameWidgetName;
	
	[Attribute()]
	protected string m_sDeadWidgetName;
	
	protected TextWidget m_NameWidget;
	protected Widget m_DeadWidget;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name
	void SetName(string name)
	{
		if (m_NameWidget && !name.IsEmpty())
			m_NameWidget.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] dead
	void SetDead(bool dead)
	{
		if (m_DeadWidget)
			m_DeadWidget.SetOpacity(dead);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_NameWidget = TextWidget.Cast(w.FindAnyWidget(m_sNameWidgetName));
		m_DeadWidget = w.FindAnyWidget(m_sDeadWidgetName);
	}
}
