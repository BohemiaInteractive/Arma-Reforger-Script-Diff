[BaseContainerProps()]
class SCR_VehicleOperabilityCheck : SCR_VehicleConditionCheck
{
	protected SCR_VehicleBuoyancyComponent m_VehicleBuoyancy;

	//------------------------------------------------------------------------------------------------
	override bool CheckCondition(Vehicle vehicle)
	{
		if (!super.CheckCondition(vehicle))
			return false;

		return IsVehicleOperable(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsVehicleOperable(notnull Vehicle vehicle)
	{
		SCR_AIVehicleUsageComponent aiVehicleUsageComponent = SCR_AIVehicleUsageComponent.Cast(vehicle.FindComponent(SCR_AIVehicleUsageComponent));
		if (aiVehicleUsageComponent && aiVehicleUsageComponent.GetDamageState() == EDamageState.DESTROYED)
			return false;

		if (!vehicle.IsAccessible())
			return false;

		m_VehicleBuoyancy = SCR_VehicleBuoyancyComponent.Cast(vehicle.FindComponent(SCR_VehicleBuoyancyComponent));
		if (m_VehicleBuoyancy)
		{
			if (m_VehicleBuoyancy.IsCenterOfMassInWater())
				return false;
		}

		SCR_DamageManagerComponent vehicleDamageManager = SCR_DamageManagerComponent.GetDamageManager(vehicle);
		return vehicleDamageManager &&
			vehicleDamageManager.GetMovementDamage() < 1 &&
			!vehicleDamageManager.IsOnFire(vehicleDamageManager.GetDefaultHitZone());
	}
}
