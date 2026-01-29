//------------------------------------------------------------------------------------------------
//! Returns true if current weapon's number of firemodes matches current condition
[BaseContainerProps()]
class SCR_WeaponFiremodeCountCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Min firemodes, 2 = safety & default usually", "")]
	private int m_iValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity's current weapon number of firemodes matches the specified condition
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		bool result = SCR_Comparer<int>.Compare(m_eOperator, muzzle.GetFireModesCount(), m_iValue);
		
		return GetReturnResult(result);
	}
};
