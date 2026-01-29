class SCR_AICharacterStats : AITaskScripted
{
	protected SCR_AIInfoComponent m_AIInfo;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
		if (!chimeraAgent)
			SCR_AgentMustChimera(this, owner);
		m_AIInfo = chimeraAgent.m_InfoComponent;
		if (!m_AIInfo)
			NodeError(this, owner, "Can't find AIInfo component.");
	}
};
