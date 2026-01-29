class SCR_AIResetCombatMoveState : AITaskScripted
{	
	protected SCR_AICombatMoveState m_State;
		
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
		{
			SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (utilityComp)
				m_State = utilityComp.m_CombatMoveState;
		}
		
		if (!m_State)
			return ENodeResult.FAIL;
		
		m_State.m_fTimerRequest_s = 0;
		m_State.m_fTimerInCover_s = 0;
		m_State.m_fTimerStopped_s = 0;
		
		m_State.CancelRequest();
		
		return ENodeResult.RUNNING;
	}	
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//--------------------------------------------------------------------------------------------
	static override bool CanReturnRunning() { return true; }
}