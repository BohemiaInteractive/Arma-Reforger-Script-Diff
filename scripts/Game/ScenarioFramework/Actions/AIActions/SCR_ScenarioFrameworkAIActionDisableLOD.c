[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionDisableLOD : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", desc: "If set, AI will not deactivate trough LODs.")]
	bool m_bDisableLOD;
	
	override void OnActivate()
	{
		super.OnActivate();
		
		if (!m_bDisableLOD)
			return;
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		m_AIGroup.PreventMaxLOD();
		
		foreach (AIAgent agent : agents)
		{
			agent.PreventMaxLOD();
		}
	}
}