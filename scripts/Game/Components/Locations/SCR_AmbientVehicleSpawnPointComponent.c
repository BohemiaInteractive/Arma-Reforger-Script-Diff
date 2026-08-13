class SCR_AmbientVehicleSpawnPointComponentClass : ScriptComponentClass
{
}

class SCR_AmbientVehicleSpawnPointComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.EditBox, "How often will the vehicle respawn when destroyed. (seconds, 0 = no respawn)", "0 inf 1")]
	protected int m_iRespawnPeriod;

	[Attribute("0", desc: "If true, the vehicle will not despawn on spawnpoint if its damage or fuel changed.")]
	protected bool m_bStopDespawnOnInteraction;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute("0", desc: "If true, only assets with ALL of provided included labels will be used.")]
	protected bool m_bRequireAllIncludedLabels;

	[Attribute("1", desc: "If true, vehicles will be saved into persistence system on despawn (if available).")]
	protected bool m_bSaveOnDespawn;

	protected static const int SPAWNING_RADIUS = 5;				//m, check empty space on a spawnpoint with this radius
	protected static const int IGNORE_CHANGES_DELAY_MS = 1000;	//ms, how long after vehicle spawn should changes to fuel or damage be ignored before despawn is disabled

	protected bool m_bDepleted;
	protected bool m_bFirstSpawnDone;
	protected bool m_bSpawnProcessed;
	protected bool m_bAllowDespawn = true;

	protected float m_fSpawnTimestamp;

	protected WorldTimestamp m_fRespawnTimestamp;
	protected WorldTimestamp m_fDespawnTimestamp;

	protected ResourceName m_sPrefab;

	protected Vehicle m_Vehicle;

	protected Faction m_SavedFaction;
	protected UUID m_sVehicleDataId = UUID.NULL_UUID;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRespawnPeriod()
	{
		return m_iRespawnPeriod;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] depleted
	void SetIsDepleted(bool depleted)
	{
		m_bDepleted = depleted;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsDepleted()
	{
		return m_bDepleted;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsFirstSpawnDone()
	{
		return m_bFirstSpawnDone;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsSpawnProcessed()
	{
		return m_bSpawnProcessed;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDespawnAllowed()
	{
		return m_bAllowDespawn;
	}

	//------------------------------------------------------------------------------------------------
	void SetAllowDespawn(bool allow)
	{
		m_bAllowDespawn = allow;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetDespawnTimestamp(WorldTimestamp time)
	{
		m_fDespawnTimestamp = time;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetDespawnTimestamp()
	{
		return m_fDespawnTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetRespawnTimestamp(WorldTimestamp timestamp)
	{
		m_fRespawnTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetRespawnTimestamp()
	{
		return m_fRespawnTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	void SetDespawnedDataId(UUID dataId)
	{
		m_sVehicleDataId = dataId;
	}

	//------------------------------------------------------------------------------------------------
	UUID GetDespawnedDataId()
	{
		return m_sVehicleDataId;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Vehicle GetSpawnedVehicle()
	{
		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnedVehicle(Vehicle vehicle)
	{
		m_Vehicle = vehicle;
		m_bSpawnProcessed = true;
		m_bFirstSpawnDone = true;
	}

	//------------------------------------------------------------------------------------------------
	void SpawnVehicle()
	{
		SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!comp)
			return;

		SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());
		if (!faction)
			faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());

		if (faction != m_SavedFaction || (!faction && m_sPrefab.IsEmpty()))
			Update(faction);

		if (m_sPrefab.IsEmpty())
			return;

		vector pos;
		const bool spawnEmpty = SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOwner().GetOrigin(), SPAWNING_RADIUS, SPAWNING_RADIUS);
		if (!spawnEmpty)
		{
#ifdef WORKBENCH
			Print("SCR_AmbientVehicleSpawnPointComponent: FindEmptyTerrainPosition failed at " + GetOwner().GetOrigin().ToString(), LogLevel.WARNING);
#endif

			// In case this spawnpoint is blocked from the start, don't process it anymore
			// Prevents unexpected behavior such as vehicles spawning on a spot where a service composition has been built and after a session load dismantled
			if (!m_bFirstSpawnDone)
				m_bDepleted = true;

			return;
		}

		if (m_Vehicle)
			RemoveInteractionHandlers(m_Vehicle);

		m_fRespawnTimestamp = null;
		m_bFirstSpawnDone = true;
		m_bSpawnProcessed = true;

		if (!m_sVehicleDataId.IsNull())
		{
			auto persistence = PersistenceSystem.GetInstance();
			if (persistence)
			{
				PersistenceSpawnRequest request();
				request.Collection = persistence.FindCollection("Vehicle");
				request.Include = {m_sVehicleDataId};

				PersistenceResultCallback callback(OnVehicleLoaded);
				persistence.RequestSpawn(request, callback);
				return;
			}
		}

		DoSpawnVehicle();
		OnVehicleReady(m_Vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected void DoSpawnVehicle()
	{
		EntitySpawnParams params();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetTransform(params.Transform);
		m_Vehicle = Vehicle.Cast(GetGame().SpawnEntityPrefabEx(m_sPrefab, false, params: params));

		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();
		if (manager)
			manager.OnVehicleSpawned(this, m_Vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleLoaded(EPersistenceStatusCode statusCode, Managed result, bool isLast)
	{
		m_Vehicle = Vehicle.Cast(result);
		if (!m_Vehicle)
			DoSpawnVehicle();

		OnVehicleReady(m_Vehicle);
		m_sPrefab = SCR_ResourceNameUtils.GetPrefabName(m_Vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleReady(notnull IEntity vehicle)
	{
		m_fSpawnTimestamp = GetGame().GetWorld().GetWorldTime();

		// Activate handbrake so the vehicles don't go downhill on their own when spawned
		CarControllerComponent carController = CarControllerComponent.Cast(vehicle.FindComponent(CarControllerComponent));
		if (carController)
			carController.SetPersistentHandBrake(true);

		// Snap to terrain
		Physics physicsComponent = vehicle.GetPhysics();
		if (physicsComponent)
			physicsComponent.SetVelocity("0 -1 0");

		EventHandlerManagerComponent handler = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
		if (handler)
			handler.RegisterScriptHandler("OnDestroyed", this, OnVehicleDestroyed);

		if (m_bStopDespawnOnInteraction)
			AddInteractionHandlers(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	protected void OnVehicleDestroyed(IEntity vehicle)
	{
		if (m_Vehicle != vehicle)
			return;

		m_Vehicle = null;
		m_bAllowDespawn = true;

		if (m_iRespawnPeriod > 0)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			m_fRespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		}
		else
			m_bDepleted = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddInteractionHandlers(notnull IEntity vehicle)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(vehicle);

		if (damageManager)
		{
			array<HitZone> hitZones = {};
			damageManager.GetAllHitZonesInHierarchy(hitZones);
			SCR_HitZone hitZoneCast;

			foreach (HitZone hitZone : hitZones)
			{
				hitZoneCast = SCR_HitZone.Cast(hitZone);

				if (!hitZoneCast)
					continue;

				hitZoneCast.GetOnDamageStateChanged().Insert(OnDamageChanged);
			}
		}

		SCR_FuelManagerComponent fuelManager = SCR_FuelManagerComponent.Cast(vehicle.FindComponent(SCR_FuelManagerComponent));

		if (fuelManager)
			fuelManager.GetOnFuelChanged().Insert(OnFuelChanged);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveInteractionHandlers(notnull IEntity vehicle)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(vehicle);
		if (damageManager)
		{
			array<HitZone> hitZones = {};
			damageManager.GetAllHitZonesInHierarchy(hitZones);
			SCR_HitZone hitZoneCast;

			foreach (HitZone hitZone : hitZones)
			{
				hitZoneCast = SCR_HitZone.Cast(hitZone);

				if (!hitZoneCast)
					continue;

				hitZoneCast.GetOnDamageStateChanged().Remove(OnDamageChanged);
			}
		}

		SCR_FuelManagerComponent fuelManager = SCR_FuelManagerComponent.Cast(vehicle.FindComponent(SCR_FuelManagerComponent));
		if (fuelManager)
			fuelManager.GetOnFuelChanged().Remove(OnFuelChanged);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DespawnVehicle()
	{
		if (m_Vehicle && m_bSaveOnDespawn)
		{
			auto persistence = PersistenceSystem.GetInstance();
			if (persistence)
			{
				m_sVehicleDataId = persistence.GetId(m_Vehicle);
				if (!m_sVehicleDataId.IsNull())
				{
					persistence.Save(m_Vehicle, ESaveGameType.SCRIPTED);
					persistence.StopTracking(m_Vehicle, false);
				}
			}
		}

		m_fDespawnTimestamp = null;
		m_bSpawnProcessed = false;
		auto vehicle = m_Vehicle;
		m_Vehicle = null; // Clear early to suppress destory event
		RplComponent.DeleteRplEntity(vehicle, false);
	}

	//------------------------------------------------------------------------------------------------
	void OnFuelChanged(float newFuel)
	{
		// Give it some time in case there are some operations right after spawn
		if (m_bAllowDespawn && GetGame().GetWorld().GetWorldTime() > m_fSpawnTimestamp + IGNORE_CHANGES_DELAY_MS)
		{
			m_bAllowDespawn = false;

			if (m_Vehicle)
				RemoveInteractionHandlers(m_Vehicle)
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnDamageChanged(SCR_HitZone hitZone)
	{
		// Give it some time in case there are some operations right after spawn
		if (m_bAllowDespawn && GetGame().GetWorld().GetWorldTime() > m_fSpawnTimestamp + IGNORE_CHANGES_DELAY_MS)
		{
			m_bAllowDespawn = false;

			if (m_Vehicle)
				RemoveInteractionHandlers(m_Vehicle)
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Update(SCR_Faction faction)
	{
		m_SavedFaction = faction;
		SCR_EntityCatalog entityCatalog;

		if (faction)
		{
			entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		}
		else
		{
			SCR_EntityCatalogManagerComponent comp = SCR_EntityCatalogManagerComponent.GetInstance();

			if (!comp)
				return;

			entityCatalog = comp.GetEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		}

		if (!entityCatalog)
			return;

		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(data, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bRequireAllIncludedLabels);

		if (data.IsEmpty())
			return;

		m_sPrefab = (data.GetRandomElement().GetPrefab());
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));

		if (!factionComponent)
		{
			Print("SCR_AmbientVehicleSpawnPointComponent: SCR_FactionAffiliationComponent not found on owner entity. Vehicle spawning will not be available.", LogLevel.WARNING);
			return;
		}

		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();
		if (manager)
			manager.RegisterSpawnpoint(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_Vehicle)
		{
			EventHandlerManagerComponent handler = EventHandlerManagerComponent.Cast(m_Vehicle.FindComponent(EventHandlerManagerComponent));
			if (handler)
				handler.RemoveScriptHandler("OnDestroyed", this, OnVehicleDestroyed);

			if (m_bStopDespawnOnInteraction)
				RemoveInteractionHandlers(m_Vehicle);
		}

		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();
		if (manager)
			manager.UnregisterSpawnpoint(this);
	}
}
