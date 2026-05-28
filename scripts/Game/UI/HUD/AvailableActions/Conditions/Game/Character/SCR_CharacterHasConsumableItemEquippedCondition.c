//! Returns true if character has provided gadget in their inventory
//! Set m_eConsumableType to filter for a specific item
[BaseContainerProps()]
class SCR_CharacterHasConsumableItemEquippedCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_EConsumableType.NONE.ToString(), desc: "Consumable type to be tested", uiwidget: UIWidgets.ComboBox, enumType: SCR_EConsumableType)]
	private SCR_EConsumableType m_eConsumableType;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has specified gadget
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		IEntity item = data.GetCurrentItemEntity();
		if (!item)
			return GetReturnResult(false);

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		return GetReturnResult(consumable && consumable.GetConsumableType() == m_eConsumableType);
	}
}
