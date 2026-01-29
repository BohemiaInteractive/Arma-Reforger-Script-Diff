//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLockOrUnlockAllTargetVehiclesInTrigger : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which spawns the trigger")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "true", desc: "If set to true, it will lock all vehicles, if set to false it will unlock all vehicles")]
	bool m_bLock;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[out] trigger
	//! \return
	bool CanActivateTriggerVariant(IEntity object, out SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] trigger
	void LockOrUnlockAllVehicles(SCR_ScenarioFrameworkTriggerEntity trigger = null)
	{
		if (!trigger)
			return;

		array<IEntity> entitesInside = {};
		trigger.GetEntitiesInside(entitesInside);
		if (entitesInside.IsEmpty())
			return;

		foreach (IEntity entity : entitesInside)
		{
			if (!Vehicle.Cast(entity))
				continue;

			SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(entity.FindComponent(SCR_VehicleSpawnProtectionComponent));
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

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_ScenarioFrameworkTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			LockOrUnlockAllVehicles(trigger);
			return;
		}

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = entityWrapper.GetValue();
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entityFrom);
		if (trigger)
		{
			LockOrUnlockAllVehicles(trigger);
			return;
		}
	}
}
//---- REFACTOR NOTE END ----