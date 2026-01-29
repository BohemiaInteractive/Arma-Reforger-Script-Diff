[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionEnableAI : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", desc: "Enable AI")]
	bool m_bEnableAI;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		IEntity agentEntity;
		foreach (AIAgent agent : agents)
		{
			if (m_bEnableAI)
				agent.ActivateAI();
			else
				agent.DeactivateAI();
		}
	}
}