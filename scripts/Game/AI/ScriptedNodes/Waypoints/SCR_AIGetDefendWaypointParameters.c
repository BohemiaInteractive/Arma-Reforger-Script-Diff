class SCR_AIGetDefendWaypointParameters : SCR_AIGetWaypointParameters
{
	ref array<string> m_tagsArray = {};
	protected static ref TStringArray s_aVarsOut2 = SCR_AINodePortsHelpers.MergeTwoArrays(SCR_AIGetWaypointParameters.s_aVarsOut_Base, {PORT_USE_TURRETS, PORT_SEARCH_TAGS, PORT_FAST_INIT, PORT_WAYPOINT_HOLDING_TIME});	
	
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
		
		SCR_DefendWaypoint wp = SCR_DefendWaypoint.Cast(m_Waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Wrong class of provided Waypoint!");
		}
		
		SCR_DefendWaypointPreset defendPreset = wp.GetCurrentDefendPreset();
		if (!defendPreset)
			return SCR_AIErrorMessages.NodeErrorDefendPreset(this, owner);
		
		defendPreset.GetTagsForSearch(m_tagsArray);
		
		SetVariableOut(PORT_USE_TURRETS, defendPreset.GetUseTurrets());
		SetVariableOut(PORT_SEARCH_TAGS, m_tagsArray);
		SetVariableOut(PORT_FAST_INIT, wp.GetFastInit());
		SetVariableOut(PORT_WAYPOINT_HOLDING_TIME, wp.GetHoldingTime());
		
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
		return "Returns defend waypoint parameters";
	}
};