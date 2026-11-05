[BaseContainerProps()]
class SCR_WaterInventoryItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!item || !super.CanBeShown(item, focusedSlot))
			return false;

		return SCR_WaterContainerComponent.Cast(item.GetOwner().FindComponent(SCR_WaterContainerComponent));
	}

	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);

		SCR_WaterContainerComponent waterContainer = SCR_WaterContainerComponent.Cast(item.GetOwner().FindComponent(SCR_WaterContainerComponent));
		if (!waterContainer)
			return super.GetItemHintName(item);

		return WidgetManager.Translate(GetName(), waterContainer.GetCurrentWater().ToString(-1, 2), waterContainer.GetMaximumWater().ToString(-1, 2));
	}
}
