class SCR_AssignedPlayerLabelUIComponent : SCR_ScriptedWidgetComponent
{	
	protected ref SCR_AssignedPlayerLabelWidgets m_Widgets = new SCR_AssignedPlayerLabelWidgets();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerName(string name)
	{
		m_Widgets.m_wPlayerName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetPlatformIconWidget()
	{
		return m_Widgets.m_wPlatformKindIcon;
	}
}
