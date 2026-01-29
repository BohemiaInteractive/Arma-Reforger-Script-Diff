class SCR_AIDecoCombatMove_Moving : DecoratorScripted
{
	protected SCR_AICombatMoveState m_State;
	
	override bool TestFunction(AIAgent owner)	
	{
		if (!m_State)
		{
			SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (utilityComp)
				m_State = utilityComp.m_CombatMoveState;
			if (!m_State)
				return false;
		}
		
		return m_State.IsMoving();
	}
	
	static override string GetOnHoverDescription() { return "Returns IsMoving condition from Combat Move State"; }
	static override bool VisibleInPalette() { return true; }
}