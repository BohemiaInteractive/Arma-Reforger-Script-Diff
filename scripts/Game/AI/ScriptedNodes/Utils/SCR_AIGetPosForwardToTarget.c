class SCR_AIGetPosForwardToTarget: AITaskScripted
{
	static const string PORT_DISTANCE = "Distance";
	static const string PORT_RESULT_VECTOR = "ResultVector";
	static const string PORT_TARGET = "Target";
	
	[Attribute("10", UIWidgets.EditBox, PORT_DISTANCE)]
	float m_Distance;
	
	protected vector m_ResultVector;
	protected IEntity m_Target;
	protected vector m_vPosition;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (GetVariableType(false, PORT_RESULT_VECTOR) != vector)
		{
			NodeError(this, owner, PORT_RESULT_VECTOR + " should be vector");
		}
		
		if (GetVariableIn(PORT_DISTANCE, m_Distance) && (GetVariableType(true, PORT_DISTANCE) != int && GetVariableType(true, PORT_DISTANCE) != float) )
		{
			NodeError(this, owner, PORT_DISTANCE + " type should be number");
		}	
		
		if (GetVariableType(true, PORT_TARGET) != vector && GetVariableType(true, PORT_TARGET) != IEntity)
		{
			NodeError(this, owner, PORT_TARGET + " should be IEntity or vector");
		}		
	}

	//------------------------------------------------------------------------------------------------	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{			
		IEntity controlledEntity = owner.GetControlledEntity();
		if (GetVariableType(true, PORT_TARGET) == vector)
		{
			GetVariableIn(PORT_TARGET,m_vPosition);
		}
		else if (GetVariableType(true, PORT_TARGET) == IEntity)
		{
			GetVariableIn(PORT_TARGET,m_Target);
			if (!m_Target)
				return ENodeResult.FAIL;
			
			if (m_Target == owner || m_Target == controlledEntity)
				return NodeError(this, owner, PORT_TARGET + " can't be the same as owner entity");
		}
		
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
		
		
		vector ownerPos = controlledEntity.GetOrigin();
		
		vector pos = m_vPosition;
		if (m_Target)
			pos = m_Target.GetOrigin();
				
		vector direction = vector.Direction(ownerPos, pos).Normalized();
		vector forwardVector = ownerPos + ( direction * m_Distance );
		// snap the vector on ground, for valid location to run to
		forwardVector[1] = controlledEntity.GetWorld().GetSurfaceY(forwardVector[0], forwardVector[2]);
		
		m_ResultVector = forwardVector;
		
		SetVariableOut(PORT_RESULT_VECTOR, m_ResultVector);

		//DrawDebug();

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------	
	protected static override string GetOnHoverDescription()
	{
		return "It will return world position in direction to target away from owner position. Doesn't take navmesh in account. Target can be entity or vector.";
	}

	//------------------------------------------------------------------------------------------------		
	void DrawDebug()
	{
		int color = ARGB(255, 255, 64, 64);

		Shape dbgShape = Shape.CreateSphere(color, ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, vector.Zero, 1.0);
			
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = m_ResultVector;
		dbgShape.SetMatrix(mat);
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