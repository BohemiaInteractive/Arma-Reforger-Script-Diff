[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_SupplyCapacityTooltipDetail : SCR_EntityTooltipDetail
{	
	[Attribute("#AR-ValueUnit_Short_Plus", desc: "Formatting of adding supplies. %1 being amount")]
	protected LocalizedString m_SupplyFormattingPositive;
	
	[Attribute("#AR-ValueUnit_Short_Minus", desc: "Formatting of removing supplies. %1 being amount")]
	protected LocalizedString m_SupplyFormattingNegative;
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		TextWidget text = TextWidget.Cast(widget);
		if (!text)
			return false;
		
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
		if (!editableUiInfo)
			return false;
		
		if (!editableUiInfo.HasEntityLabel(EEditableEntityLabel.SERVICE_SUPPLY_STORAGE))
			return false;
		
		if (SCR_CampaignMilitaryBaseComponent.SUPPLY_DEPOT_CAPACITY >= 0)
			text.SetTextFormat(m_SupplyFormattingPositive, SCR_CampaignMilitaryBaseComponent.SUPPLY_DEPOT_CAPACITY.ToString());
		else 
			text.SetTextFormat(m_SupplyFormattingNegative, SCR_CampaignMilitaryBaseComponent.SUPPLY_DEPOT_CAPACITY.ToString());
		
		return true;
	}
}
