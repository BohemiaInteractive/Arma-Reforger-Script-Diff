class SCR_AIGetEntityWaypointParameters : SCR_AIGetWaypointParameters
{
	protected static ref TStringArray s_aVarsOut2 = SCR_AINodePortsHelpers.MergeTwoArrays(SCR_AIGetWaypointParameters.s_aVarsOut_Base, {PORT_ENTITY});
	
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
		
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(m_Waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Wrong class of provided Waypoint!");
		}
		
		SetVariableOut(PORT_ENTITY, wp.GetEntity());		
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns entity waypoint Entity parameter";
	}
};