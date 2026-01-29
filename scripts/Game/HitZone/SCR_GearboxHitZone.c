class SCR_GearboxHitZone : SCR_VehicleHitZone
{
	[Attribute("0.25", UIWidgets.EditBox, desc: "Minimum gearbox efficiency ratio", params: "0 1 0.01")]
	protected float m_fMinimumGearboxEfficiencyRatio;

	//------------------------------------------------------------------------------------------------
	float GetEfficiency()
	{
		EDamageState state = GetDamageState();
		float efficiency = GetDamageStateThreshold(state);
		return Math.Lerp(m_fMinimumGearboxEfficiencyRatio, 1, efficiency);
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		Vehicle vehicle = Vehicle.Cast(pOwnerEntity.GetRootParent());
		if (!vehicle)
			return;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (damageManager)
			damageManager.RegisterVehicleHitZone(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		UpdateGearboxState();
	}
	//------------------------------------------------------------------------------------------------
	void UpdateGearboxState()
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.GetDamageManager());
		if (damageManager)
			damageManager.UpdateVehicleState();
	}
}
