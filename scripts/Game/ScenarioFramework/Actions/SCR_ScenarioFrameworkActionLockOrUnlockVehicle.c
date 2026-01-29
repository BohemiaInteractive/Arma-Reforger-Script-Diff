//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLockOrUnlockVehicle : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "true", desc: "If set to true, it will lock the vehicle, if set to false it will unlock the vehicle")]
	bool m_bLock;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		if (!Vehicle.Cast(entity))
			return;

		SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(entity.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (!spawnProtectionComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Spawn Protection Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Spawn Protection Component Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		if (m_bLock)
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
			spawnProtectionComponent.SetVehicleOwner(-2);
		}
		else
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(true);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_CannotEnterVehicle-UC");
			spawnProtectionComponent.ReleaseProtection();
		}
	}
}
//---- REFACTOR NOTE END ----