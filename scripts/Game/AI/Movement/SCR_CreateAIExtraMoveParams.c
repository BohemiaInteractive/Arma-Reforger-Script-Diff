class SCR_CreateAIExtraMoveParams : AITaskScripted
{
	// Inputs
	protected static const string PORT_FLANKING_START_POS = "InFlankStartPos";
	protected static const string PORT_FLANKING_END_POS = "InFlankEndPos";
	
	// Outputs
	protected static const string PORT_PARAMS = "OutParams";
	
	protected ref AIExtraMoveParams m_Params;
	
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fStraightPathCostModifier;
	
	//----------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Params)
			m_Params = new AIExtraMoveParams();
		
		m_Params.Reset();
		
		InitStraightPathCostModifier();
		
		SetVariableOut(PORT_PARAMS, m_Params);
		
		return ENodeResult.SUCCESS;
	};
	
	//----------------------------------------------------------------------------------------
	void InitStraightPathCostModifier()
	{		
		vector flankStartPos, flankEndPos;
		GetVariableIn(PORT_FLANKING_START_POS, flankStartPos);
		GetVariableIn(PORT_FLANKING_END_POS, flankEndPos);
		
		vector vDir = flankEndPos - flankStartPos;
		
		if (vDir != vector.Zero)
			vDir.Normalize();
		
		// It will verify inside that parameters make sense
		m_Params.SetStraightPathCostModifier(m_fStraightPathCostModifier, vDir);
	}
	
	//----------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsOut = { PORT_PARAMS };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = { PORT_FLANKING_START_POS, PORT_FLANKING_END_POS };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override string GetOnHoverDescription() { return "Creates AIExtraMoveParams object which can be passed to AITaskMove"; }
}