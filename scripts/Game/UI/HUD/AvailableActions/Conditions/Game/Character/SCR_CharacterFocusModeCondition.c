//! Returns true IF camera focus mode is meeting current conditions
[BaseContainerProps()]
class SCR_CharacterFocusModeCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("0.05", UIWidgets.EditBox, "0-1 for focus mode", "")]
	private float m_fValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in focus mode
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(SCR_Comparer<float>.Compare(m_eOperator, data.GetFocusModeAmount(), m_fValue));
	}
}
