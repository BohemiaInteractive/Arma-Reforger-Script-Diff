[BaseContainerProps(configRoot: true)]
class SCR_ReusableInventoryItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		SCR_ReusableDeployableInventoryComponent reusableIIC = SCR_ReusableDeployableInventoryComponent.Cast(item);
		if (!reusableIIC)
			return string.Empty;

		return WidgetManager.Translate(GetName(), reusableIIC.GetRemainingUses());
	}
}
