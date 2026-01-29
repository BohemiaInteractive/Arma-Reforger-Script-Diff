//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_SightsHasIlluminationCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.CheckBox, desc: "True - find illumination of controlled turret | False - find current weapon in hand illumination")]
	protected bool m_bCheckTurret;

	//------------------------------------------------------------------------------------------------
	//! Return true if currently controlled vehicle turret has illumination for sights
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		SCR_2DOpticsComponent sights = null;

		if (m_bCheckTurret)
		{
			sights = CurrentTurretSight(data);
		}
		else
		{
			sights = CurrentWeaponSights(data);
		}

		if (!sights)
			return false;

		SCR_2DOpticsComponentClass sightsData = SCR_2DOpticsComponentClass.Cast(sights.GetComponentData(sights.GetOwner()));
		if (!sightsData)
			return false;

		// Has illumination
		return GetReturnResult(sightsData.HasIllumination());
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_2DOpticsComponent CurrentWeaponSights(SCR_AvailableActionsConditionData data)
	{
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return null;

		BaseWeaponManagerComponent weaponManager = character.GetCharacterController().GetWeaponManagerComponent();
		if (!weaponManager)
			return null;

		BaseSightsComponent currentSights = weaponManager.GetCurrentSights();
		if (!currentSights)
			return null;

		return SCR_2DOpticsComponent.Cast(currentSights);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_2DOpticsComponent CurrentTurretSight(SCR_AvailableActionsConditionData data)
	{
		// Current vehicle controller
		BaseControllerComponent controller = data.GetCurrentVehicleController();
		if (!controller)
			return null;

		// Turret
		IEntity turretEntity = controller.GetOwner();
		if (!turretEntity)
			return null;

		TurretComponent turret = TurretComponent.Cast(turretEntity.FindComponent(TurretComponent));
		if (!turret)
			return null;

		// Sights
		return SCR_2DOpticsComponent.Cast(turret.FindComponent(SCR_2DOpticsComponent));
	}
};
