class SCR_AIGetWaypointParameters : AITaskScripted
{
	static const string PORT_WAYPOINT_IN			= "WaypointIn";
	static const string PORT_WAYPOINT_OUT			= "WaypointOut";
	static const string PORT_RADIUS 				= "Radius";
	static const string PORT_ORIGIN 				= "Origin";
	static const string PORT_PRIORITY_LEVEL 		= "PriorityLevel";
	static const string PORT_BOARDING_PARAMS 		= "BoardingParams";	
	static const string PORT_WAYPOINT_HOLDING_TIME 	= "HoldingTime";
	static const string PORT_USE_TURRETS			= "UseTurrets";
	static const string PORT_SEARCH_TAGS 			= "SearchTags";
	static const string PORT_FAST_INIT 				= "FastInit";
	static const string PORT_VEHICLE 				= "Vehicle";
	static const string PORT_ENTITY 				= "Entity";
	
	protected SCR_AIWaypoint m_Waypoint;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity wpEntity;
		if (!GetVariableIn(PORT_WAYPOINT_IN, wpEntity))
		{
			AIGroup group = AIGroup.Cast(owner);
			if (!group)
			{
				SCR_AgentMustBeAIGroup(this, owner);
				return ENodeResult.FAIL;
			}
			m_Waypoint = SCR_AIWaypoint.Cast(group.GetCurrentWaypoint());
		}	
		else 
			m_Waypoint = SCR_AIWaypoint.Cast(wpEntity);
		
		if (!m_Waypoint)
			// waypoint is deleted and we didnt react on it, yet
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_WAYPOINT_OUT, m_Waypoint);
		SetVariableOut(PORT_RADIUS, m_Waypoint.GetCompletionRadius());
		SetVariableOut(PORT_ORIGIN, m_Waypoint.GetOrigin());
		SetVariableOut(PORT_PRIORITY_LEVEL, m_Waypoint.GetPriorityLevel());
			
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_WAYPOINT_IN
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	protected static ref TStringArray s_aVarsOut_Base = {
		PORT_WAYPOINT_OUT,
		PORT_RADIUS,
		PORT_ORIGIN,
		PORT_PRIORITY_LEVEL
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut_Base;
	}
};