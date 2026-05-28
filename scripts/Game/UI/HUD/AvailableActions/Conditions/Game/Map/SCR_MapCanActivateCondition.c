[BaseContainerProps()]
class SCR_MapCanActivateCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		if (data.m_bCanResetElementRotation)
			return GetReturnResult(false);

		return GetReturnResult(data.m_bCanActivateMapElement);
	}
}