[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "%1 - Dynamic refund cost")]
class SCR_ReusableItemSellAmountData : SCR_ArsenalAlternativeData
{
	//------------------------------------------------------------------------------------------------
	override int GetRefundAmount(int defaultRefundAmount, IEntity soldItem = null)
	{
		if (!soldItem)
			return defaultRefundAmount;

		SCR_ReusableDeployableInventoryComponent itemComp = SCR_ReusableDeployableInventoryComponent.Cast(soldItem.FindComponent(SCR_ReusableDeployableInventoryComponent));
		if (!itemComp)
			return defaultRefundAmount;

		int remainingUses = itemComp.GetRemainingUses();
		if (remainingUses < 1)
			return 0;

		float refundValue = remainingUses / itemComp.GetMaxNumberOfUses() * defaultRefundAmount;
		return refundValue;
	}
}
