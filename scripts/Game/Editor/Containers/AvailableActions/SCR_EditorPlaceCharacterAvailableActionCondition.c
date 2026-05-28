[BaseContainerProps()]
class SCR_EditorPlaceCharacterAvailableActionCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(!SCR_PlayerController.GetLocalControlledEntity());
	}
}