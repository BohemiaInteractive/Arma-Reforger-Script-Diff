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
	// Set to true to enable temporary [DBG_AI] diagnostics in this class.
	protected static const bool DBG_AI_LOGGING = false;

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

	[Attribute("0", UIWidgets.ComboBox, "Importance tier for the spawned group. Default LOW - ambient patrols bow out first when the active-AI budget fills. Promote specific spawnpoints to NORMAL/HIGH when a particular patrol matters more.", enums: ParamEnumArray.FromEnum(SCR_EAISpawnImportance))]
	protected SCR_EAISpawnImportance m_eImportance;

	protected bool m_bSpawned;
	protected bool m_bPaused;
	protected bool m_bGroupSpawnActive;

	// Explicit "do not respawn this patrol" flag. Campaign code sets this on faction changes to
	// retire remnant patrols. The alive count lives on the group as
	// ChimeraAIGroup::m_iDormantAliveCount.
	protected bool m_bEliminated;

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
	//! Mark this patrol as retired - the ambient system will stop processing it. Used by campaign
	//! code when a base changes faction and old remnants should not respawn under the new owner.
	void SetIsEliminated(bool eliminated)
	{
		m_bEliminated = eliminated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsEliminated()
	{
		return m_bEliminated;
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

		// Re-establish proximity-driven lifecycle on the group. This is also called from SpawnPatrol
		// during the initial creation, but must run on save-game restore too: the group entity is
		// re-created without its lifecycle policy (m_eLifecyclePolicy is not persisted), and
		// ProcessSpawnpoint early-returns on GetIsSpawned()==true so SpawnPatrol does not re-fire.
		// SetLifecyclePolicy is idempotent on repeated same-policy calls.
		ConfigureGroupLifecycle();
	}

	//------------------------------------------------------------------------------------------------
	//! Apply the spawn / despawn distances and ProximityDriven policy to the currently spawned
	//! group. Pulls clamped defaults from SCR_AmbientPatrolSystem and applies per-spawnpoint
	//! overrides on top.
	protected void ConfigureGroupLifecycle()
	{
		if (!m_Group)
			return;

		// Re-anchor the group to the spawnpoint origin. SCR_AIGroupSerializer does not persist
		// the entity transform, so save-restored groups come back at <0,0,0> (or the prefab
		// default), which makes proximity checks meaningless and forces members to spawn off-map.
		// The spawnpoint's own world position is the authoritative anchor for this patrol.
		vector spawnpointTransform[4];
		GetOwner().GetWorldTransform(spawnpointTransform);
		m_Group.SetWorldTransform(spawnpointTransform);

		if (DBG_AI_LOGGING)
			Print(string.Format("[DBG_AI] ConfigureGroupLifecycle: anchored group %1 to pos=%2", m_Group, spawnpointTransform[3]), LogLevel.NORMAL);

		// Importance is not persisted on the group, so save-restored groups come back at the
		// default tier. Re-tag from the spawnpoint's authored value (LOW for ambient patrols).
		m_Group.SetImportance(m_eImportance);

		SCR_AmbientPatrolSystem patrolSystem = SCR_AmbientPatrolSystem.GetInstance();
		float spawnDist = 600;
		float despawnDist = 800;
		if (patrolSystem)
		{
			spawnDist = patrolSystem.GetSpawnDistance();
			despawnDist = patrolSystem.GetDespawnDistance();
		}
		if (m_iSpawnDistanceOverride >= 0)
			spawnDist = m_iSpawnDistanceOverride;
		if (m_iDespawnDistanceOverride >= 0)
			despawnDist = m_iDespawnDistanceOverride;
		m_Group.SetLifecyclePolicy(SCR_EAIGroupLifecyclePolicy.ProximityDriven, spawnDist, despawnDist);

		// If a player reaches the patrol position while the members could not spawn (active-AI
		// limit), the area counts as cleared - the patrol must not appear there later. The
		// group deletes itself in that case; the event below retires this spawnpoint.
		m_Group.SetEliminateWhenReached(true);
		m_Group.GetOnEliminatedWhenReached().Insert(OnGroupEliminatedWhenReached);

		if (DBG_AI_LOGGING)
			Print(string.Format("[DBG_AI] ConfigureGroupLifecycle: lifecycle set for group %1 (spawnDist=%.0fm, despawnDist=%.0fm, importance=%2)", m_Group, spawnDist, despawnDist, m_eImportance), LogLevel.NORMAL);
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
		m_bGroupSpawnActive = true;

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

		// Suppress SCR_AIGroup::EOnInit auto-spawn so the spawnpoint stays in control of the spawn
		// intent (observer-gated, breadth-first via SCR_AIWorld's queue).
		SCR_AIGroup.IgnoreSpawning(true);

		m_Group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefabEx(m_sPrefab, false, params: params));
		if (!m_Group)
			return;

		SetspawnedGroup(m_Group);

		// Importance and lifecycle policy were applied in SetspawnedGroup -> ConfigureGroupLifecycle.
		// Hand the dormant/active transitions over to the group: it queries ObserversSystem
		// periodically and auto-dismisses / re-materialises members as observers come and go.
		m_Group.AddWaypoint(m_Waypoint);

		// Initial materialisation through the spawn queue. observerRange uses the group's currently
		// configured spawn distance (set inside SetspawnedGroup -> ConfigureGroupLifecycle).
		if (!m_Group.GetSpawnImmediately())
			m_Group.RequestSpawn(-1, m_Group.GetSpawnDistance());
	}

	//------------------------------------------------------------------------------------------------
	//! Legacy external entry points. The group now drives its own dormant/active transitions via
	//! SCR_EAIGroupLifecyclePolicy.ProximityDriven (set in SpawnPatrol). These wrappers remain for
	//! scripts / save-game deserialisation that still poke activation explicitly.
	void ActivateGroup()
	{
		if (m_Group)
		{
			m_bGroupSpawnActive = true;
			// observerRange = 0 skips re-validation because save-game restore is unconditional.
			m_Group.RequestSpawn();
		}
	}
	//------------------------------------------------------------------------------------------------
	void DeactivateGroup()
	{
		if (m_Group)
		{
			m_bGroupSpawnActive = false;
			m_Group.DespawnMembers();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! This checks only if group was selected by script to be actively spawning, it does not detect Dormant groups
	//! \return
	bool IsGroupActive()
	{
		return m_bGroupSpawnActive;
	}
	
	//------------------------------------------------------------------------------------------------
	//! This checks if group was selected by script to be actively spawning and its members are not despawned due to ChimeraAIGroup::IsDormant
	//! \return
	bool IsGroupActiveAndNotDormant()
	{
		return m_bGroupSpawnActive && m_Group && !m_Group.IsDormant();
	}

	//------------------------------------------------------------------------------------------------
	//! The group eliminated itself because a player reached its position while the members
	//! could not spawn (active-AI limit). The group entity is deleted right after this call.
	//! Retire the spawnpoint so the ambient system never creates a replacement patrol;
	//! m_bEliminated is persisted by the spawnpoint serializer.
	protected void OnGroupEliminatedWhenReached(SCR_AIGroup group)
	{
		m_bEliminated = true;
	}

	//------------------------------------------------------------------------------------------------
	void OnAgentRemoved()
	{
		if (!m_Group || m_Group.GetAgentsCount() > 0)
			return;

		// Group went empty via DespawnMembers (intentional deactivation), not via combat deaths.
		// ActivateGroup re-materialises the group from its stored dormant alive count; don't schedule
		// a fresh-group respawn on top of it.
		if (m_Group.IsDormant())
			return;

		// All members were killed in combat. The old system used m_iMembersAlive == 0 in
		// ProcessSpawnpoint to block re-creation after a full wipe; that field is gone.
		// Mark the spawnpoint eliminated so ProcessSpawnpoint never creates a replacement group.
		m_bEliminated = true;
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
