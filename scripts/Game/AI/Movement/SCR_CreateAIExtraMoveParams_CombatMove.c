// Special node which creates extra move parameters for SCR_AICombatMoveRequest_Move
class SCR_CreateAIExtraMoveParams_CombatMove : AITaskScripted
{
	// Inputs
	protected static const string PORT_REQUEST = "Request";
	protected static const string PORT_MOVING_TO_COVER = "InMovingToCover";
	
	// Outputs
	protected static const string PORT_PARAMS = "OutParams";
	
	protected ref AIExtraMoveParams m_Params;
	
	protected static const float STRAIGHT_PATH_COST_MODIFIER = 4.5; // Modifier when we want to find a flanking path
	
	//----------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(rqBase);
		
		if (!rq)
			return ENodeResult.FAIL;
		
		bool movingToCover;
		GetVariableIn(PORT_MOVING_TO_COVER, movingToCover);
		
		if (movingToCover || rq.m_vAvoidStraightPathDir == vector.Zero)
		{
			// No flanking requested, or we are moving to cover point
			// There is no need to use extra move parameters
			AIExtraMoveParams nullParams = null;
			SetVariableOut(PORT_PARAMS, nullParams); // Otherwise it complains about variable type mismatch
			return ENodeResult.SUCCESS;
		}
		
		// Initialize extra parameters
		if (!m_Params)
			m_Params = new AIExtraMoveParams();
		m_Params.Reset();
		
		m_Params.SetStraightPathCostModifier(STRAIGHT_PATH_COST_MODIFIER, rq.m_vAvoidStraightPathDir);
		
		SetVariableOut(PORT_PARAMS, m_Params);
		
		return ENodeResult.SUCCESS;
	};
	
	//----------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsOut = { PORT_PARAMS };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = { PORT_REQUEST, PORT_MOVING_TO_COVER };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}