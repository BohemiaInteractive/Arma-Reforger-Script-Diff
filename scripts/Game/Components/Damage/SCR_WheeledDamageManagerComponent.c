class SCR_WheeledDamageManagerComponentClass : SCR_VehicleDamageManagerComponentClass
{
}

class SCR_WheeledDamageManagerComponent : SCR_VehicleDamageManagerComponent
{
	protected float m_fWheelsEfficiency = 1;

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWheelsEfficiency()
	{
		return m_fWheelsEfficiency;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetWheelsEfficiency(float efficiency)
	{
		m_fWheelsEfficiency = efficiency;
	}

	//------------------------------------------------------------------------------------------------
	// Compute current simulation state of vehicle
	// Called when hitzone damage states change
	override void UpdateVehicleState()
	{
		int wheelsCount;
		float wheelsEfficiency;
		SCR_WheelHitZone wheelHitZone;

		foreach (HitZone hitZone : m_aVehicleHitZones)
		{
			wheelHitZone = SCR_WheelHitZone.Cast(hitZone);
			if (!wheelHitZone || wheelHitZone.GetWheelIndex() < 0)
				continue;

			wheelsCount++;
			wheelsEfficiency += wheelHitZone.GetEfficiency();
		}

		if (wheelsCount > 0)
			SetWheelsEfficiency(wheelsEfficiency / wheelsCount);

		super.UpdateVehicleState();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateMovementDamage()
	{
		float movementDamage;

		if (!m_bEngineFunctional)
			movementDamage = 1;
		else
			movementDamage = 1 - (m_fGearboxEfficiency * m_fEngineEfficiency * m_fWheelsEfficiency);

		if (!float.AlmostEqual(movementDamage, GetMovementDamage()))
			SetMovementDamage(movementDamage);
		
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller)
			controller.SetCanMove(movementDamage < 1);
	}
}
