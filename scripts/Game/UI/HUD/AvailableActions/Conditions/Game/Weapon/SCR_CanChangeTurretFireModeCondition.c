[BaseContainerProps()]
class SCR_CanChangeTurretFireModeCondition : SCR_AvailableActionCondition
{
	[Attribute(SCR_EFireModeChange.WEAPON_GROUP.ToString(), desc: "Dictates what should be checked for avialable", uiwidget: UIWidgets.ComboBox, enumType: SCR_EFireModeChange)]
	protected SCR_EFireModeChange m_eCheckAvailable;

	//------------------------------------------------------------------------------------------------
	//! Return true if character is in ADS of current controlled vehicle turret
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		ChimeraCharacter character = data.GetCharacter();
		if (!character)
			return false;

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (!compartmentAccessComponent)
			return false;

		if (compartmentAccessComponent.IsGettingOut() || compartmentAccessComponent.IsGettingOut())
			return false;

		SCR_FireModeManagerComponent fireModeMgr = compartmentAccessComponent.GetControlledFireModeManager();
		if (!fireModeMgr)
			return false;

		int availableOptions;
		switch (m_eCheckAvailable)
		{
			case SCR_EFireModeChange.WEAPON_GROUP:
				array<ref SCR_WeaponGroup> output = {};
				fireModeMgr.GetAllWeaponGroups(output);
				availableOptions = output.Count();
				break;

			case SCR_EFireModeChange.FIRE_MODE:
				availableOptions = fireModeMgr.GetNumberOfAvailableFireModes();
				break;

			case SCR_EFireModeChange.RIPPLE_QUANTITY:
				array<int> output = {};
				availableOptions = fireModeMgr.GetAvialableRippleQuantities(output);
				break;
		}

		return GetReturnResult(availableOptions > 1);
	}
}