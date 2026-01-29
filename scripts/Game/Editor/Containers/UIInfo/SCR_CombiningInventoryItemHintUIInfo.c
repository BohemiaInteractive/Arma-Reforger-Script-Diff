[BaseContainerProps(configRoot: true)]
class SCR_CombiningInventoryItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		return WidgetManager.Translate(GetName(), GetDescription());
	}
}
