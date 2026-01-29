class SCR_CampaignArmoryStorageComponentClass : UniversalInventoryStorageComponentClass
{
}

class SCR_CampaignArmoryStorageComponent : UniversalInventoryStorageComponent
{
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eSupportedItemTypes;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eArsenalAmmunitionMode;
	
	protected bool m_bCanStoreItems = false;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] enable
	void AllowStoringItems(bool enable)
	{
		m_bCanStoreItems = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetSupportedItems()
	{
		return m_eSupportedItemTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemMode GetAmmunitionMode()
	{
		return m_eArsenalAmmunitionMode;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool CanStoreItem(IEntity item, int slotID)
	{
		if (!super.CanStoreItem(item, slotID))
			return false;
		
		if (!m_bCanStoreItems)
			return false;
		
		InventoryItemComponent inventoryComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (inventoryComponent)
		{
			
			if (inventoryComponent.GetParentSlot())
				return false;
			else
				return true;
		}
		
		return false;
	}
}
