//------------------------------------------------------------------------------------------------
//! Checks all sub conditions and returns true if all conditions return true, false otherwise
[BaseContainerProps()]
class SCR_ANDCondition : SCR_AvailableActionsGroupCondition
{		
	//------------------------------------------------------------------------------------------------
	//! Override and implement this method in any inherited conditions.
	//! Use GetReturnResult prior to returning the value to take m_bNegateCondition into account
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		bool isOk = true;
		foreach (auto subCondition : m_aConditions)
		{
			if (subCondition && !subCondition.IsAvailable(data))
			{
				isOk = false;
				break;
			}
		}
		
		return GetReturnResult(isOk);
	}
};
