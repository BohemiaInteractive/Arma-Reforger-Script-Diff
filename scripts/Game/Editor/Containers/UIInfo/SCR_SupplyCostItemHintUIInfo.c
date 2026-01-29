[BaseContainerProps(configRoot: true)]
class SCR_SupplyCostItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected float m_fItemSupplyCost = -1;
	
	//------------------------------------------------------------------------------------------------
	void SetSupplyCost(float supplyCost)
	{
		m_fItemSupplyCost = supplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		return item && m_fItemSupplyCost >= 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		return WidgetManager.Translate(GetName(), SCR_ResourceSystemHelper.SuppliesToString(m_fItemSupplyCost));
	}
}
