[BaseContainerProps(configRoot: true)]
class SCR_SupplyRefundItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected float m_fItemSupplyRefund = -1;
	protected bool m_bIsSupplyStorageAvailable;
	
	//------------------------------------------------------------------------------------------------
	void SetSupplyRefund(float supplyRefund, bool isSupplyStorageAvailable)
	{
		m_fItemSupplyRefund = supplyRefund;
		m_bIsSupplyStorageAvailable = isSupplyStorageAvailable;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		return item && m_fItemSupplyRefund >= 0 && super.CanBeShown(item, focusedSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (m_fItemSupplyRefund == 0 || m_bIsSupplyStorageAvailable)
			return WidgetManager.Translate(GetName(), SCR_ResourceSystemHelper.SuppliesToString(m_fItemSupplyRefund));
		else 
			return WidgetManager.Translate(GetDescription(), SCR_ResourceSystemHelper.SuppliesToString(m_fItemSupplyRefund));
	}
}
