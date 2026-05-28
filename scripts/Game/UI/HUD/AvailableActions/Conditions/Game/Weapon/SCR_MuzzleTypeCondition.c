//! Returns true if current muzzle's type matches specified type
[BaseContainerProps()]
class SCR_MuzzleTypeCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EMuzzleType.MT_BaseMuzzle.ToString(), desc: "Muzzle Type", uiwidget: UIWidgets.ComboBox, enumType: EMuzzleType)]
	protected EMuzzleType m_eMuzzleType;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current muzzle's type matches specified type
	//! Returns opposite if m_bNegateCondition is enabled, but only if there is current muzzle at all
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		return GetReturnResult(muzzle && muzzle.GetMuzzleType() == m_eMuzzleType);
	}
}
