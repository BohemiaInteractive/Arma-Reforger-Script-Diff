class SCR_AmbientPatrolSpawnPointComponentClass : ScriptComponentClass
{
	[Attribute("{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et", UIWidgets.ResourceNamePicker, "Cycle waypoint to be used for waypoints in hierarchy.", "et")]
	protected ResourceName m_sCycleWaypointPrefab;

	[Attribute("{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", UIWidgets.ResourceNamePicker, "Waypoint to be used if no waypoints are found in hierarchy.", "et")]
	protected ResourceName m_sDefaultWaypointPrefab;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetCycleWaypointPrefab()
	{
		return m_sCycleWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}
}

class SCR_AmbientPatrolSpawnPointComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EGroupType))]
	protected SCR_EGroupType m_eGroupType;

	[Attribute("0")]
	protected bool m_bPickRandomGroupType;

	[Attribute("-1", desc: "Overrides the system’s spawn distance for this spawn point (in meters). Use -1 to inherit the default. Value is clamped by min/max spawn distances.")]
	protected int m_iSpawnDistanceOverride;

	[Attribute("-1", desc: "Overrides the system’s despawn distance for this spawn point (in meters). Use -1 to inherit the default. Value is clamped by min/max despawn distances.")]
	protected int m_iDespawnDistanceOverride;

	[Attribute("0", UIWidgets.EditBox, "How often will the group respawn. (seconds, 0 = no respawn)", "0 inf 1")]
	protected int m_iRespawnPeriod;

	[Attribute("0.95", desc: "If (CurrentAIs / AILimit) > this value, the group will not be spawned.",  params: "0 0.95 0.01")]
	protected float m_fAILimitThreshold;

	protected bool m_bSpawned;
	protected bool m_bPaused;
	protected bool m_bGroupActive;
	protected int m_iMembersAlive = -1; //How many were alive during despawn to respawn again later
	protected AIWaypoint m_Waypoint;
	protected ResourceName m_sPrefab;
	protected SCR_AIGroup m_Group;
	protected Faction m_SavedFaction;
	protected WorldTimestamp m_RespawnTimestamp;
	protected WorldTimestamp m_DespawnTimestamp;

	//------------------------------------------------------------------------------------------------
	int GetSpawnDistanceOverride()
	{
		return m_iSpawnDistanceOverride;
	}

	//------------------------------------------------------------------------------------------------
	int GetDespawnDistanceOverride()
	{
		return m_iDespawnDistanceOverride;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] count
	void SetMembersAlive(int count)
	{
		m_iMembersAlive = count;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMembersAlive()
	{
		return m_iMembersAlive;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] spawned
	void SetIsSpawned(bool spawned)
	{
		m_bSpawned = spawned;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsSpawned()
	{
		return m_bSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//! Pause this spawnpoint so a group will not be spawned unless players leave and re-enter the area
	//! \param[in] paused
	void SetIsPaused(bool paused)
	{
		m_bPaused = paused;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsPaused()
	{
		return m_bPaused;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetAILimitThreshold()
	{
		return m_fAILimitThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetDespawnTimestamp(WorldTimestamp time)
	{
		m_DespawnTimestamp = time;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetDespawnTimestamp()
	{
		return m_DespawnTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetRespawnTimestamp(WorldTimestamp timestamp)
	{
		m_RespawnTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetRespawnTimestamp()
	{
		return m_RespawnTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_AIGroup GetSpawnedGroup()
	{
		return m_Group;
	}

	//------------------------------------------------------------------------------------------------
	void SetspawnedGroup(SCR_AIGroup group)
	{
		m_Group = group;
		
		if (m_iRespawnPeriod != 0)
			m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	AIWaypoint GetWaypoint()
	{
		return m_Waypoint;
	}

	//------------------------------------------------------------------------------------------------
	void SetWaypoint(AIWaypoint wp)
	{
		m_Waypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetRandomPrefabByProbability(notnull SCR_EntityCatalog entityCatalog, notnull array<SCR_EntityCatalogEntry> data)
	{
		float highestProbability;
		array<ResourceName> prefabs = {};
		array<int> eligiblePrefabIds = {};
		array<float> probabilities = {};
		SCR_EntityCatalogEntry catalogEntry;
		SCR_EntityCatalogAmbientPatrolData patrolData;
		float probability;

		for (int i = 0, count = data.Count(); i < count; i++)
		{
			int catalogIndex = data[i].GetCatalogIndex();
			catalogEntry = entityCatalog.GetCatalogEntry(catalogIndex);

			if (!catalogEntry)
				continue;

			patrolData = SCR_EntityCatalogAmbientPatrolData.Cast(catalogEntry.GetEntityDataOfType(SCR_EntityCatalogAmbientPatrolData));

			if (!patrolData)
				continue;

			probability = patrolData.GetProbabilityOfPresence();

			prefabs.Insert(catalogEntry.GetPrefab());
			probabilities.Insert(probability);

			if (probability > highestProbability)
				highestProbability = probability
		}

		if (prefabs.IsEmpty())
			return ResourceName.Empty;

		float rand = Math.RandomFloat(0, highestProbability);

		for (int i = 0, count = probabilities.Count(); i < count; i++)
		{
			if (probabilities[i] >= rand)
				eligiblePrefabIds.Insert(i);
		}

		if (eligiblePrefabIds.IsEmpty())
			return ResourceName.Empty;

		return prefabs[eligiblePrefabIds.GetRandomElement()];
	}

	//------------------------------------------------------------------------------------------------
	protected void Update(SCR_Faction faction)
	{
		if (!m_Waypoint)
			PrepareWaypoints();

		m_SavedFaction = faction;
		if (!faction)
			return;

		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		if (!entityCatalog)
			return;

		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetEntityListWithData(SCR_EntityCatalogAmbientPatrolData, data);

		if (m_bPickRandomGroupType)
		{
			m_sPrefab = GetRandomPrefabByProbability(entityCatalog, data);
			return;
		}

		SCR_EntityCatalogEntry catalogEntry;
		SCR_EntityCatalogAmbientPatrolData patrolData;

		for (int i = 0, count = data.Count(); i < count; i++)
		{
			int catalogIndex = data[i].GetCatalogIndex();
			catalogEntry = entityCatalog.GetCatalogEntry(catalogIndex);
			if (!catalogEntry)
				continue;

			patrolData = SCR_EntityCatalogAmbientPatrolData.Cast(catalogEntry.GetEntityDataOfType(SCR_EntityCatalogAmbientPatrolData));
			if (!patrolData)
				continue;

			if (patrolData.GetGroupType() != m_eGroupType)
				continue;

			m_sPrefab = catalogEntry.GetPrefab();
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected AIWaypointCycle SpawnCycleWaypoint(notnull EntitySpawnParams params)
	{
		array<AIWaypoint> waypoints = {};
		array<IEntity> queue = {GetOwner()};
		AIWaypoint waypoint;
		IEntity processedEntity;
		IEntity nextInHierarchy;

		while (!queue.IsEmpty())
		{
			processedEntity = queue[0];
			queue.Remove(0);

			waypoint = AIWaypoint.Cast(processedEntity);

			if (waypoint)
				waypoints.Insert(waypoint);

			nextInHierarchy = processedEntity.GetChildren();

			while (nextInHierarchy)
			{
				queue.Insert(nextInHierarchy);
				nextInHierarchy = nextInHierarchy.GetSibling();
			}
		}

		if (waypoints.IsEmpty())
			return null;

		if (waypoints.Count() == 1)
		{
			m_Waypoint = waypoints[0];
			return null;
		}

		SCR_AmbientPatrolSpawnPointComponentClass componentData = SCR_AmbientPatrolSpawnPointComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return null;

		AIWaypointCycle wp = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefabEx(componentData.GetCycleWaypointPrefab(), false, params: params));
		if (!wp)
			return null;

		wp.SetWaypoints(waypoints);

		return wp;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void PrepareWaypoints()
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOwner().GetOrigin();

		AIWaypointCycle predefinedWaypoint = SpawnCycleWaypoint(params);
		if (predefinedWaypoint)
		{
			m_Waypoint = predefinedWaypoint;
			return;
		}
		else if (m_Waypoint)
		{
			return;
		}

		SCR_AmbientPatrolSpawnPointComponentClass componentData = SCR_AmbientPatrolSpawnPointComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;

		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefabEx(componentData.GetDefaultWaypointPrefab(), false, params: params));
		if (!wp)
			return;

		m_Waypoint = wp;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SpawnPatrol()
	{
		SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!comp)
			return;

		SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());
		if (!faction)
			faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());

		if (faction != m_SavedFaction || m_iRespawnPeriod > 0)
			Update(faction);

		m_bSpawned = true;
		m_bGroupActive = true;

		if (m_sPrefab.IsEmpty())
			return;

		EntitySpawnParams params();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOwner().GetOrigin();

		if (m_iRespawnPeriod == 0 && m_Waypoint && Math.RandomFloat01() >= 0.5)
		{
			AIWaypointCycle cycleWP = AIWaypointCycle.Cast(m_Waypoint);
			if (cycleWP)
			{
				array<AIWaypoint> waypoints = {};
				cycleWP.GetWaypoints(waypoints);
				params.Transform[3] = waypoints.GetRandomElement().GetOrigin();
			}
		}

		m_Group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefabEx(m_sPrefab, false, params: params));
		if (!m_Group)
			return;

		SetspawnedGroup(m_Group);
		
		if (!m_Group.GetSpawnImmediately())
		{
			if (m_iMembersAlive > 0)
				m_Group.SetMaxUnitsToSpawn(m_iMembersAlive);

			m_Group.SpawnUnits();
		}

		m_Group.AddWaypoint(m_Waypoint);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DespawnPatrol()
	{
		m_DespawnTimestamp = null;
		m_bSpawned = false;

		if (!m_Group)
		{
			m_iMembersAlive = 0;
			return;
		}

		array<AIAgent> units = {};
		m_Group.GetAgents(units);
		int count = m_Group.GetAgentsCount();
		m_iMembersAlive = count;
		RplComponent.DeleteRplEntity(m_Group, false);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ActivateGroup()
	{
		if (m_Group)
		{
			m_bGroupActive = true;
			m_Group.ActivateAllMembers();
		}
	}
	//------------------------------------------------------------------------------------------------
	//!
	void DeactivateGroup()
	{
		if (m_Group)
		{
			m_bGroupActive = false;
			m_Group.DeactivateAllMembers();
		}
	}
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsGroupActive()
	{
		return m_bGroupActive;
	}

	//------------------------------------------------------------------------------------------------
	void OnAgentRemoved()
	{
		if (!m_Group || m_Group.GetAgentsCount() > 0)
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		if (m_RespawnTimestamp.GreaterEqual(world.GetServerTimestamp()))
			return;

		// Set up respawn timestamp, convert s to ms, reset original group size
		m_RespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		m_iMembersAlive = -1;
		m_bSpawned = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
		{
			Print("SCR_AmbientPatrolSpawnPointComponent: SCR_FactionAffiliationComponent not found on owner entity. Patrol spawning will not be available.", LogLevel.WARNING);
			return;
		}

		SCR_AmbientPatrolSystem manager = SCR_AmbientPatrolSystem.GetInstance();
		if (manager)
			manager.RegisterPatrol(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		SCR_AmbientPatrolSystem manager = SCR_AmbientPatrolSystem.GetInstance();
		if (manager)
			manager.UnregisterPatrol(this);
	}
}
