class SCR_AIGetCoverIdleDuration : AITaskScripted
{
	protected static const string PORT_IDLE_DURATION = "IdleDuration";
	
	protected SCR_AIUtilityComponent m_Utility;
	
	protected static ref TStringArray s_aVarsOut = { PORT_IDLE_DURATION };
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	static override string GetOnHoverDescription()
	{
		return "Returns how many seconds the soldier should stay  in cover";
	}
	
	override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!m_Utility)
			NodeError(this, owner, "Utility component not found");
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		if (!m_Utility)
			return ENodeResult.FAIL;
		
		EAIThreatState threat = m_Utility.m_ThreatSystem.GetState();
		float duration = 0;
		switch (threat)
		{
			case EAIThreatState.SAFE:
			case EAIThreatState.VIGILANT:		duration = Math.RandomFloat(2.0, 4.0); break;
			case EAIThreatState.ALERTED:	duration = Math.RandomFloat(7.0, 11.0); break;
			case EAIThreatState.THREATENED:	duration = Math.RandomFloat(11.0, 16.0); break;
		}
		SetVariableOut(PORT_IDLE_DURATION, duration);
		return ENodeResult.SUCCESS;
	}
};
