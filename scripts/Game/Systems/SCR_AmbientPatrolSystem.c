//------------------------------------------------------------------------------------------------
class SCR_AmbientPatrolSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(ESystemLocation.Server)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	// Set to true to enable temporary [DBG_AI] diagnostics in this class.
	protected static const bool DBG_AI_LOGGING = false;

	protected static const int CHECK_INTERVAL = 3;	//s, how often should an individual patrol spawn be checked

	[Attribute("250", desc: "Minimum allowed spawn distance, in meters.")]
	protected int m_iMinSpawnDistance;

	[Attribute("1000", desc: "Maximum allowed spawn distance, in meters.")]
	protected int m_iMaxSpawnDistance;

	[Attribute("200", desc: "Buffer distance, in meters, added beyond spawn distance before entities are despawned.")]
	protected int m_iDespawnBufferDistance;

	[Attribute("250", desc: "Minimum allowed despawn distance, in meters.")]
	protected int m_iMinDespawnDistance;

	[Attribute("1000", desc: "Maximum allowed despawn distance, in meters.")]
	protected int m_iMaxDespawnDistance;

	// Squared mirrors of the Attribute fields above. Computed in OnInit because Enforce field
	// initializers run before Attribute values are loaded - a value-derived initializer would
	// always evaluate to 0*0 here.
	protected int m_iMinSpawnDistanceSq;
	protected int m_iMaxSpawnDistanceSq;
	protected int m_iDespawnBufferDistanceSq;
	protected int m_iMinDespawnDistanceSq;
	protected int m_iMaxDespawnDistanceSq;

	protected ref array<SCR_AmbientPatrolSpawnPointComponent> m_aPatrols = {};

	protected int m_iIndexToCheck;
	protected int m_iSpawnDistanceSq;
	protected int m_iDespawnDistanceSq;

	protected float m_fTimer;
	protected float m_fCheckInterval;

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		// No need to run updates unless some patrols are actually registered
		if (m_aPatrols.IsEmpty())
			Enable(false);

		m_iMinSpawnDistanceSq = m_iMinSpawnDistance * m_iMinSpawnDistance;
		m_iMaxSpawnDistanceSq = m_iMaxSpawnDistance * m_iMaxSpawnDistance;
		m_iDespawnBufferDistanceSq = m_iDespawnBufferDistance * m_iDespawnBufferDistance;
		m_iMinDespawnDistanceSq = m_iMinDespawnDistance * m_iMinDespawnDistance;
		m_iMaxDespawnDistanceSq = m_iMaxDespawnDistance * m_iMaxDespawnDistance;

		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		int fractionOfVD = GetGame().GetViewDistance() * 0.3;
		m_iSpawnDistanceSq = ClampSpawnDistanceSq(fractionOfVD * fractionOfVD);
		m_iDespawnDistanceSq = ClampDespawnDistanceSq(m_iSpawnDistanceSq + m_iDespawnBufferDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!GetGame().AreGameFlagsSet(EGameFlags.SpawnAI))
		{
			Enable(false);
			return;
		}

		m_fTimer += args.GetTimeSliceSeconds();
		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;

		// Don't process spawning at the very start - wait for a save to be applied if it exists
		// Otherwise full-size groups get spawned even if they are marked as eliminated in the save file
		if (SCR_PersistenceSystem.IsLoadInProgress())
			return;

		// Once per full pass through the patrol list, print a summary line so we can see the
		// system is alive and how many spawnpoints it owns. [DBG_AI] - temporary diagnostics.
		if (DBG_AI_LOGGING && m_iIndexToCheck == 0)
			Print(string.Format("[DBG_AI] AmbientPatrolSystem tick: patrols=%1, spawnDist=%2m, despawnDist=%3m", m_aPatrols.Count(), Math.Sqrt(m_iSpawnDistanceSq), Math.Sqrt(m_iDespawnDistanceSq)), LogLevel.NORMAL);
		ProcessSpawnpoint(m_iIndexToCheck++);
		if (!m_aPatrols.IsIndexValid(m_iIndexToCheck))
			m_iIndexToCheck = 0;
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AmbientPatrolSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return SCR_AmbientPatrolSystem.Cast(world.FindSystem(SCR_AmbientPatrolSystem));
	}

	//------------------------------------------------------------------------------------------------
	protected int ClampSpawnDistanceSq(int spawnDistanceSq)
	{
		return Math.ClampInt(spawnDistanceSq, m_iMinSpawnDistanceSq, m_iMaxSpawnDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	protected int ClampDespawnDistanceSq(int despawnDistanceSq)
	{
		return Math.ClampInt(despawnDistanceSq, m_iMinDespawnDistanceSq, m_iMaxDespawnDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCheckInterval()
	{
		m_fCheckInterval = CHECK_INTERVAL / m_aPatrols.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Coarse per-spawnpoint gate. Once the group entity exists it self-drives via
	//! SCR_EAIGroupLifecyclePolicy.ProximityDriven (set in SpawnPatrol); this system only decides
	//! "is it time to instantiate the group entity at all?". Asks ObserversSystem directly because
	//! at this point the group entity does not exist yet.
	protected void ProcessSpawnpoint(int spawnpointIndex)
	{
		SCR_AmbientPatrolSpawnPointComponent spawnpoint = m_aPatrols[spawnpointIndex];
		if (!spawnpoint || spawnpoint.IsEliminated())
			return;

		// Group entity already exists - the group manages its own members from here on.
		if (spawnpoint.GetIsSpawned())
			return;

		ChimeraWorld world = GetWorld();
		const WorldTimestamp currentTime = world.GetServerTimestamp();
		if (spawnpoint.GetRespawnTimestamp().Greater(currentTime))
			return;

		// Manager-level "is there room in the budget at all?" probe; per-tier cap on the actual
		// group is re-checked when the group tries to materialise members.
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (aiWorld && !aiWorld.CanActivateGroup(null))
		{
			if (DBG_AI_LOGGING)
				Print(string.Format("[DBG_AI] ProcessSpawnpoint[%1] BLOCKED: CanActivateGroup(null)=false (over budget)", spawnpointIndex), LogLevel.NORMAL);
			return;
		}

		ObserversSystem observers = ObserversSystem.Cast(world.FindSystem(ObserversSystem));
		if (!observers)
		{
			if (DBG_AI_LOGGING)
				Print(string.Format("[DBG_AI] ProcessSpawnpoint[%1] BLOCKED: ObserversSystem not found", spawnpointIndex), LogLevel.WARNING);
			return;
		}

		int despawnDistanceSq = m_iDespawnDistanceSq;
		int spawnPointDespawnDistance = spawnpoint.GetDespawnDistanceOverride();
		if (spawnPointDespawnDistance >= 0)
			despawnDistanceSq = ClampDespawnDistanceSq(spawnPointDespawnDistance * spawnPointDespawnDistance);

		vector pos = spawnpoint.GetOwner().GetOrigin();
		if (!observers.HasObserverWithinRangeSq(pos[0], pos[2], despawnDistanceSq))
		{
			if (DBG_AI_LOGGING)
				Print(string.Format("[DBG_AI] ProcessSpawnpoint[%1] BLOCKED: no observer within %2m of %3", spawnpointIndex, Math.Sqrt(despawnDistanceSq), pos), LogLevel.NORMAL);
			return;
		}

		if (DBG_AI_LOGGING)
			Print(string.Format("[DBG_AI] ProcessSpawnpoint[%1] OK -> SpawnPatrol at %2 (despawnDist=%3m)", spawnpointIndex, pos, Math.Sqrt(despawnDistanceSq)), LogLevel.NORMAL);
		spawnpoint.SpawnPatrol();
	}

	//------------------------------------------------------------------------------------------------
	//! Exposed for SCR_AmbientPatrolSpawnPointComponent to forward to SCR_AIGroup.SetLifecyclePolicy.
	float GetSpawnDistance()
	{
		return Math.Sqrt(m_iSpawnDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	float GetDespawnDistance()
	{
		return Math.Sqrt(m_iDespawnDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	void RegisterPatrol(notnull SCR_AmbientPatrolSpawnPointComponent patrol)
	{
		if (!IsEnabled())
			Enable(true);

		m_aPatrols.Insert(patrol);
		UpdateCheckInterval();
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterPatrol(notnull SCR_AmbientPatrolSpawnPointComponent patrol)
	{
		m_aPatrols.RemoveItem(patrol);
		m_iIndexToCheck = 0;

		if (!m_aPatrols.IsEmpty())
		{
			UpdateCheckInterval();
			return;
		}

		Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	int GetPatrols(notnull out array<SCR_AmbientPatrolSpawnPointComponent> patrols)
	{
		return patrols.Copy(m_aPatrols);
	}

}
