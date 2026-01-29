void OnAmbientVehicleSpawnedDelegate(SCR_AmbientVehicleSpawnPointComponent spawnpoint, Vehicle vehicle);

typedef func OnAmbientVehicleSpawnedDelegate;

typedef ScriptInvokerBase<OnAmbientVehicleSpawnedDelegate> OnAmbientVehicleSpawnedInvoker;

class SCR_AmbientVehicleSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(ESystemLocation.Server)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected static const int CHECK_INTERVAL = 3;					//s, how often should an individual spawnpoint be checked
	protected static const int DESPAWN_TIMEOUT = 10000;				//ms
	protected static const int PARKED_THRESHOLD = 5;				//m, how far away can the spawned vehicle moved before being considered used

	protected static const int SPAWN_RADIUS_MIN_SQ = 500 * 500;		// Square value for distance checks
	protected static const int SPAWN_RADIUS_MAX_SQ = 1000 * 1000;	// Square value for distance checks
	protected static const int DESPAWN_RADIUS_DIFF_SQ = 200 * 200;	// Square value for distance checks

	protected ref array<SCR_AmbientVehicleSpawnPointComponent> m_aSpawnpoints = {};
	protected ref array<IEntity> m_aPlayers = {};

	protected ref OnAmbientVehicleSpawnedInvoker m_OnVehicleSpawned;

	protected int m_iIndexToCheck;
	protected int m_iSpawnDistanceSq;
	protected int m_iDespawnDistanceSq;

	protected float m_fTimer;
	protected float m_fCheckInterval;

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		// No need to run updates unless some spawnpoints are actually registered
		if (m_aSpawnpoints.IsEmpty())
			Enable(false);

		RefreshPlayerList();

		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		int fractionVD = GetGame().GetViewDistance() * 0.3;
		m_iSpawnDistanceSq = fractionVD * fractionVD;
		m_iSpawnDistanceSq = Math.Min(SPAWN_RADIUS_MAX_SQ, m_iSpawnDistanceSq);
		m_iSpawnDistanceSq = Math.Max(SPAWN_RADIUS_MIN_SQ, m_iSpawnDistanceSq);
		m_iDespawnDistanceSq = m_iSpawnDistanceSq + DESPAWN_RADIUS_DIFF_SQ;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!GetGame().AreGameFlagsSet(EGameFlags.SpawnVehicles))
		{
			Enable(false);
			return;
		}

		m_fTimer += args.GetTimeSliceSeconds();
		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;

		ProcessSpawnpoint(m_iIndexToCheck++);
		if (!m_aSpawnpoints.IsIndexValid(m_iIndexToCheck))
			m_iIndexToCheck = 0;
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AmbientVehicleSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return SCR_AmbientVehicleSystem.Cast(world.FindSystem(SCR_AmbientVehicleSystem));
	}

	//------------------------------------------------------------------------------------------------
	OnAmbientVehicleSpawnedInvoker GetOnVehicleSpawned()
	{
		if (!m_OnVehicleSpawned)
			m_OnVehicleSpawned = new OnAmbientVehicleSpawnedInvoker();

		return m_OnVehicleSpawned;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCheckInterval()
	{
		m_fCheckInterval = CHECK_INTERVAL / m_aSpawnpoints.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPlayerList()
	{
		m_aPlayers.Clear();
		array<int> playerIds = {};
		PlayerManager pc = GetGame().GetPlayerManager();
		int playersCount = pc.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity player = pc.GetPlayerControlledEntity(playerId);

			if (!player)
				continue;

			CharacterControllerComponent comp = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));

			if (!comp || comp.IsDead())
				continue;

			m_aPlayers.Insert(player);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnedOrDeleted(int playerId, IEntity player)
	{
		RefreshPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessSpawnpoint(int spawnpointIndex)
	{
		if (!m_aSpawnpoints || !m_aSpawnpoints.IsIndexValid(spawnpointIndex))
			return;

		SCR_AmbientVehicleSpawnPointComponent spawnpoint = m_aSpawnpoints[spawnpointIndex];
		if (!spawnpoint || spawnpoint.GetIsDepleted())
			return;

		Vehicle spawnedVeh = spawnpoint.GetSpawnedVehicle();
		ChimeraWorld world = GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		int respawnPeriod = spawnpoint.GetRespawnPeriod();

		// Non-respawning vehicle has beed deleted by other means
		// Ignore this spawnpoint from now on
		if (!spawnedVeh && spawnpoint.GetIsSpawnProcessed() && respawnPeriod <= 0)
		{
			spawnpoint.SetIsDepleted(true);
			return;
		}

		if (!spawnedVeh)
		{
			WorldTimestamp respawnTimestamp = spawnpoint.GetRespawnTimestamp();

			// Respawn timer is ticking
			if (respawnTimestamp.Greater(currentTime))
				return;

			if (spawnpoint.GetIsFirstSpawnDone())
			{
				// Vehicle has been deleted, setup respawn timer if enabled
				if (respawnTimestamp == 0 && respawnPeriod > 0)
				{
					spawnpoint.SetRespawnTimestamp(currentTime.PlusSeconds(respawnPeriod));
					return;
				}
			}
		}

		vector location = spawnpoint.GetOwner().GetOrigin();

		// Only handle vehicles which are still on their spawnpoints
		if (spawnedVeh && vector.DistanceXZ(spawnedVeh.GetOrigin(), location) > PARKED_THRESHOLD)
		{
			spawnpoint.RemoveInteractionHandlers(spawnedVeh);

			// Non-respawning spawnpoints get depleted once the vehicle leaves the spawnpoint
			if (spawnpoint.GetRespawnPeriod() <= 0)
				spawnpoint.SetIsDepleted(true);

			return;
		}

		bool playersNear = false;
		bool playersFar = true;
		int distance;

		// Define if any player is close enough to spawn or if all players are far enough to despawn
		foreach (IEntity player : m_aPlayers)
		{
			if (!player)
				continue;

			distance = vector.DistanceSq(player.GetOrigin(), location);

			if (distance < m_iDespawnDistanceSq)
			{
				playersFar = false;

				if (distance < m_iSpawnDistanceSq)
				{
					playersNear = true;
					break;
				}
			}
		}

		if (!spawnedVeh && playersNear)
		{
			Vehicle vehicle = spawnpoint.SpawnVehicle();

			if (vehicle && m_OnVehicleSpawned)
				m_OnVehicleSpawned.Invoke(spawnpoint, vehicle);

			return;
		}

		// Delay is used so dying players don't see the despawn happen
		if (spawnedVeh && playersFar && spawnpoint.IsDespawnAllowed())
		{
			const WorldTimestamp despawnT = spawnpoint.GetDespawnTimestamp();
			if (despawnT == 0)
			{
				spawnpoint.SetDespawnTimestamp(currentTime.PlusMilliseconds(DESPAWN_TIMEOUT));
			}
			else if (currentTime.Greater(despawnT))
			{
				spawnpoint.DespawnVehicle();
			}
		}
		else
		{
			spawnpoint.SetDespawnTimestamp(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterSpawnpoint(notnull SCR_AmbientVehicleSpawnPointComponent spawnpoint)
	{
		if (!IsEnabled())
			Enable(true);

		m_aSpawnpoints.Insert(spawnpoint);
		UpdateCheckInterval();
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterSpawnpoint(notnull SCR_AmbientVehicleSpawnPointComponent spawnpoint)
	{
		m_aSpawnpoints.RemoveItem(spawnpoint);
		m_iIndexToCheck = 0;

		if (!m_aSpawnpoints.IsEmpty())
		{
			UpdateCheckInterval();
			return;
		}

		Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	int GetSpawnpoints(out notnull array<SCR_AmbientVehicleSpawnPointComponent> spawnpoints)
	{
		return spawnpoints.Copy(m_aSpawnpoints);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1)
	{
		RefreshPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		RefreshPlayerList();
	}
}
