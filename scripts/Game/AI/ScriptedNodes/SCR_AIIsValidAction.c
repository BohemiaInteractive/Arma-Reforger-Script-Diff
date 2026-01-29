class SCR_AIIsValidAction : DecoratorScripted
{
	private SCR_AIBaseUtilityComponent m_Utility;
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_Utility)
		{
			m_Utility = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
			if (!m_Utility)
				return false;
		}
		
		SCR_AIActionBase executedAction = SCR_AIActionBase.Cast(m_Utility.GetExecutedAction());
		
		if (!executedAction)
			return false;
		
		// There seems to be a bug that RunBT node keeps running previous tree if we provide an empty string
		if (executedAction.m_sBehaviorTree == string.Empty)
			return false;

		EAIActionState state = executedAction.GetActionState();
		return state != EAIActionState.COMPLETED && state != EAIActionState.FAILED;
	}

	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static override string GetOnHoverDescription()
	{
		return "Test if current action is still valid, or it was already completed or failed";
	}
};