[BaseContainerProps()]
class SCR_MapCanRotateCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.m_bCanRotateMapElement);
	}
}
