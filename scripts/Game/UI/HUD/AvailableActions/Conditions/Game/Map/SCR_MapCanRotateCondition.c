//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_MapCanRotateCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (data.m_bCanRotateMapElement)
			return GetReturnResult(true);

		return GetReturnResult(false);
	}
};