class SCR_ServerInfoUIComponent : ScriptedWidgetComponent
{
	protected SCR_ServerInfoEditorComponent m_ServerInfoManager;
	
	protected TextWidget m_Text;
	
	//------------------------------------------------------------------------------------------------
	protected void OnReceive(int fps, int memoryKB, int tickCount)
	{
		int memoryMB = memoryKB / 1024;
		string uptime = SCR_FormatHelper.GetTimeFormatting(tickCount * 0.001);
		
		m_Text.SetTextFormat("FPS: %1<br />Uptime: %2<br />Memory: %3 MB", fps, uptime, memoryMB);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Text = TextWidget.Cast(w);
		m_Text.SetText(string.Empty);
		
		m_ServerInfoManager = SCR_ServerInfoEditorComponent.Cast(SCR_ServerInfoEditorComponent.GetInstance(SCR_ServerInfoEditorComponent, true));
		if (m_ServerInfoManager)
			m_ServerInfoManager.GetOnReceive().Insert(OnReceive);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_ServerInfoManager)
			m_ServerInfoManager.GetOnReceive().Remove(OnReceive);
	}
}
