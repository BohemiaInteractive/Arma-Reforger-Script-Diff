[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_iEmptyCostReduction", "1", "%1 - Empty cost deduction: %2")]
class SCR_SingleShotWeaponSellAmountData : SCR_ArsenalAlternativeData
{
	[Attribute(defvalue: "1", desc: "Amount of supplies which is going to be deducted from default refund value for the fact that weapon is empty", params: "1 inf")]
	protected int m_iEmptyCostReduction;

	//------------------------------------------------------------------------------------------------
	override int GetRefundAmount(int defaultRefundAmount, IEntity soldItem = null)
	{
		if (!soldItem)
			return defaultRefundAmount;

		BaseMuzzleComponent muzzle = BaseMuzzleComponent.Cast(soldItem.FindComponent(BaseMuzzleComponent));
		if (!muzzle)
			return defaultRefundAmount;

		if (muzzle.GetAmmoCount() == muzzle.GetMaxAmmoCount())
			return defaultRefundAmount;

		return Math.Max(0, defaultRefundAmount - m_iEmptyCostReduction);
	}
}
