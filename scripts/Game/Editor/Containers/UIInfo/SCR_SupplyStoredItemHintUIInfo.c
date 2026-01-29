[BaseContainerProps(configRoot: true)]
class SCR_SupplyStoredItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		float totalResources, maxResources;
		return SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, totalResources, maxResources);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!m_ResourceComponent)
			return super.GetItemHintName(item);
		
		float totalResources, maxResources;
		SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, totalResources, maxResources);

		float additionalTotalResources, additionalMaxResources;
		SCR_SupplyInventorySlotUI supplySlot = SCR_SupplyInventorySlotUI.Cast(m_LastFocusedSlot);
		if (supplySlot)
			supplySlot.GetAdditionalResourceValuesFromStack(additionalTotalResources, additionalMaxResources);

		return WidgetManager.Translate(
			GetName(), 
			SCR_ResourceSystemHelper.SuppliesToString(totalResources + additionalTotalResources), 
			SCR_ResourceSystemHelper.SuppliesToString(maxResources + additionalMaxResources));
	}
}
