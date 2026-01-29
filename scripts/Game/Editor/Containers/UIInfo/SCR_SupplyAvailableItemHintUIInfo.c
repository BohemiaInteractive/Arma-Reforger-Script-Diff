[BaseContainerProps(configRoot: true)]
class SCR_SupplyAvailableItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		if (!m_ResourceComponent.IsResourceTypeEnabled())
			return false;
		
		float availableResources;
		return SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, availableResources);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!m_ResourceComponent)
			return super.GetItemHintName(item);
		
		float availableResources;
		SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, availableResources);
		
		return WidgetManager.Translate(GetName(), SCR_ResourceSystemHelper.SuppliesToString(availableResources));
	}
}
