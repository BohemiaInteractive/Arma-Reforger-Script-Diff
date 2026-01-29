//------------------------------------------------------------------------------------------------
//! Returns true IF camera focus mode is meeting current conditions
[BaseContainerProps()]
class SCR_CharacterFocusModeCondition : SCR_AvailableActionCondition
{
	[Attribute("1", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("0.05", UIWidgets.EditBox, "0-1 for focus mode", "")]
	private float m_fValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in focus mode
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = SCR_Comparer<float>.Compare(m_eOperator, data.GetFocusModeAmount(), m_fValue);
		return GetReturnResult(result);
	}
};
