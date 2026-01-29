[BaseContainerProps()]
class SCR_CanItemBeInspectedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity holds an item that can be inspected in that moment
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		IEntity item = data.GetCurrentItemEntity();
		if (!item)//if player is not holding a gadget then check if he is holding a weapon
		{
			item = data.GetCurrentWeaponEntity();
			if (!item)
				return GetReturnResult(false);
		}

		CharacterControllerComponent controller = data.GetCharacterController();
		if (!controller)
			return GetReturnResult(false);

		return GetReturnResult(controller.CanInspect(item));
	}
}
