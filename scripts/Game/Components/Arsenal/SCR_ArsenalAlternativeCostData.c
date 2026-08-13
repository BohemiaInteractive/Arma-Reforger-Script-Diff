[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_iSupplyCost", "1", "%1 - Supply cost: %2")]
class SCR_ArsenalAlternativeCostData : SCR_ArsenalAlternativeData
{
	[Attribute("1", desc: "Alternative supply cost", params: "0 inf")]
	int m_iSupplyCost;
	
	//------------------------------------------------------------------------------------------------
	override int GetSupplyCost(int defaultCost)
	{
		return m_iSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetRefundAmount(int defaultRefundAmount, IEntity soldItem = null)
	{
		return m_iSupplyCost;
	}
}
