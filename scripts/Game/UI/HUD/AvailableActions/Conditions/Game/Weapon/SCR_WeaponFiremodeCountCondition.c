//! Returns true if current weapon's number of firemodes matches current condition
[BaseContainerProps()]
class SCR_WeaponFiremodeCountCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Min firemodes, 2 = safety & default usually")]
	private int m_iValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity's current weapon number of firemodes matches the specified condition
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		return GetReturnResult(muzzle && SCR_Comparer<int>.Compare(m_eOperator, muzzle.GetFireModesCount(), m_iValue));
	}
}
