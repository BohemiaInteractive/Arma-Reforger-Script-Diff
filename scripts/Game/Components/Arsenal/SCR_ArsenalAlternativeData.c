[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_eAlternativeCostType", "1", "%1")]
class SCR_ArsenalAlternativeData
{
	[Attribute(SCR_EArsenalSupplyCostType.GADGET_ARSENAL.ToString(), desc: "Cost type if system searches for the cost", uiwidget: UIWidgets.SearchComboBox, enumType: SCR_EArsenalSupplyCostType)]
	SCR_EArsenalSupplyCostType m_eAlternativeCostType;

	//------------------------------------------------------------------------------------------------
	int GetSupplyCost(int defaultCost)
	{
		return defaultCost;
	}

	//------------------------------------------------------------------------------------------------
	int GetRefundAmount(int defaultRefundAmount, IEntity soldItem = null)
	{
		return defaultRefundAmount;
	}
}
