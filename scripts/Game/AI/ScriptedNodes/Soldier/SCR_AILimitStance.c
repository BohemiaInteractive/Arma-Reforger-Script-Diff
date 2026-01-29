class SCR_AILimitStance : AITaskScripted
{
	protected static const string STANCE_OUT_PORT = "StanceOut";
	protected static const string STANCE_IN_PORT = "StanceIn";
	
	protected SCR_AIUtilityComponent m_Utility;
	
	[Attribute(ECharacterStance.STAND.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterStance), desc: "Upper or lower limit")]
	protected ECharacterStance m_eStanceLimit0;
	
	[Attribute(ECharacterStance.STAND.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterStance), desc: "Upper or lower limit")]
	protected ECharacterStance m_eStanceLimit1;
	
	//------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		int stanceIn;
		GetVariableIn(STANCE_IN_PORT, stanceIn);
		
		int stanceOut;
		if (m_eStanceLimit0 < m_eStanceLimit1)
			stanceOut = Math.ClampInt(stanceIn, m_eStanceLimit0, m_eStanceLimit1);
		else
			stanceOut = Math.ClampInt(stanceIn, m_eStanceLimit1, m_eStanceLimit0);
		SetVariableOut(STANCE_OUT_PORT, stanceOut);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { STANCE_OUT_PORT };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = { STANCE_IN_PORT };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string s0 = string.Format("Stance Limit 0: %1\n", typename.EnumToString(ECharacterStance, m_eStanceLimit0));
		string s1 = string.Format("Stance Limit 1: %1\n", typename.EnumToString(ECharacterStance, m_eStanceLimit1));
		return s0 + s1;			
	}
	
	//------------------------------------------------------------------------------------
	static override string GetOnHoverDescription() { return "Limits stance value within a range"; };
}