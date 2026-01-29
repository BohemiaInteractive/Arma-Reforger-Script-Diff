class SCR_EngineHitZone : SCR_VehicleHitZone
{
	[Attribute("25", UIWidgets.Auto, desc: "Minimum engine power scale (% of initial)", params: "0 100 0.1")]
	protected float m_fMinimumEnginePowerScale;

	[Attribute("20", UIWidgets.Auto, desc: "Minimum engine startup chance (% of initial)", params: "0 100 0.1")]
	protected float m_fMinimumEngineStartupChance;
	
	protected SCR_PowerComponent m_Power;

	//------------------------------------------------------------------------------------------------
	float GetEfficiency()
	{
		EDamageState state = GetDamageState();
		float efficiency = GetDamageStateThreshold(state);

		// Battery influence on engine power output
		if (m_Power && m_Power.ShouldApplyNoPowerPenalty() && !m_Power.HasPower())
			efficiency *= m_Power.GetNoPowerMultiplier();

		return Math.Lerp(0.01 * m_fMinimumEnginePowerScale, 1, efficiency);
	}

	//! Called when hit zone is initialized
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		Vehicle vehicle = Vehicle.Cast(pOwnerEntity.GetRootParent());
		if (!vehicle)
			return;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (damageManager)
			damageManager.RegisterVehicleHitZone(this);

		m_Power = SCR_PowerComponent.Cast(vehicle.FindComponent(SCR_PowerComponent));
		if (m_Power)
			m_Power.GetEventBatteryStateChanged().Insert(UpdateEngineState);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone.
	This is also called when transmitting the damage to parent hitzones!
	\param type Type of damage
	\param damage Amount of damage received
	\param struckHitZone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param instigator Damage source parent (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);

		// Powerful impact should always stop engine
		if (damageContext.damageValue < GetCriticalDamageThreshold()*GetMaxHealth())
			return;

		BaseVehicleNodeComponent node = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));

		VehicleControllerComponent controller;
		if (node)
			controller = VehicleControllerComponent.Cast(node.FindComponent(VehicleControllerComponent));

		if (controller && controller.IsEngineOn())
			controller.StopEngine(false);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage state changes.
	*/
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		UpdateEngineState();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Update engine power and stop destroyed egnine according to vehicle features present.
	*/
	void UpdateEngineState()
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.GetDamageManager());
		if (damageManager)
			damageManager.UpdateVehicleState();

		// Damage and battery influence on starter and engine power output
		// TODO: Delegate this feature to controller/damage manager
		EDamageState state = GetDamageState();
		float health = GetDamageStateThreshold(state);
		float startupChance;
		if (m_Power && !m_Power.HasPower())
			startupChance = 0;
		else
			startupChance = Math.Lerp(m_fMinimumEngineStartupChance, 100, health);

		VehicleControllerComponent controller = vehicle.GetVehicleController();
		if (controller)
			controller.SetEngineStartupChance(startupChance);
	}
}
