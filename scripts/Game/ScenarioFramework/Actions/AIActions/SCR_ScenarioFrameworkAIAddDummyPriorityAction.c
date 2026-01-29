[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAddDummyPriorityAction : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "0", desc: "Priority", params: "0 2000 0.5")]
	float m_fPriority;
	
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
		
		AIActionBase action = new SCR_AIScenarioFrameworkDummyPriorityAction();
		action.SetPriority(m_fPriority);
		utility.AddAction(action);
	}
}