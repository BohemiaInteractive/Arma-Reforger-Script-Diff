[BaseContainerProps()]
class SCR_FuelItemHintsUIInfo : SCR_InventoryItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!item || !super.CanBeShown(item, focusedSlot))
			return false;
		
		return FuelManagerComponent.Cast(item.GetOwner().FindComponent(FuelManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);
		
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(item.GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return super.GetItemHintName(item);
		
		return WidgetManager.Translate(GetName(), SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(fuelManager.GetTotalFuel(), 1), SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(fuelManager.GetTotalMaxFuel(), 1));
	}
}
