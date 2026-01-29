//------------------------------------------------------------------------------------------------
class DecoratorScripted_IsEqual : DecoratorScripted
{
	static const string PORT_VALUE1 = "value1";
	static const string PORT_VALUE2 = "value2";
	
	
	protected override bool TestFunction(AIAgent owner)
	{
		if (GetVariableType(true, PORT_VALUE1) == GetVariableType(true, PORT_VALUE2))
		{
			if (GetVariableType(true, PORT_VALUE1) == int)
			{
				int value1 = 0;	
				int value2 = 0;	
				GetVariableIn(PORT_VALUE1, value1);
				GetVariableIn(PORT_VALUE2, value2);
				
				return value1 == value2;
			}
			else if (GetVariableType(true, PORT_VALUE1) == float)
			{
				float val1 = 0;	
				float val2 = 0;	
				GetVariableIn(PORT_VALUE1, val1);
				GetVariableIn(PORT_VALUE2, val2);
				
				return float.AlmostEqual(val1, val2);
			}
			else if (GetVariableType(true, PORT_VALUE1) == bool)
			{
				bool val1; 
				bool val2; 
				GetVariableIn(PORT_VALUE1, val1);
				GetVariableIn(PORT_VALUE2, val2);
				
				return val1 == val2;
			}
			else if (GetVariableType(true, PORT_VALUE1).IsInherited(IEntity))
			{
				IEntity val1 = null; 
				IEntity val2 = null; 
				GetVariableIn(PORT_VALUE1, val1);
				GetVariableIn(PORT_VALUE2, val2);
				
				return val1 == val2;
			}
			else if (GetVariableType(true, PORT_VALUE1).IsInherited(Managed))
			{
				Managed val1 = null; 
				Managed val2 = null; 
				GetVariableIn(PORT_VALUE1, val1);
				GetVariableIn(PORT_VALUE2, val2);
				
				return val1 == val2;
			}
		}
		
		return false;
	}
	
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected static override string GetOnHoverDescription()
	{
		return "DecoratorScripted_IsEqual: Compares whether the given variables are equal. Supports int-int, float-float, bool - bool, IEntity - IEntity and children of Managed comparison";
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_VALUE1,
		PORT_VALUE2
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
};
