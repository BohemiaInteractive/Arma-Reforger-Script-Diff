//------------------------------------------------------------------------------------------------
//! Returns true if character is in desired stance
[BaseContainerProps()]
class SCR_CharacterStanceCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: ECharacterStance.STAND.ToString(), desc: "Character Stance", uiwidget: UIWidgets.ComboBox, enumType: ECharacterStance)]
	private ECharacterStance m_eCharacterStance;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in desired stance
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetCharacterStance() == m_eCharacterStance);
	}
}
