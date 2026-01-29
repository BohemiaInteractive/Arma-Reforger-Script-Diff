class SCR_AIFailAction : SCR_AIActionTask
{
	[Attribute(typename.EnumToString(EAIActionFailReason, EAIActionFailReason.UNKNOWN), UIWidgets.ComboBox, desc: "Fail reason which will be set by SetFailReason()", enums: ParamEnumArray.FromEnum(EAIActionFailReason))]
	protected EAIActionFailReason m_eFailReason;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.SetFailReason(m_eFailReason);
		action.Fail();
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
		return "Fails action specified in input or current action";
	}		

};