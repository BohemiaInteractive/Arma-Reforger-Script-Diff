//! Returns true if character has radios of some count
[BaseContainerProps()]
class SCR_CharacterRadioCountCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("0", UIWidgets.EditBox, "", "")]
	private int m_iValue;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(SCR_Comparer<int>.Compare(m_eOperator, data.GetCharacterRadiosCount(), m_iValue));
	}
}
