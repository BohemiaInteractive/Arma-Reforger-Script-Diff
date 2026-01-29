//------------------------------------------------------------------------------------------------
//! Returns true if ammo of current vehicle weapon matches the condition
[BaseContainerProps()]
class SCR_VehicleWeaponAmmoCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("1", UIWidgets.ComboBox, "Should comparison be done using percentage of ammo?", "" )]
	private bool m_bComparePercentage;

	[Attribute("2", UIWidgets.EditBox, "Percentages are 0-100, non-percentage values are rounded to integers", "")]
	private float m_fValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when currently controled vehicle weapon in vehicle has met the specified ammo condition
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		BaseWeaponComponent weapon = data.GetCurrentVehicleWeapon();
		if (!weapon)
			return false;

		BaseMagazineComponent magazine = weapon.GetCurrentMagazine();
		if (!magazine)
			return false;

		int current = magazine.GetAmmoCount();
		int max = magazine.GetMaxAmmoCount();
		bool result = false;
		if (m_bComparePercentage)
		{
			if (max > 0.0)
			{
				float perc = (current/(float)max) * 100.0;
				result = SCR_Comparer<float>.Compare(m_eOperator, perc, m_fValue);
				return GetReturnResult(result);
			}
			return GetReturnResult(false);
		}

		result = SCR_Comparer<int>.Compare(m_eOperator, current, (int)m_fValue);
		return GetReturnResult(result);
	}
};
