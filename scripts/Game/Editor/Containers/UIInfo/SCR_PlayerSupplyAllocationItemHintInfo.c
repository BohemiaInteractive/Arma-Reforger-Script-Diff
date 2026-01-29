[BaseContainerProps(configRoot: true)]
class SCR_PlayerSupplyAllocationItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected float m_fItemMSARCost = -1;

	//------------------------------------------------------------------------------------------------
	void SetMSARCost(float cost)
	{
		m_fItemMSARCost = cost;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;

		return item && m_fItemMSARCost > 0;
	}

	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		return WidgetManager.Translate(GetName(), SCR_ResourceSystemHelper.SuppliesToString(m_fItemMSARCost));
	}
}
