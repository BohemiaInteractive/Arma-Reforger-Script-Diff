[BaseContainerProps()]
class SCR_IsUsingVariableSightsCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		BaseWeaponComponent weapon = data.GetCurrentWeapon();
		if (!weapon)
			return false;

		BaseSightsComponent sights = weapon.GetSights();
		if (!sights)
			return false;

		SCR_BaseVariableSightsFOVInfo info = SCR_BaseVariableSightsFOVInfo.Cast(sights.GetFOVInfo());
		return GetReturnResult(info && info.GetCount() > 1);
	}
}
