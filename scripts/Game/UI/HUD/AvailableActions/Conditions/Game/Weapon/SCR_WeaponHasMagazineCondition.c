//------------------------------------------------------------------------------------------------
//! Returns true if weapon has loaded magazine
[BaseContainerProps()]
class SCR_WeaponHasMagazineCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when magazine is in current weapon
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = false;

		BaseMagazineComponent magazine = data.GetCurrentMagazine();
		if (magazine)
			result = true;

		return GetReturnResult(result);
	}
};
