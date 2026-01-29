[BaseContainerProps()]
class SCR_BaseSupplyItemHintUIInfo : SCR_InventoryItemHintUIInfo
{	
	protected SCR_ResourceComponent m_ResourceComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{		
		m_ResourceComponent = GetResourceComponent(item);
		if (!m_ResourceComponent)
			return false;
			
		return super.CanBeShown(item, focusedSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceComponent GetResourceComponent(InventoryItemComponent item)
	{
		if (!item)
			return null;
		
		return SCR_ResourceComponent.FindResourceComponent(item.GetOwner());
	}
}
