[BaseContainerProps()]
class SCR_CanUseCanteenCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Checks whether the held canteen has enough water and can be used by the character.
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		IEntity character = data.GetCharacter();
		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(data.GetHeldGadgetComponent());
		if (!character || !consumable)
			return GetReturnResult(false);

		SCR_ConsumableFlaskEffect effect = SCR_ConsumableFlaskEffect.Cast(consumable.GetConsumableEffect());
		return GetReturnResult(effect && effect.CanApplyItemEffect(character, character, consumable.GetOwner()));
	}
}
