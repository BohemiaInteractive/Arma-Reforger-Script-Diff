[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_FactionOutfitItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected Faction m_Faction;
	
	protected int m_iValue;
	
	//------------------------------------------------------------------------------------------------
	void SetFaction(notnull Faction faction, notnull SCR_OutfitFactionData outfitFactionData)
	{
		m_Faction = faction;
		
		m_iValue = outfitFactionData.GetOutfitFactionValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget)
			return false;
		
		ResourceName icon = m_Faction.GetUIInfo().GetIconPath();
		if (icon.IsEmpty())
			return false;
		
		imageWidget.LoadImageTexture(0, icon);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetItemHintNameTo(InventoryItemComponent item, TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetTextFormat(GetItemHintName(item), m_Faction.GetUIInfo().GetName());
		return true;
	}
}
