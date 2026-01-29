class SCR_AILookAtPosition: AITaskScripted
{
	[Attribute("1", UIWidgets.CheckBox, "Succeed when requested looking too close" )]
	protected bool m_bSuccessWhenClose;
	
	[Attribute("1.5", UIWidgets.EditBox, "Vertical offset to add to position" )]
	protected float m_fVerticalOffset;
	
	[Attribute("1.5", UIWidgets.EditBox, "DistSQ below look is ignored" )]
	protected float m_fIgnoreLookDistSq;

	[Attribute("20", UIWidgets.EditBox, "Priority of the look" )]
	protected float m_fPriority;

	protected static const string POSITION_PORT = "PositionIn";
	protected static const string POSITION_CORRECTION_PORT = "PositionOut";
	protected static const string PRIORITY_PORT = "PriorityIn";
	
	protected SCR_AIUtilityComponent m_Utility;	
	
	//-----------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//-----------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		POSITION_PORT,
		PRIORITY_PORT
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		POSITION_CORRECTION_PORT
	};
	override array<string> GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//-----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		vector lookPosition;
		float priority;
		
		if (!m_Utility)
		{
			m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (!m_Utility)
			{
				NodeError(this, owner, "This character does not have UtilityComponent!");
				return ENodeResult.FAIL;
			}
		}	
		
		if (!GetVariableIn(PRIORITY_PORT,priority))
			priority = m_fPriority;
		
		if (!GetVariableIn(POSITION_PORT,lookPosition))
			return ENodeResult.FAIL;
		
		if (lookPosition == vector.Zero)
		{
			NodeError(this,owner, "Provided zero vector!");
			return ENodeResult.FAIL;
		}

		if (!m_Utility.m_LookAction)
			return ENodeResult.FAIL;

		lookPosition[1] = lookPosition[1] + m_fVerticalOffset;
		
		if (vector.DistanceSq(lookPosition, m_Utility.GetOrigin()) < m_fIgnoreLookDistSq)
		{
			// distance is too close to look at
			ClearVariable(POSITION_CORRECTION_PORT);
			if (m_bSuccessWhenClose)
				return ENodeResult.SUCCESS;
			else 
				return ENodeResult.FAIL;
		}
		
		m_Utility.m_LookAction.LookAt(lookPosition, priority);
		SetVariableOut(POSITION_CORRECTION_PORT,lookPosition);
		return ENodeResult.SUCCESS;
    }
	
	//-----------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Look at position setter for execution inside tree LookAction.bt, the position may be corrected by terrain";
	}
};

