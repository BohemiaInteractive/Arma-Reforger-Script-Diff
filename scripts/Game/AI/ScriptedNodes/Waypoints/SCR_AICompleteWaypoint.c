class SCR_AICompleteWaypoint : AITaskScripted
{
	protected static ref TStringArray s_aVarsIn = {
		"WaypointIn"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		AIWaypoint wp;
		GetVariableIn("WaypointIn",wp);
		if ( !wp )
			wp = group.GetCurrentWaypoint();
		if ( wp )
		{
			group.CompleteWaypoint(wp);
			return ENodeResult.SUCCESS;
		}				
		
		return ENodeResult.FAIL;
	}

	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected static override string GetOnHoverDescription()
	{
		return "Completes current waypoint";
	}		

};