//------------------------------------------------------------------------------------------------
//! Returns true if character is in desired stance
[BaseContainerProps()]
class SCR_CharacterStanceCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.ComboBox, "Character stance", "", ParamEnumArray.FromEnum(ECharacterStance) )]
	private ECharacterStance m_eCharacterStance;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in desired stance
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetCharacterStance() == m_eCharacterStance;
		return GetReturnResult(result);
	}
};
