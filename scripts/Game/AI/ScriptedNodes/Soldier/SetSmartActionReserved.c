class SCR_AISetSmartActionReserved : AITaskScripted
{
	static const string SMART_ACTION_PORT 	= "SmartActionIn";
	static const string OWNER_PORT			= "OwnerAgentIn";
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AISmartActionComponent smartActionComponent;
		AIAgent ownerAgent;
		
		if (!GetVariableIn(SMART_ACTION_PORT, smartActionComponent))
			return NodeError(this, owner, "No smartaction to reserve!");
		if (!GetVariableIn(OWNER_PORT, ownerAgent))
			ownerAgent = owner;
		
		smartActionComponent.ReserveAction(ownerAgent);
		return ENodeResult.SUCCESS;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		OWNER_PORT,
		SMART_ACTION_PORT
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "SetSmartActionInaccessible: makes the smart action reserved for other AIs, it is released when performActionBehavior completes/ fails.";
	};	
};