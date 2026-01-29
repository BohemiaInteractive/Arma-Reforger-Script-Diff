//------------------------------------------------------------------------------------------------
//! Returns true if character has weapon equipped
[BaseContainerProps()]
class SCR_CharacterHasWeaponCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has a weapon equipped
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetCurrentWeaponEntity() != null;
		return GetReturnResult(result);
	}
};
