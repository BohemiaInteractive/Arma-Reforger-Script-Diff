//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_WeaponFiremodeCondition : SCR_AvailableActionCondition
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Fire mode to check", "", ParamEnumArray.FromEnum(EWeaponFiremodeType))]
	protected EWeaponFiremodeType m_iValue;

	[Attribute("true", uiwidget: UIWidgets.CheckBox, "True = currently active, False = contains")]
	protected bool m_bIsCurrent;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		// Check muzzle
		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		// Get result
		bool result = false;

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
};
