[BaseContainerProps(configRoot: true)]
class SCR_BlockUnequipItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		//~ Only show if item is not in slot of another character
		if (item.GetOwner().GetParent() != SCR_PlayerController.GetLocalControlledEntity())
			return false;
		
		//~ Show when Item is in equipment slot
		return SCR_EquipmentStorageSlot.Cast(item.GetParentSlot()) != null;
	}
}
