/*!
Component for handling server creation action.
Not for viusals.
*/

class SCR_HostButtonComponent : SCR_ScriptedWidgetComponent
{
	SCR_ModularButtonComponent m_Button;
	MissionWorkshopItem m_Scenario;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Button = SCR_ModularButtonComponent.FindComponent(w);
		
		// Invokers actions 
		m_Button.m_OnClicked.Insert(OnClicked);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClicked(SCR_ModularButtonComponent button)
	{
		// Open hosting menu
		ServerHostingUI dialog = SCR_CommonDialogs.CreateServerHostingDialog();
		
		if (m_Scenario && dialog)
			dialog.SelectScenario(m_Scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem scenario)
	{
		m_Scenario = scenario;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_HostButtonComponent FindComponent(Widget w)
	{
		return SCR_HostButtonComponent.Cast(w.FindHandler(SCR_HostButtonComponent));
	}
};