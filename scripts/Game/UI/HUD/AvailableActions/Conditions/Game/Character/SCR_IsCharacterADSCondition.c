[BaseContainerProps()]
class SCR_IsCharacterADSCondition : SCR_AvailableActionCondition
{
	[Attribute("1", desc: "Check if character is aiming with gadget")]
	protected bool m_bCheckGadget;

	[Attribute("1", desc: "Check if character is aiming down the sights of his personal weapon")]
	protected bool m_bCheckPersonalWeapon;

	[Attribute("1", desc: "Check if character is aiming down the sights of a weapon that is mouneted on a turret")]
	protected bool m_bCheckTurretWeapon;

	//------------------------------------------------------------------------------------------------
	//! Returns true when character is in any form of ADS
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		if (m_bCheckGadget && data.GetGadgetRaised())
			return GetReturnResult(true);

		if (m_bCheckTurretWeapon)
		{
			TurretControllerComponent turretController = data.GetCurrentTurretController();
			if (turretController && turretController.IsWeaponADS())
				return GetReturnResult(true);
		}

		return GetReturnResult(m_bCheckPersonalWeapon && data.GetIsCharacterADS());
	}
}
