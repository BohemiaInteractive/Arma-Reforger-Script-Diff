class SCR_AIGetRandomPoint: AITaskScripted
{
	static const string WAYPOINT_PORT			= "Waypoint";
	static const string ORIGIN_PORT				= "Origin";
	static const string RADIUS_PORT				= "Radius";
	static const string EXCLUSION_RADIUS_PORT	= "ExclusionRadius";
	static const string PORT_RESULT_VECTOR		= "ResultVector";
	
	[Attribute("10", UIWidgets.EditBox, "Radius around origin")]
	protected float m_Radius;
	
	[Attribute("0", UIWidgets.EditBox, "Exclusion radius around origin")]
	protected float m_ExclusionRadius;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{			
		if (GetVariableType(false, PORT_RESULT_VECTOR) != vector)
		{
			NodeError(this, owner, PORT_RESULT_VECTOR + " has to be vector");
		}
		
		if (GetVariableIn(RADIUS_PORT, m_Radius) && (GetVariableType(true, RADIUS_PORT) != int && GetVariableType(true, RADIUS_PORT) != float))
		{
			NodeError(this, owner, RADIUS_PORT + " type should be number");
		}	
	}	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		vector origin = vector.Zero;
		IEntity waypointEnt;
				
		if (GetVariableIn(WAYPOINT_PORT,waypointEnt))
		{
			AIWaypoint waypoint = AIWaypoint.Cast(waypointEnt);
			if (!waypoint)
			{
				NodeError(this,owner, WAYPOINT_PORT + " is null! or wrong type");
				return ENodeResult.FAIL;
			};
			m_Radius = waypoint.GetCompletionRadius();
			origin = waypoint.GetOrigin();
		}	
		else 
		{
			if (GetVariableType(true, RADIUS_PORT) == int)
			{
				int radius;
				GetVariableIn(RADIUS_PORT, radius);
				m_Radius = radius;
			}
			else if (GetVariableType(true, RADIUS_PORT) == float)
			{
				GetVariableIn(RADIUS_PORT, m_Radius);
			}
			
			if (GetVariableType(true, ORIGIN_PORT) == vector)
			{
				GetVariableIn(ORIGIN_PORT, origin);
			}
		}	
		
		vector result = s_AIRandomGenerator.GenerateRandomPointInRadius(m_ExclusionRadius, m_Radius, origin, true);
		result[1] = origin[1]; 
		SetVariableOut(PORT_RESULT_VECTOR, result);

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static override protected string GetOnHoverDescription()
	{
		return "Returns random position in circle (2D) from given point or within radius of a waypoint. When origin vector isn't present it will assume zero vector instead.";
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT,
		ORIGIN_PORT,
		RADIUS_PORT
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_RESULT_VECTOR
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};