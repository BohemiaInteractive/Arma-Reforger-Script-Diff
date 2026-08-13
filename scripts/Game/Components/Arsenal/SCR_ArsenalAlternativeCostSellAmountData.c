[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_iSupplyCost", "1", "%1 - Supply cost: %2")]
class SCR_ArsenalAlternativeCostSellAmountData : SCR_ArsenalAlternativeCostData
{
	[Attribute("1", desc: "Alternative supply refund amount. Supply refund multiplier is still added to it", params: "0 inf")]
	protected int m_iSupplyRefundAmount;
	
	//------------------------------------------------------------------------------------------------
	override int GetRefundAmount(int defaultRefundAmount, IEntity soldItem = null)
	{
		return m_iSupplyRefundAmount;
	}
}
