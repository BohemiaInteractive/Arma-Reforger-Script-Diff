//------------------------------------------------------------------------------------------------
//! Returns true if weapon has loaded magazine
[BaseContainerProps()]
class SCR_WeaponBipodCondition : SCR_AvailableActionCondition
{
	[Attribute("true", uiwidget: UIWidgets.CheckBox, "True = unfolded, False = folded")]
	protected bool m_bBipodState;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current weapon has bipod and matches the desired state
	//! Returns opposite if m_bNegateCondition is enabled, but only if weapon has bipod
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		// Only evaluate weapons equipped with bipod
		BaseWeaponComponent weapon = data.GetCurrentWeapon();
		if (!weapon || !weapon.HasBipod())
			return false;

		return GetReturnResult(weapon.GetBipod() == m_bBipodState);
	}
};
