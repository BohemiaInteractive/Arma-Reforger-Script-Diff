class SCR_AIDecoIsEnemyWithinRadius : DecoratorScripted
{
	static const string WAYPOINT_PORT = "WaypointIn";
	static const string LOCATION_PORT = "LocationOut";
	
	//---------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		LOCATION_PORT
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//---------------------------------------------------------------------------------------------------
	// Checks if at least one enemy known to group is within the completion radius
	bool TestLocationOfKnownEnemies(SCR_AIGroupUtilityComponent groupUtility, AIWaypoint waypoint)
	{
		float waypointRadiusSq = waypoint.GetCompletionRadius() * waypoint.GetCompletionRadius();
		
		vector wpOrigin = waypoint.GetOrigin();
		
		foreach (SCR_AITargetInfo tgtInfo : groupUtility.m_Perception.m_aTargets)
		{
			if (tgtInfo.m_eCategory == EAITargetInfoCategory.DESTROYED)
				continue;
			
			if (vector.DistanceSq(tgtInfo.m_vWorldPos, wpOrigin) < waypointRadiusSq)
			{
				SetVariableOut(LOCATION_PORT, tgtInfo.m_vWorldPos);
				return true;
			}
		}
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------	
	protected override bool TestFunction(AIAgent owner)
	{
		AIWaypoint waypoint;
		SCR_AIGroupUtilityComponent guc;
		
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
			return false;
		
		if ( !GetVariableIn(WAYPOINT_PORT,waypoint) )
			waypoint = group.GetCurrentWaypoint();
		if (! waypoint )
		{
			Print("Node IsEnemyWithinRadius executed without valid waypoint!");
			return false;
		};
		guc = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));
		if ( !guc )	
		{
			Print("Node IsEnemyWithinRadius executed on owner without group utility component!");
			return false;
		}
		return TestLocationOfKnownEnemies(guc, waypoint);
	}	
	
	//---------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static override string GetOnHoverDescription()
	{
		return "Decorator that test that at least one detected enemy is within completion radius of waypoint, current waypoint is used if none is provided";
	}
};