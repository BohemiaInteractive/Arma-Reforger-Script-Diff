[BaseContainerProps()]
class SCR_MapCanDragCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.m_bCanDragMapElement);
	}
}
