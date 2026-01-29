class SCR_AISetBehaviorInterruptable : SCR_AIActionTask
{
	[Attribute("true", UIWidgets.CheckBox, "IsInterruptable")]
	private bool m_IsInterruptable;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.SetActionInterruptable(m_IsInterruptable);
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
		return "Sets behavior to be /not be interruptable by another behavior";
	}		

	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_IsInterruptable)
			return "INTERRUPTABLE";
		else
			return "NOT INTERRUPTABLE";		
	}
};