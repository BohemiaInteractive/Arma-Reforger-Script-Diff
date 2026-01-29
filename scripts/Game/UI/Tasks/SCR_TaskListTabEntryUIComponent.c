class SCR_TaskListTabEntryUIComponent : SCR_ScriptedWidgetComponent
{	
	protected SCR_ETaskTabType m_eTabType;
	protected ref SCR_TaskListTabEntryWidgets m_Widgets = new SCR_TaskListTabEntryWidgets();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);
	}
		
	//------------------------------------------------------------------------------------------------
	void Initialize(SCR_ETaskTabType tab)
	{
		m_eTabType = tab;
		
		switch (m_eTabType)
		{
			case SCR_ETaskTabType.AVAILABLE:
			{
				m_Widgets.m_wText.SetText("#AR-Tasks_TaskTab_Available");
				break;
			}
			case SCR_ETaskTabType.FINISHED:
			{
				m_Widgets.m_wText.SetText("#AR-Tasks_TaskTab_Completed");
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeTabColor(Color color)
	{
		m_Widgets.m_wBackground.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ETaskTabType GetTabType()
	{
		return m_eTabType;
	}
}
