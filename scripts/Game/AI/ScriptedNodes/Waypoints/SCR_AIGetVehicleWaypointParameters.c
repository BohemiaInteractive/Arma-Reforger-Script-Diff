class SCR_AIGetVehicleWaypointParameters : SCR_AIGetWaypointParameters
{
	protected static ref TStringArray s_aVarsOut2 = SCR_AINodePortsHelpers.MergeTwoArrays(SCR_AIGetWaypointParameters.s_aVarsOut_Base, {PORT_VEHICLE, PORT_BOARDING_PARAMS});
		
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
		
		SCR_BoardingEntityWaypoint bEWp = SCR_BoardingEntityWaypoint.Cast(m_Waypoint);
		if (bEWp)
		{
			IEntity vehicle = bEWp.GetEntity();
			if (!vehicle)
				return NodeError(this, owner, "Waypoint needs entity reference!");
			SetVariableOut(PORT_VEHICLE, vehicle);
			SetVariableOut(PORT_BOARDING_PARAMS, bEWp.GetAllowance());
			return ENodeResult.SUCCESS;
		}
		
		SCR_BoardingWaypoint bWp = SCR_BoardingWaypoint.Cast(m_Waypoint);
		if (bWp)
		{
			SetVariableOut(PORT_BOARDING_PARAMS, bWp.GetAllowance());
			return ENodeResult.SUCCESS;
		}	
		return NodeError(this, owner, "Wrong class of provided Waypoint!");
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns vehicle and boarding parameters for GetIn and GetOut waypoints";
	}		

};