[BaseContainerProps()]
class SCR_MapCanResetRotationCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data && data.m_bCanRotateMapElement && data.m_bCanActivateMapElement && data.m_bCanResetElementRotation);
	}
}