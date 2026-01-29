class SCR_AIDecoIsWithinRadius : DecoratorScripted
{
	protected static ref TStringArray s_aVarsIn = {
		"WaypointIn"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
	
	protected override bool TestFunction(AIAgent owner)
	{
		AIWaypoint waypoint;
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return false;
		}
				
		if (!GetVariableIn("WaypointIn",waypoint))
			waypoint = group.GetCurrentWaypoint();		
		if (waypoint)
			return waypoint.IsWithinCompletionRadius(group);
		return false;
	}	

	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static override string GetOnHoverDescription()
	{
		return "Decorator that test that all AIAgents are within completion radius of waypoint, current waypoint is used if none is provided";
	}	
};