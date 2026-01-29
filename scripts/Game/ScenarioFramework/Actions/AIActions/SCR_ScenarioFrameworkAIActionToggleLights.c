[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionToggleLights : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", desc: "Set lights on")]
	bool m_bSetLightsOn;
	
	[Attribute(defvalue: "0", desc: "Set light lense")]
	int m_iLensID;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		IEntity agentEntity;
		foreach (AIAgent agent : agents)
		{
			agentEntity = agent.GetControlledEntity();
			if (agentEntity)
				ProcessEntity(agentEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessEntity(IEntity agentEntity)
	{
		SCR_FlashlightComponent flashlightComp;
		flashlightComp = SCR_FlashlightComponent.Cast(agentEntity.FindComponent(SCR_FlashlightComponent));
		if (flashlightComp)
		{
			flashlightComp.OnToggleActive(m_bSetLightsOn);
			
			for (int i = 0; i < m_iLensID; i++) {
        		flashlightComp.CycleThroughLenses(true);	
    		}
		}
		
		IEntity child = agentEntity.GetChildren();
		while (child)
		{
			ProcessEntity(child);
			child = child.GetSibling();
		}
	}
}