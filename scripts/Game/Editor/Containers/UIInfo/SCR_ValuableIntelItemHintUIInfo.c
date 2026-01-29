[BaseContainerProps(configRoot: true)]
class SCR_ValuableIntelItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected Faction m_IntelFaction;
	
	//------------------------------------------------------------------------------------------------
	void SetIntel(SCR_IdentityInventoryItemComponent intelItem)
	{
		m_IntelFaction = intelItem.GetValuableIntelFaction();
		
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!playerFaction)
			return;
		
		UIInfo uiInfo = playerFaction.GetUIInfo();
		if (!uiInfo)
			return;
		
		Icon = uiInfo.GetIconPath();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!m_IntelFaction)
			return false;
		
		return super.CanBeShown(item, focusedSlot);
	}
}
