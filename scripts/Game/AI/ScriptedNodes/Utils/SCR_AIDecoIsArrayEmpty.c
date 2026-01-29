class SCR_AIDecoIsArrayEmpty : DecoratorScripted
{
	static const string VARIABLE_PORT		= "VariableIn";
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		bool isEmpty;
		Managed myManaged = null;
		string typeOfArray = GetVariableType(true, VARIABLE_PORT).ToString();
		
		if (!typeOfArray.Contains("array"))
			return false;
		if (!GetVariableIn(VARIABLE_PORT, myManaged))
			return false;	
		
		GetGame().GetScriptModule().Call(myManaged, "IsEmpty", false, isEmpty);		
		
		return isEmpty;		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		VARIABLE_PORT
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}	
};