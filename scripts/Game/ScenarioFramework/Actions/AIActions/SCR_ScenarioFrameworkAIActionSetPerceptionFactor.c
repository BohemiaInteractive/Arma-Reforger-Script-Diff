[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetPerceptionFactor : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Sets perception ability. Affects speed at which perception detects targets. Bigger value means proportionally faster detection.", params: "0 100 0.001", category: "Common")]
	float m_fPerceptionFactor;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetPerceptionFactor(m_fPerceptionFactor);
		}
	}
}