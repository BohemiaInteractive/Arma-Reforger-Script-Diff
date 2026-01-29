class SCR_AIGetCombatMoveState : AITaskScripted
{
	// Inputs
	protected static const string PORT_IN_COVER = "InCover";
	protected static const string PORT_COVER_LOCK = "CoverLock";
	
	protected SCR_AICombatMoveState m_State;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utilityComp)
			m_State = utilityComp.m_CombatMoveState;
	}
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_IN_COVER, m_State.m_bInCover);
		SetVariableOut(PORT_COVER_LOCK, m_State.GetAssignedCover());
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_IN_COVER,
		PORT_COVER_LOCK
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
}