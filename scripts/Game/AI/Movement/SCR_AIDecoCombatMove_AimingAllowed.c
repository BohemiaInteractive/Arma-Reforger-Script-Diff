class SCR_AIDecoCombatMove_AimingAllowed : DecoratorScripted
{
	protected SCR_AICombatMoveState m_State;
	protected SCR_AIUtilityComponent m_Utility;
	
	override bool TestFunction(AIAgent owner)	
	{
		if (!m_State)
		{
			m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (m_Utility)
				m_State = m_Utility.m_CombatMoveState;
			if (!m_State)
				return false;
		}
		
		SCR_AIBehaviorBase behavior = SCR_AIBehaviorBase.Cast(m_Utility.GetExecutedAction());
		
		// If behavior doesn't use Combat Movement tree, then Combat Movement restrictions don't matter
		if (behavior && !behavior.m_bUseCombatMove)
			return true;
		
		return m_State.m_bAimAtTarget;
	}
	
	static override string GetOnHoverDescription() { return "Returns m_bAimAtTarget from Combat Move State"; }
	static override bool VisibleInPalette() { return true; }
}