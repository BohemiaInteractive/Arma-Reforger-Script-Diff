//! Returns true if weapon has loaded magazine
[BaseContainerProps()]
class SCR_WeaponBipodCondition : SCR_AvailableActionCondition
{
	[Attribute("true", uiwidget: UIWidgets.CheckBox, "True = unfolded, False = folded")]
	protected bool m_bBipodState;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current weapon has bipod and matches the desired state
	//! Returns opposite if m_bNegateCondition is enabled, but only if weapon has bipod
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		// Only evaluate weapons equipped with bipod
		BaseWeaponComponent weapon = data.GetCurrentWeapon();
		return GetReturnResult(weapon && weapon.HasBipod() && weapon.GetBipod() == m_bBipodState);
	}
}
