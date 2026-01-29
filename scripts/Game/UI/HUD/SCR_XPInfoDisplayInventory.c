class SCR_XPInfoDisplayInventory : SCR_ScriptedWidgetComponent
{
	protected SCR_XPInfoDisplay m_XPInfoDisplay;

	protected SCR_XPInfoDisplay GetXPInfoDisplay()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)		
			return null;
		
		return SCR_XPInfoDisplay.Cast(hudManager.FindInfoDisplay(SCR_XPInfoDisplay));
	}
	
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;
		
		super.HandlerAttached(w);	
		
		
		SCR_TaskManagerUIComponent taskManager = SCR_TaskManagerUIComponent.GetInstance();	
		m_XPInfoDisplay = GetXPInfoDisplay();
		if (!m_XPInfoDisplay || !taskManager)
		{
			w.SetVisible(false);
			return;
		}
		
		SCR_XPInfoWidgetData hudXPWidget = new SCR_XPInfoWidgetData();
		hudXPWidget.InitWidgets(m_wRoot);
		m_XPInfoDisplay.AddNewWidgets(hudXPWidget, SCR_EXPInfoWidget.INVENTORY);
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (m_XPInfoDisplay)
			m_XPInfoDisplay.RemoveWidgetsById(SCR_EXPInfoWidget.INVENTORY);
			
		super.HandlerDeattached(w);
	}
}