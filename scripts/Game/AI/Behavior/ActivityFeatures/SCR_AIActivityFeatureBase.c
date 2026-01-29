class SCR_AIActivityFeatureBase
{
	//-------------------------------------------------------------------------------------
	protected bool IsAgentAvailable(SCR_ChimeraAIAgent agent)
	{
		SCR_AIInfoComponent infoComp = agent.m_InfoComponent;
		
		return agent && infoComp.GetAIState() == EUnitAIState.AVAILABLE &&
			!infoComp.HasUnitState(EUnitState.IN_TURRET) && !infoComp.HasUnitState(EUnitState.IN_VEHICLE) &&
			!infoComp.HasUnitState(EUnitState.UNCONSCIOUS);
	}
}