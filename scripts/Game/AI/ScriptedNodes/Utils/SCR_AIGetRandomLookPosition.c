class SCR_AIGetRandomLookPosition : AITaskScripted
{
	// Inputs
	protected static const string PORT_IN_POS = "InPos";
	
	// Outputs
	protected static const string PORT_OUT_POS = "OutPos";
	
	[Attribute("0.5", UIWidgets.EditBox, params: "0 1", desc: "Radius of random position is calculated based on distance multiplied by this value")]
	protected float m_fTangent;
	
	//----------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
			return ENodeResult.FAIL;
		
		vector inPos;
		if (!GetVariableIn(PORT_IN_POS, inPos))
			return ENodeResult.FAIL;
		
		float distance = vector.Distance(controlledEntity.GetOrigin(), inPos);
		
		vector outPos = s_AIRandomGenerator.GenerateRandomPointInRadius(0, distance*m_fTangent, inPos, true);
		outPos[1] = inPos[1]; 
		
		SetVariableOut(PORT_OUT_POS, outPos);
		
		return ENodeResult.SUCCESS;
	}
	
	//----------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_OUT_POS };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = { PORT_IN_POS };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	override static string GetOnHoverDescription()
	{
		return "Generates randomized positions based on estimated target position, to look at them randomly.";
	}
	
	override string GetNodeMiddleText()
	{
		return string.Format("Tangent: %1", m_fTangent);
	}
	
	override static bool VisibleInPalette() { return true; }
}