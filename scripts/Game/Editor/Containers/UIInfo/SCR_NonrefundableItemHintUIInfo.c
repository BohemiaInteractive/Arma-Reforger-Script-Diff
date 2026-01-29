[BaseContainerProps(configRoot: true)]
class SCR_NonrefundableItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected bool m_bContainsNonRefundableItem;
	
	//------------------------------------------------------------------------------------------------
	void SetContainsNonrefundableItem(bool containsNonRefundableItem)
	{
		m_bContainsNonRefundableItem = containsNonRefundableItem;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (m_bContainsNonRefundableItem)
			return GetDescription();
		else 
			return GetName();
	}
}
