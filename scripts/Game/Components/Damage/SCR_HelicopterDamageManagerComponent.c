class SCR_HelicopterDamageManagerComponentClass : SCR_VehicleDamageManagerComponentClass
{
}

class SCR_HelicopterDamageManagerComponent : SCR_VehicleDamageManagerComponent
{
	protected ref array<SCR_RotorHitZone>	m_aRotorHitZones = {};
	protected float							m_fRotorsEfficiency = 1;

	//------------------------------------------------------------------------------------------------
	//! Disable contact mask, typically used for when vehicle becomes destroyed
	//! \param[in] enabled
	// TODO: Consider for all vehicles. Note destructible response indexes may still need it until wreck is stopped.
	void EnableContactMaskOnHost(bool enabled)
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner());
		if (!vehicle)
			return;

		RplComponent rpl = vehicle.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;

		if (enabled)
			SetEventMask(vehicle, EntityEvent.CONTACT);
		else
			ClearEventMask(vehicle, EntityEvent.CONTACT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		//! Disable collision damage calculations for wrecks
		EnableContactMaskOnHost(newState != EDamageState.DESTROYED);
	}

	//------------------------------------------------------------------------------------------------
	// Register simulation feature hitzone
	override void RegisterVehicleHitZone(notnull HitZone hitZone)
	{
		super.RegisterVehicleHitZone(hitZone);

		// If it has rotor point info, it is a physical rotor
		SCR_RotorHitZone rotorHitZone = SCR_RotorHitZone.Cast(hitZone);
		if (rotorHitZone && rotorHitZone.GetPointInfo() && !m_aRotorHitZones.Contains(rotorHitZone))
			m_aRotorHitZones.Insert(rotorHitZone);
	}

	//------------------------------------------------------------------------------------------------
	// Register simulation feature hitzone
	override void UnregisterVehicleHitZone(HitZone hitZone)
	{
		super.UnregisterVehicleHitZone(hitZone);

		// Also unregister rotor hitzones
		SCR_RotorHitZone rotorHitZone = SCR_RotorHitZone.Cast(hitZone);
		if (rotorHitZone && rotorHitZone.GetPointInfo())
			m_aRotorHitZones.RemoveItem(rotorHitZone);
	}

	//------------------------------------------------------------------------------------------------
	// Compute current simulation state of vehicle
	// Called when hitzone damage state needs to be interpreted
	override void UpdateVehicleState()
	{
		super.UpdateVehicleState();

		HelicopterControllerComponent controller = HelicopterControllerComponent.Cast(m_Controller);
		VehicleHelicopterSimulation simulation = VehicleHelicopterSimulation.Cast(m_Simulation);

		if (!controller || !simulation || !simulation.IsValid())
			return;

		int rotorID;
		SCR_RotorHitZone rotorHitZone;
		int rotorCount = simulation.RotorCount();
		array<float> rotorEfficiencies = {};

		// By default rotor efficiency shall match the combined engine and gearbox efficiency
		for (int i; i < rotorCount; i++)
		{
			rotorEfficiencies.Insert(m_fEngineEfficiency * m_fGearboxEfficiency);
		}

		foreach (HitZone hitZone : m_aVehicleHitZones)
		{
			rotorHitZone = SCR_RotorHitZone.Cast(hitZone);
			if (rotorHitZone)
			{
				rotorID = rotorHitZone.GetRotorIndex();
				if (rotorEfficiencies.IsIndexValid(rotorID))
					rotorEfficiencies[rotorID] = rotorEfficiencies[rotorID] * rotorHitZone.GetEfficiency();
			}
		}

		// Set rotor efficiency on simulation
		m_fRotorsEfficiency = 1;
		foreach (int i, float rotorEfficiency : rotorEfficiencies)
		{
			simulation.RotorSetForceScaleState(i, rotorEfficiency);
			simulation.RotorSetTorqueScaleState(i, rotorEfficiency);
			m_fRotorsEfficiency *= rotorEfficiency;
		}

		// Account for rotor damage
		UpdateMovementDamage();
	}

	//------------------------------------------------------------------------------------------------
	override void SetEngineFunctional(bool functional)
	{
		super.SetEngineFunctional(functional && m_bGearboxFunctional);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateMovementDamage()
	{
		float movementDamage;

		if (!m_bEngineFunctional)
			movementDamage = 1;
		else
			movementDamage = 1 - (m_fGearboxEfficiency * m_fEngineEfficiency * m_fRotorsEfficiency);

		if (!float.AlmostEqual(movementDamage, GetMovementDamage()))
			SetMovementDamage(movementDamage);
		
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller)
			controller.SetCanMove(movementDamage < 1);
	}
}
