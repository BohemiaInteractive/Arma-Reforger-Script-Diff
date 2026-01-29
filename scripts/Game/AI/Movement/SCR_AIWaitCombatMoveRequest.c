class SCR_AIWaitCombatMoveRequest : AITaskScripted
{
	protected static const string PORT_REQUEST = "Request";
	
	protected SCR_AIUtilityComponent m_UtilityComp;
	
	protected ref SCR_AICombatMoveRequestBase m_CurrentRequest; // Ref ptr to current request we are processing
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_UtilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
	}
	
	//------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComp)
			return ENodeResult.FAIL;

		SCR_AICombatMoveRequestBase rq = m_UtilityComp.m_CombatMoveState.GetRequest();
		if (!rq || rq == m_CurrentRequest)
			return ENodeResult.RUNNING;
		
		m_CurrentRequest = rq;
		
		// Expose request to port
		SetVariableOut(PORT_REQUEST, m_CurrentRequest);
		
		return ENodeResult.SUCCESS;
	}
	
	
	
	//------------------------------------------------------------------------
	static ref TStringArray s_aVarsOut = 
	{
		PORT_REQUEST
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool CanReturnRunning() { return true; }
	
	static override bool VisibleInPalette() { return true; }
}