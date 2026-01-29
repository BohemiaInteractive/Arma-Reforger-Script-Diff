class SCR_AISuspendAction : SCR_AIActionTask
{

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.SetActionIsSuspended(true);
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	

	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Suspends the action, calls SetSuspended(true).";
	}
};