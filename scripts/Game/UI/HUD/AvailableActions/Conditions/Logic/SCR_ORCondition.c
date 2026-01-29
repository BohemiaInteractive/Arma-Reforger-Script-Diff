//------------------------------------------------------------------------------------------------
//! Checks all sub conditions and returns true if at least one condition returns true, false otherwise
[BaseContainerProps()]
class SCR_ORCondition : SCR_AvailableActionsGroupCondition
{		
	//------------------------------------------------------------------------------------------------
	//! Override and implement this method in any inherited conditions.
	//! Use GetReturnResult prior to returning the value to take m_bNegateCondition into account
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		bool isOk = false;
		foreach (auto subCondition : m_aConditions)
		{
			if (subCondition && subCondition.IsAvailable(data))
			{
				isOk = true;
				break;
			}
		}
		
		return GetReturnResult(isOk);
	}
};
