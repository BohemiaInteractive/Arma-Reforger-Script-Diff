[BaseContainerProps()]
class SCR_EditorPlaceCharacterAvailableActionCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		return !controlledEntity;
	}
};