class SCR_AIGetTimedWaypointParameters : SCR_AIGetWaypointParameters
{
	
	protected static ref TStringArray s_aVarsOut2 = SCR_AINodePortsHelpers.MergeTwoArrays(SCR_AIGetWaypointParameters.s_aVarsOut_Base, {PORT_WAYPOINT_HOLDING_TIME});
	
	//----------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut2;
    }
	
	//----------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (ENodeResult.FAIL == super.EOnTaskSimulate(owner, dt))
			return ENodeResult.FAIL;
		
		SCR_TimedWaypoint wp = SCR_TimedWaypoint.Cast(m_Waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Wrong class of provided Waypoint!");
		}
		
		SetVariableOut(PORT_WAYPOINT_HOLDING_TIME, wp.GetHoldingTime());
		return ENodeResult.SUCCESS;
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//----------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Gets holding time set on current timed waypoint";
	}		

};