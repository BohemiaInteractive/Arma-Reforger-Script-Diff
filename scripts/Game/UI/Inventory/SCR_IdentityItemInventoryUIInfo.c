[BaseContainerProps()]
class SCR_IdentityItemInventoryUIInfo : SCR_InventoryUIInfo
{
	protected const LocalizedString INTEL_TEXT = "#AR-Item_Identity_ValuableIntel";
	
	//------------------------------------------------------------------------------------------------
	override string GetInventoryItemDescription(InventoryItemComponent item)
	{
		SCR_IdentityInventoryItemComponent identityItem = SCR_IdentityInventoryItemComponent.Cast(item);
		if (!identityItem)
			return super.GetInventoryItemDescription(item);
		
		SCR_ExtendedIdentityComponent identityComponent = identityItem.GetLinkedExtendedIdentity();
		if (!identityComponent)
			return GetValuableIntelItemDescription(item, identityItem);

		//~ Do not set description as it is handled by the Identity UI
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetValuableIntelItemDescription(InventoryItemComponent item, SCR_IdentityInventoryItemComponent identityItem)
	{
		if (!identityItem || !identityItem.HasValuableIntel())
			return super.GetInventoryItemDescription(item);
		
		return super.GetInventoryItemDescription(item) + "\n\n" + INTEL_TEXT;
		
	}
}
