enum EVehicleHitZoneGroup : EHitZoneGroup
{
	HULL = 10,
	ENGINE = 20,
	DRIVE_TRAIN = 30,
	FUEL_TANKS = 40,
	WHEELS = 50,
	CARGO = 60,
	AMMO_STORAGE = 70,
	TURRET = 80,
	OPTICS = 90,
	NIGHT_OPTICS = 91,
	VIEWPORT = 100,
	ROTOR_ASSEMBLY = 110,
	TAIL_ROTOR = 111,
	LANDING_GEAR = 120,
	PROPELLER = 130,
	INSTRUMENTS = 140,
	CONTROLS = 150,
	LIGHT = 160,
}

//#define VEHICLE_DAMAGE_DEBUG
//#define VEHICLE_DEBUG_OTHER
class SCR_VehicleDamageManagerComponentClass : SCR_DamageManagerComponentClass
{
	[Attribute("3.5", desc: "Max distance of hitzone, which should receive damage, from contact point.", category: "Collision Damage")]
	protected float m_fMaxSharedDamageDistance;

	[Attribute("0.7", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fFrontMultiplier;

	[Attribute("0.7", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fBottomMultiplier;

	[Attribute("1", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fRearMultiplier;

	[Attribute("1.5", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fLeftMultiplier;

	[Attribute("1.5", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fRightMultiplier;

	[Attribute("2", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fTopMultiplier;

	[Attribute("30", desc: "Speed in km/h over which will occupants be dealt damage in collision", category: "Collision Damage")]
	protected float m_fOccupantsDamageSpeedThreshold;

	[Attribute("100", desc: "Speed in km/h over which will occupants die in collision", category: "Collision Damage")]
	protected float m_fOccupantsSpeedDeath;	

	[Attribute("0 0 0", desc: "Position for frontal impact calculation", category: "Collision Damage", params: "inf inf 0 purpose=coords space=entity coordsVar=m_vFrontalImpact")]
	protected vector m_vFrontalImpact;

	[Attribute("0", desc: "Frontal impact damage needed to destroy this vehicle\nTakes into account current damage multipliers\nUse context menu on Damage Manager component to compute this value.\n[hp]", category: "Collision Damage")]
	protected float m_fVehicleDestroyDamage;

	[Attribute("15", desc: "Speed of collision that damages the vehicle\n[km/h]", category: "Collision Damage")]
	protected float m_fVehicleDamageSpeedThreshold;

	[Attribute("120", desc: "Speed of collision that destroys the vehicle\n[km/h]", category: "Collision Damage")]
	protected float m_fVehicleSpeedDestroy;
	
	[Attribute("200", desc: "Explosion damage value which, if exceeded, may cause passengers to be ejected from the vehicle", category: "Passenger Ejection")]
	protected int m_iMinExplosionEjectionDamageThreshold;
	
	[Attribute("0.5", desc: "Chance victim is ejected when explosion damage is sufficient to pass the m_iMinExplosionEjectionDamageThreshold \nLess than 0 = fully random \n1 = always", params: "-1 1 0.01", category: "Passenger Ejection")]
	protected float m_iExplosionDamageEjectionChance;
	
	[Attribute("70", desc: "Collision damage value which, if exceeded, may cause passengers to be ejected from the vehicle", category: "Passenger Ejection")]
	protected int m_iMinCollisionEjectionDamageThreshold;
	
	[Attribute("0.5", desc: "Chance victim is ejected when collisiondamage is sufficient to pass the m_iMinCollisionEjectionDamageThreshold \nLess than 0 = fully random \n1 = always", params: "-1 1 0.01", category: "Passenger Ejection")]
	protected float m_iCollisionDamageEjectionChance;

	protected float m_fDamageScaleToVehicle = float.MAX;
	protected float m_fMomentumVehicleThreshold = float.MAX;
	protected float m_fDamageScaleToCharacter = float.MAX;
	protected float m_fMomentumOccupantsThreshold = float.MAX;

	protected static const float APPROXIMATE_CHARACTER_LETHAL_DAMAGE = 150;

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxSharedDamageDistance()
	{
		return m_fMaxSharedDamageDistance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetVehicleMomentumThreshold(float ownerMass)
	{
		if (m_fMomentumVehicleThreshold != float.MAX)
			return m_fMomentumVehicleThreshold;

		m_fMomentumVehicleThreshold = ownerMass * m_fVehicleDamageSpeedThreshold * Physics.KMH2MS;

		return m_fMomentumVehicleThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetVehicleDamageScale(float ownerMass)
	{
		if (m_fDamageScaleToVehicle != float.MAX)
			return m_fDamageScaleToVehicle;

		float momentumVehicleDestroy = ownerMass * m_fVehicleSpeedDestroy * Physics.KMH2MS - GetVehicleMomentumThreshold(ownerMass);
		if (momentumVehicleDestroy == 0)
		{
			m_fDamageScaleToVehicle = 0;
			return m_fDamageScaleToVehicle;
		}

		m_fDamageScaleToVehicle = m_fVehicleDestroyDamage / momentumVehicleDestroy;

		return m_fDamageScaleToVehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetCharacterDamageScale(float ownerMass)
	{
		if (m_fDamageScaleToCharacter != float.MAX)
			return m_fDamageScaleToCharacter;

		float momentumOccupantsDeath = ownerMass * m_fOccupantsSpeedDeath * Physics.KMH2MS - GetOccupantsMomentumThreshold(ownerMass);
		if (momentumOccupantsDeath == 0)
		{
			m_fDamageScaleToCharacter = 0;
			return m_fDamageScaleToCharacter;
		}

		m_fDamageScaleToCharacter = APPROXIMATE_CHARACTER_LETHAL_DAMAGE / momentumOccupantsDeath;

		return m_fDamageScaleToCharacter;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsMomentumThreshold(float ownerMass)
	{
		if (m_fMomentumOccupantsThreshold != float.MAX)
			return m_fMomentumOccupantsThreshold;

		m_fMomentumOccupantsThreshold = ownerMass * m_fOccupantsDamageSpeedThreshold * Physics.KMH2MS;
		
		return m_fMomentumOccupantsThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetVehicleDestructionSpeed()
	{
		return m_fVehicleSpeedDestroy;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetVehicleDamageSpeedThreshold()
	{
		return m_fVehicleDamageSpeedThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetFrontMultiplier()
	{
		return m_fFrontMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBottomMultiplier()
	{
		return m_fBottomMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRearMultiplier()
	{
		return m_fRearMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLeftMultiplier()
	{
		return m_fLeftMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRightMultiplier()
	{
		return m_fRightMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTopMultiplier()
	{
		return m_fTopMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsDamageSpeedThreshold()
	{
		return m_fOccupantsDamageSpeedThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsSpeedDeath()
	{
		return m_fOccupantsSpeedDeath;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetFrontalImpact()
	{
		return m_vFrontalImpact;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinExplosionEjectionDamageThreshold()
	{
		return m_iMinExplosionEjectionDamageThreshold;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinCollisionEjectionDamageThreshold()
	{
		return m_iMinCollisionEjectionDamageThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetCollisionDamageEjectionChance()
	{
		return m_iCollisionDamageEjectionChance;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetExplosionDamageEjectionChance()
	{
		return m_iCollisionDamageEjectionChance;
	}
}

enum SCR_EPhysicsResponseIndex
{
	NORMAL = 0,
	TINY_MOMENTUM = 1,
	SMALL_MOMENTUM = 2,
	MEDIUM_MOMENTUM = 3,
	LARGE_MOMENTUM = 4,
	HUGE_MOMENTUM = 5,
	TINY_DESTRUCTIBLE = 6,
	SMALL_DESTRUCTIBLE = 7,
	MEDIUM_DESTRUCTIBLE = 8,
	LARGE_DESTRUCTIBLE = 9,
	HUGE_DESTRUCTIBLE = 10,
	NO_COLLISION = 11
}

class SCR_VehicleDamageManagerComponent : SCR_DamageManagerComponent
{
	protected const float MIN_IMPULSE_THRESHOLD = 5;
	protected const float HITZONE_DAMAGE_VALUE_THRESHOLD = 0.01;
	protected const int MIN_RESPONSE_INDEX = SCR_EPhysicsResponseIndex.TINY_MOMENTUM;
	protected const int MAX_RESPONSE_INDEX = SCR_EPhysicsResponseIndex.HUGE_MOMENTUM;
	protected const int CACHED_COLLISION_PROCESSING_DELAY = 50;
	
	protected float m_fVelocityMultiplierOnWaterEnter = 0.5;
	protected int m_fMinWaterFallDamageVelocity = 5;

	static ref map<SCR_EPhysicsResponseIndex, float> s_mResponseIndexMomentumMap = new map<SCR_EPhysicsResponseIndex, float>();

	[Attribute(defvalue: "0.7", desc: "Engine efficiency at which it is considered to be malfunctioning\n[x * 100%]", category: "Vehicle Damage")]
	protected float m_fEngineMalfunctioningThreshold;

	[Attribute(defvalue: "VehicleFireState", desc: "Vehicle parts fire state signal name", category: "Secondary damage")]
	protected string m_sVehicleFireStateSignal;

	[Attribute(defvalue: "FuelTankFireState", desc: "Fuel tank fire signal name", category: "Secondary damage")]
	protected string m_sFuelTankFireStateSignal;

	[Attribute(defvalue: "SuppliesFireState", desc: "Supplies fire signal name", category: "Secondary damage")]
	protected string m_sSuppliesFireStateSignal;

	[Attribute(defvalue: "0.02", desc: "Fuel tank fire damage rate\n[x * 100%]", params: "0 100 0.001", category: "Secondary damage")]
	protected float m_fFuelTankFireDamageRate;

	[Attribute(defvalue: "0.02", desc: "Supplies fire damage rate\n[x * 100%]", params: "0 100 0.001", category: "Secondary damage")]
	protected float m_fSuppliesFireDamageRate;

	[Attribute(defvalue: "1", desc: "Delay between secondary fire damage\n[s]", params: "0 10000 0.1", category: "Secondary damage")]
	protected float m_fSecondaryFireDamageDelay;

	protected float m_fMinImpulse;
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;

	protected static ref ScriptInvokerInt s_OnVehicleDestroyed;
	protected static ref ScriptInvoker s_OnVehicleDamageStateChanged;

	//! Common vehicle features that will influence its simulation
	protected ref array<HitZone> m_aVehicleHitZones = {};
	protected CompartmentControllerComponent m_Controller;
	protected VehicleBaseSimulation m_Simulation;

	protected float m_fEngineEfficiency = 1;
	protected bool m_bEngineFunctional = true;

	protected float m_fGearboxEfficiency = 1;
	protected bool m_bGearboxFunctional = true;

	// The vehicle damage manager needs to know about all the burning hitZones that it consists of
	protected ref array<SCR_FlammableHitZone>			m_aFlammableHitZones;
	protected SignalsManagerComponent					m_SignalsManager;

	// Vehicle fire
	protected float									m_fVehicleFireDamageTimeout;

	// Fuel tanks fire
	protected ParticleEffectEntity					m_FuelTankFireParticle; // Burning fuel particle
	protected FuelManagerComponent					m_FuelManager;
	protected float									m_fFuelTankFireDamageTimeout;

	// Supplies fire
	protected ParticleEffectEntity					m_SuppliesFireParticle; // Burning supplies particle
	protected float									m_fSuppliesFireDamageTimeout;

	// Audio features
	protected int									m_iVehicleFireStateSignalIdx;
	protected int									m_iFuelTankFireStateSignalIdx;
	protected int									m_iSuppliesFireStateSignalIdx;
	
	// Impact FX
	protected SCR_ImpactEffectComponent m_ImpactEffectComponent;

	[RplProp(onRplName: "OnVehicleFireStateChanged")]
	SCR_ESecondaryExplosionScale					m_eVehicleFireState;

	[RplProp(onRplName: "OnFuelTankFireStateChanged")]
	SCR_ESecondaryExplosionScale					m_eFuelTankFireState;

	[RplProp(onRplName: "OnSuppliesFireStateChanged")]
	SCR_ESecondaryExplosionScale					m_eSuppliesFireState;

	[RplProp()]
	protected vector								m_vVehicleFireOrigin;

	[RplProp()]
	protected vector								m_vFuelTankFireOrigin;

	[RplProp()]
	protected vector								m_vSuppliesFireOrigin;

#ifdef VEHICLE_DAMAGE_DEBUG
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif
#ifdef VEHICLE_DEBUG_OTHER
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif

	protected ref array<ref SCR_CollisionDamageContainer> m_LastFrameCollisions;
	protected ref array<ref SCR_CollisionDamageContainer> m_CurrentCollisions;
#ifdef ENABLE_DIAG
	protected ref SCR_DebugShapeManager m_DebugShape = new SCR_DebugShapeManager();
	protected ref array<ref Tuple2<vector, bool>> m_aLocalCollisions = {};
#endif

#ifdef WORKBENCH
	//! The reason for this magic number is that the collision position can be slightly unpredictable.
	//! Because of that, what may end up happening is that the distribution of the damage won't have enough hit zone health to absorb the impact,
	//! which in turn means that more damage will be passed to the hull hit zone, resulting in a vehicle explosion at a lower speed.
	//! Thus, the precomputed damage is reduced by 10% to accommodate this bit of randomness.
	protected const float HACK_DAMAGE_CALCULATION_REDUCTION = 0.9;
#endif

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_VehicleDamageManagerComponentClass GetPrefabData()
	{
		return SCR_VehicleDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTopMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 2;

		return prefabData.GetTopMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRightMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetRightMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLeftMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetLeftMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRearMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1;

		return prefabData.GetRearMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBottomMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.7;

		return prefabData.GetBottomMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetFrontMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.7;

		return prefabData.GetFrontMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxSharedDamageDistance()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetMaxSharedDamageDistance();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsDamageSpeedThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 30;

		return prefabData.GetOccupantsDamageSpeedThreshold();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsSpeedDeath()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 80;

		return prefabData.GetOccupantsSpeedDeath();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinCollisionDamageEjectionThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 70;

		return prefabData.GetMinCollisionEjectionDamageThreshold();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinExplosionDamageEjectionThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 200;

		return prefabData.GetMinExplosionEjectionDamageThreshold();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetCollisionDamageEjectionChance()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.5;

		return prefabData.GetCollisionDamageEjectionChance();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetExplosionDamageEjectionChance()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.5;

		return prefabData.GetExplosionDamageEjectionChance();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetFrontalImpact()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return "0 1 1";

		return prefabData.GetFrontalImpact();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] side
	//! \return
	float GetSideDamageMultiplier(SCR_EBoxSide side)
	{
		switch (side)
		{
			case SCR_EBoxSide.FRONT:
				return GetFrontMultiplier();
			case SCR_EBoxSide.REAR:
				return GetRearMultiplier();
			case SCR_EBoxSide.BOTTOM:
				return GetBottomMultiplier();
			case SCR_EBoxSide.TOP:
				return GetTopMultiplier();
			case SCR_EBoxSide.LEFT:
				return GetLeftMultiplier();
			case SCR_EBoxSide.RIGHT:
				return GetRightMultiplier();

		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		Vehicle vehicle = Vehicle.Cast(owner);
		if (!vehicle)
			return;

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		m_Controller = CompartmentControllerComponent.Cast(owner.FindComponent(CompartmentControllerComponent));
		m_Simulation = VehicleBaseSimulation.Cast(owner.FindComponent(VehicleBaseSimulation));
		m_FuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
		m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_ImpactEffectComponent = SCR_ImpactEffectComponent.Cast(owner.FindComponent(SCR_ImpactEffectComponent));

		// can happen when any physics or simulation property is invalid
		if (!m_Simulation)
		{
			m_fMinImpulse = 0;
			return;
		}

		if (m_SignalsManager)
		{
			m_iVehicleFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sVehicleFireStateSignal);
			m_iFuelTankFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sFuelTankFireStateSignal);
			m_iSuppliesFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sSuppliesFireStateSignal);
		}
		
		SCR_VehicleBuoyancyComponent vehicleBuoyancy = SCR_VehicleBuoyancyComponent.Cast(vehicle.FindComponent(SCR_VehicleBuoyancyComponent));
		if (vehicleBuoyancy)
			vehicleBuoyancy.GetOnWaterEnter().Insert(OnWaterEnter);

		UpdateVehicleState();

		RplComponent rpl = vehicle.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;

		Physics physics = owner.GetPhysics();
		if (physics)
			m_fMinImpulse = physics.GetMass() * 2;

		if (m_fMinImpulse > 0)
			SetEventMask(owner, EntityEvent.CONTACT);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	// Register simulation feature hit zone
	void RegisterVehicleHitZone(notnull HitZone hitZone)
	{
		// Remove the hitzone if it was already registered
		if (!m_aVehicleHitZones.Contains(hitZone))
			m_aVehicleHitZones.Insert(hitZone);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	// Register simulation feature hit zone
	void UnregisterVehicleHitZone(HitZone hitZone)
	{
		m_aVehicleHitZones.RemoveItem(hitZone);
	}

	//------------------------------------------------------------------------------------------------
	//! Compute current simulation state of vehicle
	//! Called when hit zone damage states change
	void UpdateVehicleState()
	{
		int engineCount;
		float engineEfficiency;
		bool engineFunctional;

		int gearboxCount;
		float gearboxEfficiency;
		bool gearboxFunctional;

		SCR_EngineHitZone engineHitZone;
		SCR_GearboxHitZone gearboxHitZone;

		foreach (HitZone hitZone : m_aVehicleHitZones)
		{
			engineHitZone = SCR_EngineHitZone.Cast(hitZone);
			if (engineHitZone)
			{
				engineCount++;
				engineEfficiency += engineHitZone.GetEfficiency();

				if (engineHitZone.GetDamageState() != EDamageState.DESTROYED)
					engineFunctional = true;

				continue;
			}

			gearboxHitZone = SCR_GearboxHitZone.Cast(hitZone);
			if (gearboxHitZone)
			{
				gearboxCount++;
				gearboxEfficiency += gearboxHitZone.GetEfficiency();

				if (gearboxHitZone.GetDamageState() != EDamageState.DESTROYED)
					gearboxFunctional = true;

				continue;
			}
		}

		if (engineCount > 0)
		{
			SetEngineFunctional(engineFunctional);
			SetEngineEfficiency(engineEfficiency / engineCount);
		}

		if (gearboxCount > 0)
		{
			SetGearboxFunctional(gearboxFunctional);
			SetGearboxEfficiency(gearboxEfficiency / gearboxCount);
		}

		UpdateMovementDamage();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateMovementDamage()
	{
		float movementDamage;

		if (!m_bEngineFunctional)
			movementDamage = 1;
		else
			movementDamage = 1 - (m_fGearboxEfficiency * m_fEngineEfficiency);

		SetMovementDamage(movementDamage);
		
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller)
			controller.SetCanMove(movementDamage < 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetEngineFunctional()
	{
		return m_bEngineFunctional;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEngineFunctional(bool functional)
	{
		m_bEngineFunctional = functional;

		if (functional)
			return;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller && controller.IsEngineOn())
			controller.StopEngine(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetEngineMalfunctionThreshold()
	{
		return m_fEngineMalfunctioningThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetEngineEfficiency()
	{
		return m_fEngineEfficiency;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEngineEfficiency(float efficiency)
	{
		m_fEngineEfficiency = efficiency;

		VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(m_Simulation);
		if (simulation && simulation.IsValid())
		{
			simulation.EngineSetPeakTorqueState(efficiency * simulation.EngineGetPeakTorque());
			simulation.EngineSetPeakPowerState(efficiency * simulation.EngineGetPeakPower());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetGearboxFunctional(bool functional)
	{
		m_bGearboxFunctional = functional;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller)
			controller.SetCanMove(functional);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetGearboxFunctional()
	{
		return m_bGearboxFunctional;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetGearboxEfficiency()
	{
		return m_fGearboxEfficiency;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetGearboxEfficiency(float efficiency)
	{
		m_fGearboxEfficiency = efficiency;

		VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(m_Simulation);
		if (simulation && simulation.IsValid())
			simulation.GearboxSetEfficiencyState(efficiency * simulation.GearboxGetEfficiency());
	}

	//------------------------------------------------------------------------------------------------
	//! Finds compatible hit zones
	//! \param[in] position in world space
	//! \param[in] physics
	//! \param[in] maxDistance distance form the provided position up to which hit zones searched for
	//! \param[in] damageType fitler that ensures that hit zones can accept provided damage type
	//! \param[out] outHitZones
	//! \param[out] outHitZoneDistanceProcentageMap map of hit zones and how far they are from the provided position
	//! \param[out] hitZonesDistancePercentSum
	//! \return number of found hit zones
	int GetSurroundingHitzones(vector position, notnull Physics physics, float maxDistance, EDamageType damageType, out notnull array<HitZone> outHitZones, out float hitZonesDistancePercentSum = 0, out map<HitZone, float> outHitZoneDistanceProcentageMap = null)
	{
		outHitZones.Clear();
		array<HitZone> allHitZones = {};
		int count = GetAllHitZonesInHierarchy(allHitZones);
		if (count <= 0)
			return 0;

		outHitZoneDistanceProcentageMap = new map<HitZone, float>();
		array<int> hitZoneColliderIDs = {};
		float distancePercent;
		float currentDistance;
		float maxDistanceSq = maxDistance * maxDistance;
		float minDistance;
		HitZoneContainerComponent hitZoneContainer;
		IEntity hitZoneParentEntity;
		vector mins, maxs;
		vector center;
		// Find hit zones that qualify for this damage
		foreach (HitZone hitZone : allHitZones)
		{
			minDistance = float.MAX;
			if (hitZone.GetDamageState() == EDamageState.DESTROYED)
				continue;

			if (hitZone.GetBaseDamageMultiplier() == 0)
				continue;

			if (hitZone.GetDamageMultiplier(damageType) == 0)
				continue;

			if (hitZone.GetColliderIDs(hitZoneColliderIDs) == 0)
			{
				hitZoneContainer = hitZone.GetHitZoneContainer();
				if (!hitZoneContainer || hitZoneContainer == this)
					continue;

				hitZoneParentEntity = hitZoneContainer.GetOwner();
				hitZoneParentEntity.GetBounds(mins, maxs);

				center = mins + (maxs - mins) * 0.5;
				minDistance = vector.DistanceSq(position, hitZoneParentEntity.CoordToParent(center));
			}
			else
			{
				foreach (int id : hitZoneColliderIDs)
				{
					currentDistance = vector.DistanceSq(position, physics.GetGeomWorldPosition(id));

					if (currentDistance < minDistance)
						minDistance = currentDistance;
				}
			}

			if (minDistance > maxDistanceSq)
				continue;

			minDistance = Math.Sqrt(minDistance);
			distancePercent = 1 - minDistance / maxDistance;

			hitZonesDistancePercentSum += distancePercent;
			outHitZoneDistanceProcentageMap.Insert(hitZone, distancePercent);
			outHitZones.Insert(hitZone);
		}

		return outHitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Distributes provided damage amongst compatible hit zones
	//! \param[in] position
	//! \param[in] damage
	//! \param[in] damageType
	//! \param[in] dontApply used for debugging the damage handling without actually passing the damage
	//! \return leftover damage that wasnt absorbed by compatible hit zones
	float DamageSurroundingHitzones(vector position, float damage, EDamageType damageType, bool dontApply = false)
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return damage;

		array<HitZone> hitZones = {};
		map<HitZone, float> hitZoneDistancePercentMap;
		float hitZonesDistancePercentSum;
		int hitZonesCount = GetSurroundingHitzones(position, physics, GetMaxSharedDamageDistance(), damageType, hitZones, hitZonesDistancePercentSum, hitZoneDistancePercentMap);
		if (hitZonesCount <= 0)
			return damage;

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) > 0)
			Print("		Damage to absorb = " + damage + " of type = " + SCR_Enum.GetEnumName(EDamageType, damageType), LogLevel.NORMAL);

		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) > 1)
			Print("		Number of hit zones to which damage will be passed: " + hitZonesCount, LogLevel.NORMAL);
#endif
		float leftoverDamage = damage;
		float currentDamage;
		float currentDamagePercent;
		float hitZoneHealth;
		float damageMultiplier;

		DamageManagerComponent damageManager;
		vector empty[3];
		empty[0] = vector.Zero;
		empty[1] = vector.Zero;
		empty[2] = vector.Zero;
		foreach (HitZone hitZone, float distancePercent : hitZoneDistancePercentMap)
		{
			currentDamagePercent = distancePercent / hitZonesDistancePercentSum;
			currentDamage = damage * currentDamagePercent;

			hitZoneHealth = hitZone.GetHealth();

#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) > 1)
				Print("			hit zone name = " + hitZone.GetName(), LogLevel.NORMAL);

			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) > 2)
			{
				Print("			hit zone health = " + hitZoneHealth, LogLevel.NORMAL);
				Print("			distance percent from the impact point = " + distancePercent, LogLevel.NORMAL);
				Print("			percentage of the vehicle damage that can be transfered to this hit zone = " + currentDamagePercent + " = " + distancePercent + " / " + hitZonesDistancePercentSum, LogLevel.NORMAL);
				Print("			damage that can be done to this hit zone = " + currentDamage + " = " + damage + " * " + currentDamagePercent, LogLevel.NORMAL);
			}
#endif

			damageMultiplier = hitZone.GetDamageMultiplier(damageType) * hitZone.GetBaseDamageMultiplier();
			if (damageMultiplier != 0)
			{
#ifdef ENABLE_DIAG
				if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) > 2)
					Print("				clamped damage = " + (Math.Clamp(currentDamage, 0, (hitZoneHealth + hitZone.GetDamageReduction()) / damageMultiplier)) + " = Clamp(" + currentDamage + ", 0, (" + hitZoneHealth + " + " + hitZone.GetDamageReduction() + ") / " + damageMultiplier + ")", LogLevel.NORMAL);
#endif
				currentDamage = Math.Clamp(currentDamage, 0, (hitZoneHealth + hitZone.GetDamageReduction()) / damageMultiplier);
			}

			if (float.AlmostEqual(currentDamage, 0, HITZONE_DAMAGE_VALUE_THRESHOLD))
				continue;

			damageManager = DamageManagerComponent.Cast(hitZone.GetHitZoneContainer());
			if (!damageManager)
				continue;

			SCR_DamageContext damageContext = new SCR_DamageContext(damageType, currentDamage, empty, GetOwner(), hitZone, null, null, -1, -1);
			if (!dontApply)
				damageManager.HandleDamage(damageContext);

			leftoverDamage -= currentDamage;
#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) > 1)
			{
				Print("			damage done to the hit zone = " + currentDamage, LogLevel.NORMAL);
				Print("			-----------------------------", LogLevel.NORMAL);
			}
#endif
		}

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
			Print("		damage absorbed by hit zones = " + (damage - leftoverDamage), LogLevel.NORMAL);
#endif
		leftoverDamage = Math.Clamp(leftoverDamage, 0, float.MAX);
		
#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
		{
			Print("		leftoverDamage returned to the hull = " + leftoverDamage, LogLevel.NORMAL);
			Print("	=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=", LogLevel.NORMAL);
		}
#endif
		return leftoverDamage;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] position
	//! \return
	// TODO: This won't work properly for all the types of objects - E.G. flat boats with huge towers on them
	// e.g x = collision point, this will say it got hit from the back, because it's inside the rear-side pyramid
	//       ||
	// __x___||_______
	// \             /
	//  \           /
	// /TODO
	// To trigger this, just crash a vehicle into something
	// Make sure the vehicle has SCR_VehicleDamageManagerComponent with attribute "Print Relative Force" set to true.
	SCR_EBoxSide GetHitDirection(vector position)
	{
		IEntity owner = GetOwner();
		//Get local space contact position
		vector contact = owner.CoordToLocal(position);

		//Get local space mins and maxs
		vector mins, maxs;
		owner.GetBounds(mins, maxs);

		//Calculate bbox center
		vector bboxCenter = (maxs - mins) * 0.5 + mins;

		//Get contact position relative to the bbox center, for correct dot product calculations later
		contact = contact - bboxCenter;

		//Here we calculate all the normals of our 6 planes as well as their distances, but these broke the code completely, so I commented them out.
		//Naming: XZA = plane A perpendicular to the X-Z plane and so on...
		vector normalXZA, normalXZB, normalYZA, normalYZB, normalXYA, normalXYB;
		float distanceXZA, distanceXZB, distanceYZA, distanceYZB, distanceXYA, distanceXYB;

		normalYZA = Vector(0, mins[2] - maxs[2], maxs[1] - mins[1]).Normalized();
		distanceYZA = vector.Dot(normalYZA, Vector(0, mins[1], mins[2]));

		normalYZB = Vector(0, mins[2] - maxs[2], mins[1] - maxs[1]).Normalized();
		distanceYZB = vector.Dot(normalYZB, Vector(0, mins[1], mins[2]));

		normalXZA = Vector(maxs[2] - mins[2], 0, mins[0] - maxs[0]).Normalized();
		distanceXZA = vector.Dot(normalXZA, Vector(mins[0], 0, mins[2]));

		normalXZB = Vector(mins[2] - maxs[2], 0, mins[0] - maxs[0]).Normalized();
		distanceXZB = vector.Dot(normalXZB, Vector(mins[0], 0, mins[2]));

		normalXYA = Vector(maxs[1] - mins[1], mins[0] - maxs[0], 0).Normalized();
		distanceXYA = vector.Dot(normalXYA, Vector(mins[0], mins[1], 0));

		normalXYB = Vector(mins[1] - maxs[1], mins[0] - maxs[0], 0).Normalized();
		distanceXYB = vector.Dot(normalXYB, Vector(mins[0], mins[1], 0));

#ifdef VEHICLE_DAMAGE_DEBUG
		//Debug shapes
		//Here we just draw all the fancy stuff
		m_aDebugShapes.Clear();

		m_aDebugShapes.Insert(Shape.Create(ShapeType.BBOX, ARGB(255, 0, 255, 255), ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, owner.CoordToParent(mins), owner.CoordToParent(maxs)));
		m_aDebugShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, position, 0.2));

		vector p[2];
		p[0] = owner.CoordToParent(bboxCenter);
		p[1] = owner.CoordToParent(normalXZA);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalXZB);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalYZA);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalYZB);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 255), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalXYA);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 255), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalXYB);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(contact);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 155, 255), ShapeFlags.NOZBUFFER, p, 2));
#endif
#ifdef VEHICLE_DEBUG_OTHER
		//This is a temporary solution, TODO: PROPER DEBUG
		//Do not remove the commented scripts!
		m_aDebugShapes.Clear();
		vector p[2];

		vector globalMins, globalMaxs;
		globalMins = owner.CoordToParent(mins);
		globalMaxs = owner.CoordToParent(maxs);

		m_aDebugShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, position, 0.2));

		/*p[0] = globalMins;
		p[1] = globalMaxs;
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0][0] = globalMaxs[0];
		p[1][0] = globalMins[0];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMins;
		p[0][1] = globalMaxs[1];
		p[1] = globalMaxs;
		p[1][1] = globalMins[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMins;
		p[0][2] = globalMaxs[2];
		p[1] = globalMaxs;
		p[1][2] = globalMins[2];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2)); */

		//Corner lines
		p[0] = globalMins;
		p[1] = p[0];
		p[1][1] = globalMaxs[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMins;
		p[0][0] = globalMaxs[0];
		p[1] = p[0];
		p[1][1] = globalMaxs[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMaxs;
		p[1] = p[0];
		p[1][1] = globalMins[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMaxs;
		p[0][0] = globalMins[0];
		p[1] = p[0];
		p[1][1] = globalMins[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		//Top pyramid
		p[0] = mins;
		p[0][1] = maxs[1] * 0.5;
		p[1] = maxs;
		Shape shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[1], p[0]);
		m_aDebugShapes.Insert(shape);
		mat[3] = owner.GetOrigin();
		shape.SetMatrix(mat);

		//Bottom pyramid
		p[0] = mins;
		p[1] = maxs;
		p[1][1] = p[1][1] * 0.5;
		shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[0], p[1]);
		m_aDebugShapes.Insert(shape);
		shape.SetMatrix(mat);

		//Right pyramid
		/*p[0][0] = maxs[1];
		p[0][1] = mins[0];
		p[0][2] = mins[2];
		p[1][0] = mins[1];
		p[1][1] = maxs[0];
		p[1][2] = maxs[2];
		p[0] = maxs;
		p[1] = mins;
		p[1][0] = mins[0]; */
		/*shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[1], p[0]);
		m_aDebugShapes.Insert(shape);
		mat[0] = -vector.Up;
		mat[1] = vector.Right;
		mat[2] = vector.Forward;
		mat[3] = owner.GetOrigin() + maxs;
		shape.SetMatrix(mat);
		Math3D.MatrixIdentity4(mat); */

		/*mat[0] = vector.Up;
		mat[1] = -vector.Right;
		mat[2] = vector.Forward; */
#endif

		//Calculating where the contact point is relative to the plane by calculating the dot product of the contact point and the planes normal
		bool XZA, XZB, YZA, YZB, XYA, XYB;
		XZA = (vector.Dot(contact, normalXZA) > 0) - distanceXZA;
		XZB = (vector.Dot(contact, normalXZB) > 0) - distanceXZB;
		YZA = (vector.Dot(contact, normalYZA) > 0) - distanceYZA;
		YZB = (vector.Dot(contact, normalYZB) > 0) - distanceYZB;
		XYA = (vector.Dot(contact, normalXYA) > 0) - distanceXYA;
		XYB = (vector.Dot(contact, normalXYB) > 0) - distanceXYB;

		//XZA = Front, right, bottom, top
		//XZB = Rear, right, bottom, top
		//YZA = Front, bottom, right, left
		//YZB = Rear, bottom, right, left
		//XYA = Front, Rear, top, left
		//XYB = Front, Rear, top, right
		//			XZA	XZB	YZA	YZB	XYA	XYB
		//Top	=	 - 	 -	 0	 0	 0	 0
		//Bottom=	 -	 -	 1	 1	 1	 1
		//Left	=	 0	 1	 -	 -	 0	 1
		//Right	=	 1	 0	 -	 -	 1	 0
		//Front =	 0	 0	 1	 0	 -	 -
		//Rear	=	 1	 1 	 0	 1	 -	 -

		//Right
		if (XZA && !XZB && XYA && !XYB)
			return SCR_EBoxSide.RIGHT;

		//Left
		if (!XZA && XZB && !XYA && XYB)
			return SCR_EBoxSide.LEFT;

		//Bottom
		if (YZA && YZB && XYA && XYB)
			return SCR_EBoxSide.BOTTOM;

		//Top
		if (!YZA && !YZB && !XYA && !XYB)
			return SCR_EBoxSide.TOP;

		//Front
		if (!XZA && !XZB && YZA && !YZB)
			return SCR_EBoxSide.FRONT;

		//Rear
		if (XZA && XZB && !YZA && YZB)
			return SCR_EBoxSide.REAR;

		return SCR_EBoxSide.FRONT;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Determines the amount of damage required to destroy the vehicle in case of a frontal collision
	float CalculateCriticalCollisionDamage()
	{
		IEntity owner = GetOwner();
		array<HitZone> hitZones = {};
		vector pointOfImpact = owner.CoordToParent(GetFrontalImpact());
		EDamageType damageType = EDamageType.COLLISION;
		float procentageSum;
		int count = GetSurroundingHitzones(pointOfImpact, owner.GetPhysics(), GetMaxSharedDamageDistance(), damageType, hitZones, procentageSum);
		if (count == 0)
			return 0;

		float damage;
		HitZone defaultHitZone = GetDefaultHitZone();

		// Find the total amount of damage required to destroy hit zones that will be damaged in case of a frontal collision
		damage = GetMinDestroyDamage(damageType, hitZones);
		damage = damage * procentageSum / count;

		float defaultHitZoneDamageMultipliers = defaultHitZone.GetBaseDamageMultiplier() * defaultHitZone.GetDamageMultiplier(damageType);
		if (defaultHitZoneDamageMultipliers == 0)
			return 0;

		float defaultHitZoneMaxDamage = (defaultHitZone.GetMaxHealth() + defaultHitZone.GetDamageReduction()) / defaultHitZoneDamageMultipliers;
		if (defaultHitZoneMaxDamage < defaultHitZone.GetDamageThreshold())
			defaultHitZoneMaxDamage += defaultHitZone.GetDamageThreshold();

		if (defaultHitZoneMaxDamage == 0)
			return 0;

		// Find how much of excessive damage will be returned to the deafult hit zone in case of an actuall collision
		float excessDamage = DamageSurroundingHitzones(pointOfImpact, damage, damageType, true);

		int securityCounter = 20;//prevent infinite loop at all cost to ensure that it wont cause a loss of someones unsaved data
		float prc;
		while (damage > 0 && !float.AlmostEqual(excessDamage, defaultHitZoneMaxDamage, 0.1))
		{
			if (excessDamage == 0)
				return 0;

			if (excessDamage < defaultHitZoneMaxDamage)
				prc = -defaultHitZoneMaxDamage / excessDamage;
			else
				prc = 1 - defaultHitZoneMaxDamage / excessDamage;
					
			damage = damage - excessDamage * prc;
			excessDamage = DamageSurroundingHitzones(pointOfImpact, damage, damageType, true);

			securityCounter--;
			if (securityCounter < 0)
				return 0;
		}

		damage = damage / GetFrontMultiplier() * HACK_DAMAGE_CALCULATION_REDUCTION;//magic number ;_;
		// The reason for this magic number is that the collision position can be slightly unpredictable.
		// Because of that, what may end up happening is that the distribution of the damage won't have enough hit zone health to absorb the impact,
		// which in turn means that more damage will be passed to the hull hit zone, resulting in a vehicle explosion at a lower speed.
		// Thus, the precomputed damage is reduced by 10% to accommodate this bit of randomness.

		// Damage is not valid - warn the user!
		if (damage < 0)
		{
			Print("Cannot destroy selected vehicle on collision", LogLevel.WARNING);
			return 0;
		}

		return Math.Ceil(damage);
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Determine whether collision into water is severe enough to apply damage, then get velocities an manually call CollisionDamage
	protected void OnWaterEnter()
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		// Don't try to apply waterFallDamage when going less than X m/s
		float impactSpeed = Math.AbsFloat(physics.GetVelocity()[1]);
		if (impactSpeed < m_fMinWaterFallDamageVelocity)
			return;
		
		vector ownerTransform = GetOwner().GetOrigin();
		vector velocityBefore = Vector(0, GetOwner().GetPhysics().GetVelocity()[1], 0);
		vector velocityAfter = velocityBefore * m_fVelocityMultiplierOnWaterEnter;
		ProcessCollision(new SCR_CollisionDamageContainer(GetOwner(), GetGame().GetWorldEntity().GetTerrain(0, 0), 0, velocityBefore, velocityAfter, vector.Zero, vector.Zero, ownerTransform, vector.Zero));
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	protected void DebugSphereUpdate()
	{
		m_DebugShape.Clear();
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) == 0)
		{
			m_aLocalCollisions.Clear();
			GetGame().GetCallqueue().Remove(DebugSphereUpdate);
			return;
		}

		IEntity owner = GetOwner();
		foreach (Tuple2<vector, bool> entry : m_aLocalCollisions)
		{
			if (entry.param2)
				m_DebugShape.AddSphere(owner.CoordToParent(entry.param1), 0.25, Color.GREEN, ShapeFlags.WIREFRAME | ShapeFlags.ONCE);
			else
				m_DebugShape.AddSphere(owner.CoordToParent(entry.param1), 0.3, Color.RED, ShapeFlags.WIREFRAME | ShapeFlags.ONCE);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Filter out any contacts under a reasonable speed for damage
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		// Returning when impulse is less than 5 doesn't impact gameplay at all, but saves a lot of OnFilteredContact calls
		if (contact.Impulse < MIN_IMPULSE_THRESHOLD)
			return false;
		
		return super.FilterContact(owner, other, contact);;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle damage and collision effects of contact
	//! Contact must NEVER be passed as variable to external functions
	override protected void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_ImpactEffectComponent && other)
			m_ImpactEffectComponent.OnImpact(other, contact.Impulse, contact.Position, contact.Normal, contact.Material2, contact.VelocityBefore1, contact.VelocityAfter1);

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (rplComp.IsProxy()) // skip null check as damage manager cannot exist without rpl component
			return;// Server will deal the damage thus there is no point for proxy to process this data

		if (contact.Impulse < m_fMinImpulse)
			return;

		// Since data will be processed after some time then position has to be held in local space
		// This needs to be done in order to grab hit zones relevant to the impact position
		vector localPosition = owner.CoordToLocal(contact.Position);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
		{
			m_DebugShape.AddSphere(contact.Position, 0.3, Color.RED, ShapeFlags.WIREFRAME);
			if (m_aLocalCollisions.IsEmpty())
				GetGame().GetCallqueue().CallLater(DebugSphereUpdate, 0, true);

			m_aLocalCollisions.Insert(new Tuple2<vector, bool>(localPosition, false));
			Print("	Collision detected between owner = " + owner.GetID() + " and other = " + other.GetID() + " at position = " + contact.Position, LogLevel.NORMAL);
			Print("		Owner Colliding surfaces = " + contact.Material1 + " | other = " +  contact.Material2, LogLevel.NORMAL);
			Print("		Owner shape indencies = " + contact.ShapeIndex1 + " | other = " + contact.ShapeIndex2, LogLevel.NORMAL);
			Print("		Impuls = " + contact.Impulse);
			float dotMultiplier = vector.Dot(contact.VelocityAfter1.Normalized(), contact.VelocityBefore1.Normalized());
			vector velocityDiff = contact.VelocityBefore1 - contact.VelocityAfter1 * dotMultiplier;
			float impactSpeedDiff = Math.AbsFloat(velocityDiff.Length());
			Print("		Owner impact speed diff = " + impactSpeedDiff + "m/s (" + impactSpeedDiff * Physics.MS2KMH + "kph)", LogLevel.NORMAL);
			dotMultiplier = vector.Dot(contact.VelocityAfter2.Normalized(), contact.VelocityBefore2.Normalized());
			velocityDiff = contact.VelocityBefore2 - contact.VelocityAfter2 * dotMultiplier;
			impactSpeedDiff = Math.AbsFloat(velocityDiff.Length());
			Print("		Other impact speed diff = " + impactSpeedDiff + "m/s (" + impactSpeedDiff * Physics.MS2KMH + "kph)", LogLevel.NORMAL);
		}
#endif

		if (!m_CurrentCollisions)
		{
			m_CurrentCollisions = new array<ref SCR_CollisionDamageContainer>();
			GetGame().GetCallqueue().CallLater(EnableDamageSystemOnFrame);
		}

		SCR_CollisionDamageContainer existingCollisionInstance;
		foreach (SCR_CollisionDamageContainer collision : m_CurrentCollisions)
		{
			if (collision.m_Other != other)
				continue;

			existingCollisionInstance = collision;
			break;
		}

		if (!existingCollisionInstance)
		{
#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
				Print("	No previous collisions cached for contact between " + owner.GetID() + " and " + other.GetID() + ". Caching this collision data!", LogLevel.NORMAL);
#endif
			m_CurrentCollisions.Insert(new SCR_CollisionDamageContainer(owner, other, contact.Impulse, contact.VelocityBefore1, contact.VelocityAfter1, contact.VelocityBefore2, contact.VelocityAfter2, localPosition, contact.Normal));
			return;
		}

		existingCollisionInstance.UpdateImpactPosition(localPosition);
		if (contact.Impulse < existingCollisionInstance.m_fImpulse)
		{
#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
				Print("	Detected collision has smaller impuls - discarding!", LogLevel.NORMAL);
#endif
			return;
		}

		float mass;
		if (contact.Physics1)
			mass = contact.Physics1.GetMass();

		float dotMultiplier = vector.Dot(contact.VelocityAfter1.Normalized(), contact.VelocityBefore1.Normalized());
		float momentumB = contact.VelocityBefore1.Length() * mass;
		float momentumA = contact.VelocityAfter1.Length() * mass * dotMultiplier;
		float totalMomentumOwner = Math.AbsFloat(momentumB - momentumA * dotMultiplier);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
		{
			Print("		owner momentum before = " + momentumB + " | momentum after = " + momentumA, LogLevel.NORMAL);
			Print("		owner dotMultiplier = " + dotMultiplier, LogLevel.NORMAL);
		}
#endif

		if (totalMomentumOwner > existingCollisionInstance.m_fTotalMomentumOwner)
			existingCollisionInstance.m_fTotalMomentumOwner = totalMomentumOwner;

		if (contact.Physics2)
			mass = contact.Physics2.GetMass();
		else
			mass = 0;

		dotMultiplier = vector.Dot(contact.VelocityAfter2.Normalized(), contact.VelocityBefore2.Normalized());
		momentumB = contact.VelocityBefore2.Length() * mass;
		momentumA = contact.VelocityAfter2.Length() * mass * dotMultiplier;
		float totalMomentumOther = Math.AbsFloat(momentumB - momentumA);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
		{
			Print("		total owner momentum = " + totalMomentumOwner + " while previous was = " + existingCollisionInstance.m_fTotalMomentumOwner, LogLevel.NORMAL);
			Print("		other momentum before = " + momentumB + " | momentum after = " + momentumA, LogLevel.NORMAL);
			Print("		other dotMultiplier = " + dotMultiplier, LogLevel.NORMAL);
			Print("		total other momentum= " + totalMomentumOther + " while previous was = " + existingCollisionInstance.m_fTotalMomentumOther, LogLevel.NORMAL);
			Print("	Detected collision has larger impuls!", LogLevel.NORMAL);
		}
#endif

		if (totalMomentumOther > existingCollisionInstance.m_fTotalMomentumOther)
			existingCollisionInstance.m_fTotalMomentumOther = totalMomentumOther;

		existingCollisionInstance.m_fImpulse = contact.Impulse;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);

		if (!m_CurrentCollisions || m_CurrentCollisions.IsEmpty())
		{
			if (m_LastFrameCollisions && !m_LastFrameCollisions.IsEmpty())
				m_LastFrameCollisions.Clear();

			DisableDamageSystemOnFrame();
			return;
		}

		bool continuedCollision;
		foreach (SCR_CollisionDamageContainer collision : m_CurrentCollisions)
		{
			if (!m_LastFrameCollisions)
			{
				ProcessCollision(collision);
				continue;
			}

			continuedCollision = false;
			foreach (SCR_CollisionDamageContainer oldCollision : m_LastFrameCollisions)
			{
				if (collision.m_Other == oldCollision.m_Other)
				{
					continuedCollision = true;
					break;
				}
			}

			if (continuedCollision)
				continue;

			ProcessCollision(collision);
		}

		m_LastFrameCollisions = m_CurrentCollisions;
		m_CurrentCollisions = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Processing of cached collision data in order to deal damage to the vhehicle and its passengers
	void ProcessCollision(notnull SCR_CollisionDamageContainer collision)
	{
		IEntity owner = GetOwner();
		// This data can be moved back to component
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return;

		// Get the physics of the dynamic object (if owner is static we ignore the collision)
		Physics ownerPhysics = owner.GetPhysics();
		if (!ownerPhysics.IsDynamic())
			return;

		vector impactPosition = owner.CoordToParent(collision.GetAverageImpactPosition());

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
		{
			m_DebugShape.AddSphere(impactPosition, 0.2, Color.GREEN, ShapeFlags.WIREFRAME);
			m_aLocalCollisions.Insert(new Tuple2<vector, bool>(collision.GetAverageImpactPosition(), true));
			Print("		Processing collision damage at the position = " + impactPosition + " - " + GetOwner().GetID(), LogLevel.NORMAL);
			Print("		Cached owner speed before collision = " + collision.GetOwnerVelocityBefore().Length() * Physics.MS2KMH + "kph)", LogLevel.NORMAL);
			Print("		Cached other speed before collision = " + collision.GetOtherVelocityBefore().Length() * Physics.MS2KMH + "kph)", LogLevel.NORMAL);
		}
#endif
		int ownerResponseIndex = ownerPhysics.GetResponseIndex();
		// We hit a destructible that will break, static object -> deal no damage to vehicle or occupants
		if (!collision.m_bOtherIsDynamic && collision.m_bOtherIsDestructible && collision.m_iOtherResponseIndex - MIN_DESTRUCTION_RESPONSE_INDEX <= ownerResponseIndex - MIN_MOMENTUM_RESPONSE_INDEX)
			return;

#ifdef DISABLE_VEHICLE_COLLISION_DAMAGE
		return;
#endif

		float ownerMass = ownerPhysics.GetMass();
		float totalMomentum = collision.m_fTotalMomentumOwner + collision.m_fTotalMomentumOther;
		IEntity instigatorEntity;
		// Find compartment manager to damage occupants
		if (m_CompartmentManager)
		{
			// This is the momentum, which will be transferred to damage
			float momentumOverOccupantsThreshold = totalMomentum * collision.m_fDamageShare - prefabData.GetOccupantsMomentumThreshold(ownerMass);

			// Deal damage if it's more that 0
			if (momentumOverOccupantsThreshold > 0)
			{
				//If the entity is dynamic, we need to look if the entity has a driver or not.
				//If the vehicle's speed is bigger than m_fVehicleDamageSpeedThreshold
				if (collision.m_bOtherIsDynamic && collision.GetOwnerVelocityBefore().LengthSq() > prefabData.GetVehicleDamageSpeedThreshold() * prefabData.GetVehicleDamageSpeedThreshold())
				{
					//If the entity has a driver:
					if (collision.m_OtherPilot)
					{
						//If their speed is bigger than GetVehicleDamageSpeedThreshold and they are not running away from the vehicle
						//(their dot product indicates that they are within 60° cone):
						//their driver is the instigator of the damage the occupants of this vehicle's compartments receive.
						//If not, the driver of this vehicle is the instigator.
						vector directionToOther = (collision.m_vOtherOriginAtThePointOfImpact - collision.m_vOwnerOriginAtThePointOfImpact).Normalized();
						if (vector.Dot(collision.GetOwnerVelocityBefore().Normalized(), directionToOther) < 0.5)
							instigatorEntity = collision.m_OtherPilot;
					}
				}

				// If there was no other pilot to blame, Blame last person to damage the vehicle, if there is nobody, blame the pilot.
				if (!instigatorEntity)
				{
					instigatorEntity = GetInstigator().GetInstigatorEntity();
					Vehicle thisVehicle = Vehicle.Cast(owner);
					if (!instigatorEntity && thisVehicle)
					{
						ChimeraCharacter pilot = ChimeraCharacter.Cast(thisVehicle.GetPilot());
						CharacterControllerComponent pilotCharacterControler;
						if (pilot)
							pilotCharacterControler = pilot.GetCharacterController();

						if (pilotCharacterControler && pilotCharacterControler.GetLifeState() != ECharacterLifeState.ALIVE)
						{
							SCR_DamageManagerComponent pilotDamageManager = pilot.GetDamageManager();
							if (pilotDamageManager)
								instigatorEntity = pilotDamageManager.GetInstigator().GetInstigatorEntity();
						}
						else
						{
							instigatorEntity = pilot;
						}
					}
				}

				// Apply damage to passengers in vehicle
				HandlePassengerDamage(EDamageType.COLLISION, momentumOverOccupantsThreshold * prefabData.GetCharacterDamageScale(ownerMass), Instigator.CreateInstigator(instigatorEntity));
			}
		}

		float collisionDamage = prefabData.GetVehicleDamageScale(ownerMass) * (totalMomentum - prefabData.GetVehicleMomentumThreshold(ownerMass));
#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
			Print("		Collision damage = " + collisionDamage + " = " + prefabData.GetVehicleDamageScale(ownerMass) + " * ((" + collision.m_fTotalMomentumOwner + " + " + collision.m_fTotalMomentumOther + ") - " + prefabData.GetVehicleMomentumThreshold(ownerMass) + ")", LogLevel.NORMAL);
#endif
		// Deal damage if collision damage is over threshold
		if (collisionDamage > 0)
		{
			// Get hit side multiplier (e. g. front is stronger than the left/right side)
			float damageSideMultiplier = GetSideDamageMultiplier(GetHitDirection(impactPosition));

#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS) != 0)
			Print("		Collision damage = " + (collisionDamage*damageSideMultiplier*collision.m_fDamageShare) + " = " + collisionDamage + " * " + damageSideMultiplier + " * " + collision.m_fDamageShare, LogLevel.NORMAL);
#endif
			collisionDamage *= damageSideMultiplier * collision.m_fDamageShare;
			// Handle damage requires a matrix, so we create an empty one
			vector empty[3];
			empty[0] = vector.Zero;
			empty[1] = vector.Zero;
			empty[2] = vector.Zero;

			SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.COLLISION, DamageSurroundingHitzones(impactPosition, collisionDamage, EDamageType.COLLISION), empty, GetOwner(), GetDefaultHitZone(), Instigator.CreateInstigator(instigatorEntity), null, -1, -1);
			damageContext.damageEffect = new SCR_CollisionDamageEffect();
			// finally we deal damage
			HandleDamage(damageContext);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerInt GetOnVehicleDestroyed()
	{
		if (!s_OnVehicleDestroyed)
			s_OnVehicleDestroyed = new ScriptInvokerInt();

		return s_OnVehicleDestroyed;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnVehicleDamageStateChanged()
	{
		if (!s_OnVehicleDamageStateChanged)
			s_OnVehicleDamageStateChanged = new ScriptInvoker();

		return s_OnVehicleDamageStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Update buoyancy loss rate
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		
		if (s_OnVehicleDamageStateChanged)
			s_OnVehicleDamageStateChanged.Invoke(this);
		
		HitZone defaultHitZone = GetDefaultHitZone();
		if (!defaultHitZone)
			return;

		SCR_VehicleBuoyancyComponent vehicleBuoyancy = SCR_VehicleBuoyancyComponent.Cast(GetOwner().FindComponent(SCR_VehicleBuoyancyComponent));
		if (vehicleBuoyancy)
		{
			vehicleBuoyancy.SetHealth(defaultHitZone.GetDamageStateThreshold(newState));
			
			if (newState == EDamageState.DESTROYED)
				vehicleBuoyancy.GetOnWaterEnter().Remove(OnWaterEnter);
		}
		
		if (s_OnVehicleDestroyed && newState == EDamageState.DESTROYED)
		{
			s_OnVehicleDestroyed.Invoke(GetInstigator().GetInstigatorPlayerID());

			float fireRate;
			GetSecondaryExplosionPosition(SCR_FuelHitZone, fireRate);
			UpdateVehicleFireState(fireRate, 0);

			if (fireRate > 0)
			{
				World world = GetOwner().GetWorld();
				FireDamageSystem system = FireDamageSystem.Cast(world.FindSystem(FireDamageSystem));
				if (system)
					system.Register(this);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! On Vehicle being blown up, there is a chance the passengers are ejected
	override protected void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);
		
		if (damageContext.damageType == EDamageType.EXPLOSIVE && damageContext.struckHitZone == GetDefaultHitZone())
			HandlePassengerDamage(EDamageType.EXPLOSIVE, damageContext.damageValue, damageContext.instigator);
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if there is damage that can be repaired
	override bool CanBeHealed(bool ignoreHealingDOT = true)
	{
		// Get drowned engine
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (vehicleNode)
		{
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(vehicleNode.FindComponent(VehicleControllerComponent));
			if (vehicleController && vehicleController.GetEngineDrowned())
				return true;
		}

		return super.CanBeHealed();
	}

	//------------------------------------------------------------------------------------------------
	//! Fix all the damage
	//! \param[in] ignoreHealingDOT
	override void FullHeal(bool ignoreHealingDOT = true)
	{
		// Fix drowned engine
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (vehicleNode)
		{
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(vehicleNode.FindComponent(VehicleControllerComponent));
			if (vehicleController && vehicleController.GetEngineDrowned())
				vehicleController.SetEngineDrowned(false);
		}

		// Repair everything else that can be repaired
		super.FullHeal(ignoreHealingDOT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Damage and eject passengers based on vehicle taking damage
	void HandlePassengerDamage(EDamageType damageType, float damage, notnull Instigator instigator)
	{
		if (damageType == EDamageType.COLLISION)
		{
			// Damage occupants of the car that experienced a collision
			m_CompartmentManager.DamageOccupants(damage, EDamageType.COLLISION, instigator);
			// if a collision was capable of causing more than minCollisionEjectionDamageThreshold damage, try to eject passengers

 			if (damage > GetMinCollisionDamageEjectionThreshold())
				m_CompartmentManager.EjectRandomOccupants(GetCollisionDamageEjectionChance(), true);
			
			return;
		}
		
		if (damageType == EDamageType.EXPLOSIVE)
		{
			// An explosion of minExplosionEjectionDamageThreshold or larger is capable of ejecting occupants.
			if (damage > GetMinExplosionDamageEjectionThreshold())
				m_CompartmentManager.EjectRandomOccupants(GetExplosionDamageEjectionChance(), true);
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	// Takes care of updating the reponse index
	void TickResponseIndexCheck()
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		float momentum = physics.GetMass() * physics.GetVelocity().Length(); // Calculate current vehicle's momentum
		float currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(physics.GetResponseIndex());
		int currentIndex = physics.GetResponseIndex();

		while (momentum > currentIndexMinMomentum && currentIndex < MAX_RESPONSE_INDEX)
		{
			currentIndex++;
			currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(currentIndex);
			if (momentum < currentIndexMinMomentum) // We are at a point, where current momentum isn't big enough for this index
			{
				// We go one index down and break from the loop
				currentIndex--;
				currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(currentIndex);
				break;
			}
		}

		while (momentum < currentIndexMinMomentum && currentIndex > MIN_RESPONSE_INDEX)
		{
			currentIndex--;
			currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(currentIndex);
			if (momentum > currentIndexMinMomentum) // We are at a point, where current momentum is big enough for this index
				break;
		}

		physics.SetResponseIndex(currentIndex);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_OnPhysicsActive(bool activeState)
	{
		ToggleResponseIndexTicking(GetOwner(), activeState);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] activeState
	void ToggleResponseIndexTicking(IEntity owner, bool activeState)
	{
		// Always remove first so we don't end up double-registering
		GetGame().GetCallqueue().Remove(TickResponseIndexCheck);
		if (activeState)
			GetGame().GetCallqueue().CallLater(TickResponseIndexCheck, 500, true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] activeState
	void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		Vehicle vehicle = Vehicle.Cast(owner);
		if (!vehicle)
			return;

		RplComponent rplComponent = vehicle.GetRplComponent();
		if (!rplComponent)
			return;

		if (rplComponent.IsProxy() && !rplComponent.IsOwner())
		{
			// Make sure to always deactivate ticking on remote proxies
			if (!activeState)
				RPC_OnPhysicsActive(activeState);
		}
		else if (rplComponent.IsOwner() || !rplComponent.IsProxy())
		{
			RPC_OnPhysicsActive(activeState);
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Flammability should become damage effect
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void RegisterFlammableHitZone(notnull SCR_FlammableHitZone hitZone)
	{
		if (m_aFlammableHitZones)
			m_aFlammableHitZones.Insert(hitZone);
		else
			m_aFlammableHitZones = {hitZone};
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void UnregisterFlammableHitZone(notnull SCR_FlammableHitZone hitZone)
	{
		if (m_aFlammableHitZones)
			m_aFlammableHitZones.RemoveItem(hitZone);

		if (m_aFlammableHitZones.IsEmpty())
			m_aFlammableHitZones = null;
	}
	//---- REFACTOR NOTE END ----

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: It should be possible to work without explicit flammable hitZones, perhaps supply crates could have this ability though simplified
	//------------------------------------------------------------------------------------------------
	override void UpdateFireDamage(float timeSlice)
	{
		if (!m_aFlammableHitZones)
			return;

		float fireRate;
		UpdateVehicleFireState(fireRate, timeSlice);
		UpdateFuelTankFireState(fireRate, timeSlice);
		UpdateSuppliesFireState(fireRate, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleFireStateChanged()
	{
		if (m_SignalsManager)
			m_SignalsManager.SetSignalValue(m_iVehicleFireStateSignalIdx, m_eVehicleFireState);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehicleFireState(SCR_ESecondaryExplosionScale state, vector origin = vector.Zero)
	{
		if (m_eVehicleFireState == state)
			return;

		m_eVehicleFireState = state;
		m_vVehicleFireOrigin = origin;
		Replication.BumpMe();

		OnVehicleFireStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFuelTankFireStateChanged()
	{
		if (m_SignalsManager)
			m_SignalsManager.SetSignalValue(m_iFuelTankFireStateSignalIdx, m_eFuelTankFireState);

		UpdateFireParticles(m_vFuelTankFireOrigin, m_FuelTankFireParticle, m_eFuelTankFireState, SCR_ESecondaryExplosionType.FUEL);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFuelTankFireState(SCR_ESecondaryExplosionScale state, vector origin = vector.Zero)
	{
		if (m_eFuelTankFireState == state)
			return;

		m_eFuelTankFireState = state;
		m_vFuelTankFireOrigin = origin;
		Replication.BumpMe();

		OnFuelTankFireStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesFireStateChanged()
	{
		if (m_SignalsManager)
			m_SignalsManager.SetSignalValue(m_iSuppliesFireStateSignalIdx, m_eSuppliesFireState);

		UpdateFireParticles(m_vSuppliesFireOrigin, m_SuppliesFireParticle, m_eSuppliesFireState, SCR_ESecondaryExplosionType.RESOURCE, EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetSuppliesFireState(SCR_ESecondaryExplosionScale state, vector origin = vector.Zero)
	{
		if (m_eSuppliesFireState == state)
			return;

		m_eSuppliesFireState = state;
		m_vSuppliesFireOrigin = origin;
		Replication.BumpMe();

		OnSuppliesFireStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleFireState(out float fireRate, float timeSlice)
	{
		vector averagePosition = GetSecondaryExplosionPosition(SCR_FlammableHitZone, fireRate);

		// Fire area damage
		m_fVehicleFireDamageTimeout -= timeSlice;
		if (m_fVehicleFireDamageTimeout < 0)
		{
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(fireRate, SCR_ESecondaryExplosionType.FUEL, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fVehicleFireDamageTimeout = m_fSecondaryFireDamageDelay;
		}

		SCR_ESecondaryExplosionScale vehicleFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.FUEL);
		SetVehicleFireState(vehicleFireState, averagePosition);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFuelTankFireState(float fireRate, float timeSlice)
	{
		if (!m_FuelManager)
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		if (fireRate <= 0)
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		if (GetState() == EDamageState.DESTROYED)
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		float burningFuel;
		vector averagePosition = GetSecondaryExplosionPosition(SCR_FuelHitZone, burningFuel);
		if (float.AlmostEqual(burningFuel, 0))
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		// Fire area damage
		float fuelBurnRate;
		m_fFuelTankFireDamageTimeout -= timeSlice;
		if (m_fFuelTankFireDamageTimeout < 0)
		{
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(burningFuel / fireRate, SCR_ESecondaryExplosionType.FUEL, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fFuelTankFireDamageTimeout = m_fSecondaryFireDamageDelay;
			fuelBurnRate = fireRate * m_fSecondaryFireDamageDelay * m_fFuelTankFireDamageRate;
		}

		// Consume fuel
		if (fuelBurnRate > 0)
		{
			SCR_FuelNode fuelTank;
			float fuelLoss;
			array<BaseFuelNode> fuelTanks = {};
			m_FuelManager.GetFuelNodesList(fuelTanks);
			foreach (BaseFuelNode fuelNode : fuelTanks)
			{
				fuelTank = SCR_FuelNode.Cast(fuelNode);
				if (!fuelTank)
					continue;

				// Burn fuel based on fuel tank health and amount of fuel vs burning fuel amount
				fuelLoss = fuelBurnRate * (1 - fuelTank.GetHealth()) * fuelNode.GetFuel() / burningFuel;
				if (fuelLoss > 0)
					fuelTank.SetFuel(fuelTank.GetFuel() - fuelLoss);
			}
		}

		SCR_ESecondaryExplosionScale fuelTankFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.FUEL);
		SetFuelTankFireState(fuelTankFireState, averagePosition);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSuppliesFireState(float fireRate, float timeSlice)
	{
		if (fireRate <= 0)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		if (GetState() == EDamageState.DESTROYED)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		SCR_ResourceEncapsulator encapsulator = GetResourceEncapsulator();
		if (!encapsulator)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		float totalResources = encapsulator.GetAggregatedResourceValue();
		if (totalResources <= 0)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		SCR_ResourceContainerQueueBase containerQueue = encapsulator.GetContainerQueue();
		int containerCount = encapsulator.GetContainerCount();

		SCR_ResourceContainer container;
		float weight;
		vector position;

		vector averagePosition = GetOwner().CoordToLocal(encapsulator.GetOwnerOrigin());

		// Get the weighed average position of explosion relative to encapsulator
		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (!container)
				continue;

			// Determine secondary explosion position
			weight = container.GetResourceValue() / totalResources;
			if (weight <= 0)
				continue;

			position = GetOwner().CoordToLocal(container.GetOwnerOrigin());
			averagePosition += position * weight;
		}

		SCR_ESecondaryExplosionScale suppliesFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.RESOURCE);
		SetSuppliesFireState(suppliesFireState, averagePosition);

		// Fire area damage
		float suppliesDamage;
		m_fSuppliesFireDamageTimeout -= timeSlice;
		if (m_fSuppliesFireDamageTimeout < 0)
		{

			EntitySpawnParams spawnParams = EntitySpawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(fireRate, SCR_ESecondaryExplosionType.RESOURCE, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fSuppliesFireDamageTimeout = m_fSecondaryFireDamageDelay;
			suppliesDamage = Math.Ceil(fireRate * m_fSecondaryFireDamageDelay * m_fSuppliesFireDamageRate);
		}

		if (containerCount > 1)
			suppliesDamage /= containerCount;

		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (container)
				container.DecreaseResourceValue(suppliesDamage);
		}
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMinImpulse()
	{
		return m_fMinImpulse;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void InitStaticMapForIndices()
	{
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.TINY_MOMENTUM, 0);
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.SMALL_MOMENTUM, 6666.6); // Approx UAZ (1600kg) at 15 km/h in kJ
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.MEDIUM_MOMENTUM, 22222.2); // 50 km/h
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.LARGE_MOMENTUM, 44444.4); // 100 km/h
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.HUGE_MOMENTUM, 133333.3); // 300 km/h
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called whenever an instigator is going to be set.
	//! \param[in] currentInstigator: This damage manager's last instigator
	//! \param[in] newInstigator: The new instigator for this damage manager
	//! \return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		// if the vehicle is on fire and we have a valid fireInstigator, do not allow any new instigators to override it
		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(GetDefaultHitZone());
		Instigator fireInstigator = flammableHitZone.GetFireInstigator();
		if (fireInstigator && flammableHitZone && IsOnFire(flammableHitZone) && flammableHitZone.GetFireInstigator() != newInstigator)
			return false;
		
		// Don't override valid instigators with invalid instigators
		if (newInstigator.GetInstigatorType() == InstigatorType.INSTIGATOR_NONE)
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_VehicleDamageManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//Can be removed when this event is called on components
		Vehicle ownerVehicle = Vehicle.Cast(ent);
		if (ownerVehicle)
			ownerVehicle.GetOnPhysicsActive().Insert(EOnPhysicsActive);

		if (!s_mResponseIndexMomentumMap.Find(SCR_EPhysicsResponseIndex.TINY_MOMENTUM, null))
			InitStaticMapForIndices();

#ifdef ENABLE_DIAG
		if (Replication.IsServer()) //collision data is only processed on the server so there is no reason to have this debug on client and RplComponent will probably not exist at this point
			DiagMenu.RegisterItem(SCR_DebugMenuID.DEBUGUI_VEHICLES_COLLISIONS, "", "Collisions debug prints", "Vehicles", "disabled,base,extended,all");
#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_VehicleDamageManagerComponent()
	{
		if (m_FuelTankFireParticle)
			SCR_ParticleHelper.StopParticleEmissionAndLights(m_FuelTankFireParticle);

		if (m_SuppliesFireParticle)
			SCR_ParticleHelper.StopParticleEmissionAndLights(m_SuppliesFireParticle);
	}
}
