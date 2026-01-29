class SCR_ScenarioRestartComponent : SCR_ScriptedWidgetComponent
{
	//----------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		SCR_ButtonBaseComponent modularButtonComponent = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (!modularButtonComponent)
			return;
		
		modularButtonComponent.m_OnClicked.Insert(RestartScenario);
	}
	
	//----------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_ButtonBaseComponent modularButtonComponent = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (!modularButtonComponent)
			return;
		
		modularButtonComponent.m_OnClicked.Remove(RestartScenario);
		
		super.HandlerDeattached(w);
	}
	
	//----------------------------------------------------------------------------------
	protected void RestartScenario()
	{
		if (!m_wRoot)
			return;
		
		SCR_ButtonBaseComponent modularButtonComponent = SCR_ButtonBaseComponent.Cast(m_wRoot.FindHandler(SCR_ButtonBaseComponent));
		if (!modularButtonComponent)
			return;
		
		modularButtonComponent.m_OnClicked.Remove(RestartScenario);

		auto manager = GetGame().GetSaveGameManager();
		manager.StartPlaythrough(manager.GetCurrentMissionResource());
	}
}
