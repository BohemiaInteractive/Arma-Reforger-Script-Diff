class SCR_AICalculateNextCombatMovePos : AITaskScripted
{
	// Inputs
	protected static const string PORT_REQUEST = "Request";
  	protected static const string PORT_TARGET_POS = "TargetPos";
	
	// Outputs
	protected static const string PORT_POS = "Pos";
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{		
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity)
			return ENodeResult.FAIL;
		
		// Read inputs
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rq);
		
		// Case for CUSTOM_POS is trivial, we move directly there
		if (rq.m_eDirection == SCR_EAICombatMoveDirection.CUSTOM_POS)
		{
			SetVariableOut(PORT_POS, rq.m_vMovePos);
			return ENodeResult.SUCCESS;
		}
		
		// The remaining cases are for moving a given distance in given direction
		
		vector ownerPos = myEntity.GetOrigin();
		
		vector moveDir = SCR_AICombatMoveUtils.CalculateMoveDirection(rq.m_eDirection, ownerPos, rq.m_vMovePos); // It can't return 000!
		
		// Estimated move speed, depends on what we are controlling
		float estMoveSpeed = SCR_AICombatMoveUtils.GetEstimatedMoveSpeed(rq);
		float moveDistance = rq.m_fMoveDuration_s * estMoveSpeed;
		
		// Randomize destination pos
		vector newPositionCenter = ownerPos + moveDir * moveDistance;
		
		vector randomizedPos = RandomizeDestinationPos(moveDistance, newPositionCenter);
		
		SetVariableOut(PORT_POS, randomizedPos);
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	protected vector RandomizeDestinationPos(float distance, vector centerPos)
	{
		float radius = 0.1 * distance;
		vector pos = s_AIRandomGenerator.GenerateRandomPointInRadius(0, radius, centerPos, true);
		pos[1] = centerPos[1];
		return pos;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POS
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
	
}