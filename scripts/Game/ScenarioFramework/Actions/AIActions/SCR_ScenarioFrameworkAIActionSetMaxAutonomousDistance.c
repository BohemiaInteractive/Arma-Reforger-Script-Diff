[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetMaxAutonomousDistance : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "700", uiwidget: UIWidgets.EditBox, desc: "Set max autonomous distance of AI group.", params: "0 1000 1", category: "Common")]
	int m_iInvestigationDistance;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(m_AIGroup.FindComponent(SCR_AIGroupUtilityComponent));
		if (!utility)
			return;
		
		utility.SetMaxAutonomousDistance(m_iInvestigationDistance);
	}
}