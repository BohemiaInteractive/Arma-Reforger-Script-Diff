//! Returns true if weapon has loaded magazine
[BaseContainerProps()]
class SCR_WeaponHasMagazineCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when magazine is in current weapon
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetCurrentMagazine() != null);
	}
}
