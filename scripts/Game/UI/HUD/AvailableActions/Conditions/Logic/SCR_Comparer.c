//------------------------------------------------------------------------------------------------
enum SCR_ComparerOperator
{
	EQUAL,
	LESS_THAN,
	LESS_THAN_OR_EQUAL,
	GREATER_THAN,
	GREATER_THAN_OR_EQUAL
};

//------------------------------------------------------------------------------------------------
//! Generic comparer using SCR_ComparerOperator
class SCR_Comparer<Class __ValueType>
{
	//------------------------------------------------------------------------------------------------
	static bool Compare(SCR_ComparerOperator op, __ValueType a, __ValueType b)
	{
		switch (op)
		{
			case SCR_ComparerOperator.EQUAL:
				return a == b;
			
			case SCR_ComparerOperator.LESS_THAN:
				return a < b;
			
			case SCR_ComparerOperator.LESS_THAN_OR_EQUAL:
				return a <= b;
			
			case SCR_ComparerOperator.GREATER_THAN:
				return a > b;
			
			case SCR_ComparerOperator.GREATER_THAN_OR_EQUAL:
				return a >= b;
		}
		
		return false;
	}
};
