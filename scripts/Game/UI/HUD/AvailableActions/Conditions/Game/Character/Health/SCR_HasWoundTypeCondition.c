[BaseContainerProps()]
class SCR_HasWoundTypeCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		IEntity controlled = data.GetCharacter();
		if (!controlled)
			return false;
		
		IEntity item = data.GetCurrentItemEntity();
		if (!item)
			return false;
		
		SCR_ConsumableItemComponent medicalItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!medicalItem)
			return false;
		
		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(medicalItem.GetConsumableEffect());
		if (!effect)
			return false;
		
		if (!data.GetCharacterDamageComponent())
			return false;
		
		bool result = effect.CanApplyEffect(controlled, controlled);
		return GetReturnResult(result);
	}
}