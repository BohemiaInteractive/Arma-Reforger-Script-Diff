[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetPlayerGroup : SCR_ScenarioFrameworkGet
{
	[Attribute(defvalue: "1", params: "0 inf")]
	int m_iPlayerId;
	
	[Attribute(desc: "Get Slave Group in case, that interacting with subordinate AI units is needed.")]
	bool m_bGetSlaveGroup;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_PlayerControllerGroupComponent pControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(m_iPlayerId);
		if (!pControllerGroupComponent)
		{
			PrintFormat("ScenarioFramework Action: SCR_PlayerControllerGroupComponent not found for Action %1.", this, level: LogLevel.ERROR);
			return null;
		}
		
		if (!m_bGetSlaveGroup)
			return new SCR_ScenarioFrameworkParam<IEntity>(pControllerGroupComponent.GetPlayersGroup());
		
		SCR_AIGroup group = pControllerGroupComponent.GetPlayersGroup();
		if (!group)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(group.GetSlave());
	}

}