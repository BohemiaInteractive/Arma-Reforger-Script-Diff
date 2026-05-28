//! Returns true if the count of additional magazines in character's inventory matches current condition
[BaseContainerProps()]
class SCR_WeaponMagazineCountCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("0", UIWidgets.EditBox, "", "")]
	private int m_iValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity owns amount of extra magazines for current weapon specified by the condition
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		int additionalMagazines = data.GetAdditionalMagazinesCount();
		bool result = SCR_Comparer<int>.Compare(m_eOperator, additionalMagazines, m_iValue);
		return GetReturnResult(result);
	}
}
