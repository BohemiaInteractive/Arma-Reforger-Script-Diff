[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data (Alternative Refund)", "DISABLED - Arsenal Data (Alternative Refund)", 1)]
class SCR_ArsenalItemAlternativeRefundAmount : SCR_ArsenalItem
{
	[Attribute("10", desc: "Refund amount of the item which is seperate from the supply cost. Take into account that the supply refund multiplier is still added to this value", params: "0 inf 1")]
	protected int m_iOverwriteSupplyRefundAmount;
	
	//------------------------------------------------------------------------------------------------
	override protected int GetRefundAmountValue()
	{
		return m_iOverwriteSupplyRefundAmount;
	}
}
