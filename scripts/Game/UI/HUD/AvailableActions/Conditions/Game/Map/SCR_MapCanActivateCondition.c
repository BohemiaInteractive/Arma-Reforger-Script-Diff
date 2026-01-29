//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_MapCanActivateCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (data.m_bCanResetElementRotation)
			return GetReturnResult(false);
		
		if (data.m_bCanActivateMapElement)
			return GetReturnResult(true);

		return GetReturnResult(false);
	}
};