class SCR_AIDecoOnWaypointChanged : DecoratorScripted
{
	static const string PORT_WAYPOINT_OUT = "OutWaypoint";
	
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_WAYPOINT_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	private AIWaypoint m_oCurrentWaypoint;
	
	protected override bool TestFunction(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return false;
		}
		
		AIWaypoint newWaypoint = group.GetCurrentWaypoint();
		bool result = true;
		
		if (!newWaypoint)
			result = false;
		else if (newWaypoint != m_oCurrentWaypoint)
			result = false;
		else
			result = true;
		
		m_oCurrentWaypoint = newWaypoint;
		
		if (m_oCurrentWaypoint)
			SetVariableOut(PORT_WAYPOINT_OUT,m_oCurrentWaypoint);
		else
			ClearVariable(PORT_WAYPOINT_OUT);	
					
		return result;		
	}
		
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected static override string GetOnHoverDescription()
	{
		return "SCR_AIDecoOnWaypointChanged: Returns false if current waypoint is changed or is null";
	}
};
	