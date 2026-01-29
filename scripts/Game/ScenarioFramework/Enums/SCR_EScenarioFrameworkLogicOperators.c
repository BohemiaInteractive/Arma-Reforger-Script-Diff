enum SCR_EScenarioFrameworkLogicOperators
{
	AND,
	OR,
	NOT,
	XOR
}

class SCR_EScenarioFrameworkLogicOperatorHelper
{
	protected static ref ParamEnumArray s_ParamEnumArray = SCR_AttributesHelper.ParamFromDescriptions(SCR_EScenarioFrameworkLogicOperators, "All conditions must be true; At least one condition must be true; At least one condition must be false; Exactly one condition must be true;");
	
	//------------------------------------------------------------------------------------------------
	//! \return ParamEnumArray to describe the logic operators.
	static ParamEnumArray GetParamInfo()
	{
		return s_ParamEnumArray;
	}
}
