class SCR_AIGetPosForward: AITaskScripted
{
	static const string PORT_DISTANCE = "DistanceIn";
	static const string PORT_RESULT_VECTOR = "VectorOut";	
	static const string PORT_TARGET = "TargetIn";
	
	[Attribute("10", UIWidgets.EditBox, PORT_DISTANCE)]
	float m_Distance;
	
	protected vector m_ResultVector;
	protected IEntity m_Target;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (GetVariableIn(PORT_DISTANCE, m_Distance) && (GetVariableType(true, PORT_DISTANCE) != int && GetVariableType(true, PORT_DISTANCE) != float) )
		{
			NodeError(this, owner, PORT_DISTANCE + " type should be number");
		}
	}

	//------------------------------------------------------------------------------------------------	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{			
		if (!GetVariableIn(PORT_TARGET,m_Target))
			m_Target = owner;
		
		AIAgent agent = AIAgent.Cast(m_Target);
		if (agent)
			m_Target = agent.GetControlledEntity();
		
		if (!m_Target)
			return ENodeResult.FAIL;
		
		vector direction = m_Target.GetYawPitchRoll().AnglesToVector();
		
		if (GetVariableType(true, PORT_DISTANCE) == int)
		{
			int distance;
			GetVariableIn(PORT_DISTANCE, distance);
			m_Distance = distance;
		}
		else if (GetVariableType(true, PORT_DISTANCE) == float)
		{
			GetVariableIn(PORT_DISTANCE, m_Distance);
		}
		
		vector pos = m_Target.GetOrigin();
				
		vector forwardVector = pos + ( direction * m_Distance );
		// snap the vector on ground, for valid location to run to
		forwardVector[1] = m_Target.GetWorld().GetSurfaceY(forwardVector[0], forwardVector[2]);
		
		m_ResultVector = forwardVector;
		
		SetVariableOut(PORT_RESULT_VECTOR, m_ResultVector);		

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------	
	protected static override string GetOnHoverDescription()
	{
		return "It will return world position of forward vector of provided target entity multiplied by distance. If no target specified, owner will be used.";
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_TARGET,
		PORT_DISTANCE
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