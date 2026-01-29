class SCR_AIUpdateCombatMoveStateTimers : AITaskScripted
{
	protected static const string PORT_RESET = "Reset";
	
	protected SCR_AICombatMoveState m_State;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utilityComp)
			m_State = utilityComp.m_CombatMoveState;
	}
	
	//--------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_State)
			m_State.m_fTimerRequest_s = 0;
	}
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
			return ENodeResult.FAIL;
		
		int reset;
		GetVariableIn(PORT_RESET, reset);
		
		if (reset)
			m_State.m_fTimerRequest_s = 0;
		else
			m_State.m_fTimerRequest_s += dt;
		
		if (m_State.m_bInCover)
			m_State.m_fTimerInCover_s += dt;
		else
			m_State.m_fTimerInCover_s = 0;
		
		if (m_State.IsMoving())
			m_State.m_fTimerStopped_s = 0;
		else
			m_State.m_fTimerStopped_s += dt;
		
		
		return ENodeResult.SUCCESS;
	}	
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	protected ref TStringArray s_aVarsIn = { PORT_RESET };
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
}