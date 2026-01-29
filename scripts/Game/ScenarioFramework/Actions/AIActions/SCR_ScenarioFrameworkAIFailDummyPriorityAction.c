[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIFailDummyPriorityAction : SCR_ScenarioFrameworkAIAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		if (!m_AIGroup)
			return;
		
		AIBaseUtilityComponent utility = AIBaseUtilityComponent.Cast(m_AIGroup.FindComponent(AIBaseUtilityComponent));
		if (!utility)
			return;
		
		SCR_AIScenarioFrameworkDummyPriorityAction oldAction = SCR_AIScenarioFrameworkDummyPriorityAction.Cast(utility.FindActionOfType(SCR_AIScenarioFrameworkDummyPriorityAction));
		if (oldAction)
			oldAction.Fail();
	}
}