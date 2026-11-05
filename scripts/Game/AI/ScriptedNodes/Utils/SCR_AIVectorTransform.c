class SCR_AIVectorTransform: AITaskScripted
{
	static const string PORT_ENTITY			= "Entity";
	static const string PORT_OFFSET			= "Offset";
	static const string PORT_POSITION		= "PositionOut";
	
	[Attribute("0", UIWidgets.Auto, desc: "Apply XZ plane transform only?")]
	bool m_bXZplaneOnly;
	
	protected ref array<ref Shape> m_aDbgShape = {};
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity entity;
		vector offset, worldVector;
		
		if (!GetVariableIn(PORT_ENTITY, entity) || !GetVariableIn(PORT_OFFSET, offset) || !entity)
			return ENodeResult.FAIL;
		
		vector entityTransform[4];
		entity.GetWorldTransform(entityTransform);
		m_aDbgShape = {};
		
		if (m_bXZplaneOnly)
		{
			// Get the entity's forward and flatten it
			vector oldForward = entityTransform[2];
			vector oldRight = entityTransform[0];
			vector oldUp = entityTransform[1];
		
			// Flatten forward onto XZ plane
			vector flatForward = Vector(oldForward[0], 0, oldForward[2]);
		
			// If forward is too vertical, use right vector to find forward
			if (flatForward.LengthSq() < 0.001)
			{
				vector flatRight = Vector(oldRight[0], 0, oldRight[2]).Normalized();
				flatForward = (vector.Up * flatRight).Normalized();
			}
			else
			{
				flatForward = flatForward.Normalized();
			}
		
			// Instead of checking dotUp, check which ORIGINAL axis 
			// most closely aligns with world Up — that tells us the entity's state
			float absDotUpUp      = Math.AbsFloat(vector.Dot(oldUp, vector.Up));
			float absDotRightUp   = Math.AbsFloat(vector.Dot(oldRight, vector.Up));
			float absDotForwardUp = Math.AbsFloat(vector.Dot(oldForward, vector.Up));
		
			// Determine the dominant flip axis
			if (absDotUpUp >= absDotRightUp && absDotUpUp >= absDotForwardUp)
			{
				// entity is mostly upright or upsidedown — Up axis is most vertical
				if (vector.Dot(oldUp, vector.Up) < 0)
					flatForward = -flatForward; // on roof, flip
			}
			else if (absDotRightUp >= absDotUpUp && absDotRightUp >= absDotForwardUp)
			{
				// entity is on its side — Right axis is most vertical
				// Use the SIGN of right·up to determine which side
				// and the SIGN of forward projected to keep consistency
				float dotFwdFlat = vector.Dot(oldForward, flatForward);
				if (dotFwdFlat < 0)
					flatForward = -flatForward;
			}
			else
			{
				// entity is with its forward side up/down — Forward axis is most vertical
				float dotRightFlat = vector.Dot(oldRight, Vector(flatForward[2], 0, -flatForward[0]));
				if (dotRightFlat < 0)
					flatForward = -flatForward;
			}
			
			vector newRight = (flatForward * vector.Up).Normalized();
			worldVector = entity.GetOrigin() - newRight * offset[0] + vector.Up * offset[1] - flatForward * offset[2];
		}	
		else
		{
			worldVector = offset.Multiply4(entityTransform);
		}
		
		SetVariableOut(PORT_POSITION, worldVector);
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
		return "Returns position in world coordinates of given offset, that is transformed by the provided entity orientation or by only its yaw.";
	}
	
	protected static ref TStringArray s_aVarsIn = {PORT_ENTITY, PORT_OFFSET};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	protected static ref TStringArray s_aVarsOut = {PORT_POSITION};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};