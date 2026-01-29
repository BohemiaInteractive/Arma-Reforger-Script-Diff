//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_MapCanDragCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (data.m_bCanDragMapElement)
			return GetReturnResult(true);

		return GetReturnResult(false);
	}
};
