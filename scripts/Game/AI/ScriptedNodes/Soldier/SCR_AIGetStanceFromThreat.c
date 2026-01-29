class SCR_AIGetStanceFromThreat : AITaskScripted
{
	protected static const string STANCE_PORT = "Stance";
	
	protected SCR_AIUtilityComponent m_Utility;
	
	//------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
	}
	
	//------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Utility)
			return ENodeResult.FAIL;
		
		EAIThreatState threat = m_Utility.m_ThreatSystem.GetState();
		
		int stance = SCR_AIStanceHandling.GetStanceFromThreat(threat);
		SetVariableOut(STANCE_PORT, stance);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { STANCE_PORT };
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}