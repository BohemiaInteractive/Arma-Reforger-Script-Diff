[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnGroupAgentEntities : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Group getter")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Actions to be executed on group member entities.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!ValidateInputEntity(object, m_Getter, m_Entity))
			return;
		
		AIGroup group = AIGroup.Cast(m_Entity);
		if (!group)
			return;
		
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		IEntity controlledEntity;
		
		foreach (AIAgent agent : agents)
		{
			controlledEntity = agent.GetControlledEntity();
			if (!controlledEntity)
				continue;
			
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			{
				action.OnActivate(controlledEntity);
			}
		}
	}
}