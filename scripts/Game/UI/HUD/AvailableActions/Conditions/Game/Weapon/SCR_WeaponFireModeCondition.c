//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_WeaponFiremodeCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EWeaponFiremodeType.Safety.ToString(), desc: "Fire mode to check", uiwidget: UIWidgets.ComboBox, enumType: EWeaponFiremodeType)]
	protected EWeaponFiremodeType m_iValue;

	[Attribute("true", uiwidget: UIWidgets.CheckBox, "True = currently active, False = contains")]
	protected bool m_bIsCurrent;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		// Check muzzle
		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		// Get result
		bool result;

		if (m_bIsCurrent)
		{
			BaseFireMode mode = muzzle.GetCurrentFireMode();
			if (mode)
				result = (mode.GetFiremodeType() == m_iValue);
		}
		else
			result = ContainsMode(muzzle, m_iValue);

		return GetReturnResult(result);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ContainsMode(BaseMuzzleComponent muzzle, EWeaponFiremodeType modeType)
	{
		// Get fire modes list
		array<BaseFireMode> fireModes = {};
		muzzle.GetFireModesList(fireModes);

		// Go throught fire modes
		int count = fireModes.Count();
		for (int i = 0; i < count; i++)
		{
			// Contains mode
			if (fireModes[i].GetFiremodeType() == modeType)
				return true;
		}

		// Not found
		return false;
	}
}
