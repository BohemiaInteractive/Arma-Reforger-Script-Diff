//------------------------------------------------------------------------------------------------
/*!
//! Returns true if character has radios of some count
*/

[BaseContainerProps()]
class SCR_CharacterRadioCountCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("0", UIWidgets.EditBox, "", "")]
	private int m_iValue;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		int count = data.GetCharacterRadiosCount();
		bool result = SCR_Comparer<int>.Compare(m_eOperator, count, m_iValue);
		return GetReturnResult(result);
	}
};
