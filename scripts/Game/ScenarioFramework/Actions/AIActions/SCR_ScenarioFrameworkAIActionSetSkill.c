[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetSkill : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EAISkill.REGULAR.ToString(), UIWidgets.ComboBox, "AI skill in combat", "", ParamEnumArray.FromEnum(EAISkill), category: "Common")]
	EAISkill m_eAISkill;
	
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
				combatComponent.SetAISkill(m_eAISkill);
		}
	}
}