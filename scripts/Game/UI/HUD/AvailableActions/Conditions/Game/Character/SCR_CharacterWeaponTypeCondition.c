//------------------------------------------------------------------------------------------------
//! Returns true if character is using weapon of given type
[BaseContainerProps()]
class SCR_CharacterWeaponTypeCondition : SCR_AvailableActionCondition
{
	[Attribute("", UIWidgets.EditBox, "", "")]
	private string m_sValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has a weapon equipped of given type
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetCurrentWeaponEntity() != null;

		if (result)
			result = data.GetCurrentWeapon().GetWeaponSlotType() == m_sValue;

		return GetReturnResult(result);
	}
};
