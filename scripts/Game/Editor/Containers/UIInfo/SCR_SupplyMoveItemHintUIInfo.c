[BaseContainerProps(configRoot: true)]
class SCR_SupplyMoveItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		float totalResources, maxResources;
		return SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, totalResources, maxResources);
	}
}
