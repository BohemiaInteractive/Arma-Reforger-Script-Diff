class SCR_AICompleteAction : SCR_AIActionTask
{

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.Complete();
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
		return "Completes action specified in input or current action";
	}		

};