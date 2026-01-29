class SCR_AIGetInNearestWaypointParameters : SCR_AIGetWaypointParameters
{
	protected static ref TStringArray s_aVarsOut2 = SCR_AINodePortsHelpers.MergeTwoArrays(SCR_AIGetWaypointParameters.s_aVarsOut_Base, {PORT_BOARDING_PARAMS, PORT_WAYPOINT_HOLDING_TIME});
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut2;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (ENodeResult.FAIL == super.EOnTaskSimulate(owner, dt))
			return ENodeResult.FAIL;
		
		SCR_BoardingTimedWaypoint wp = SCR_BoardingTimedWaypoint.Cast(m_Waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Wrong class of provided Waypoint!");
		}
		
		SetVariableOut(PORT_BOARDING_PARAMS, wp.GetAllowance());		
		SetVariableOut(PORT_WAYPOINT_HOLDING_TIME, wp.GetHoldingTime());		
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns get in nearest waypoint parameters";
	}
};