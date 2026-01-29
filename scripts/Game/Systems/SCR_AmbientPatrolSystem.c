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

	protected static const int CHECK_INTERVAL = 3;					//s, how often should an individual patrol spawn be checked
	protected static const int DESPAWN_TIMEOUT = 10000;				//ms
	protected static const int SPAWN_RADIUS_BLOCK_SQ = 150 * 150;	// Square value for distance checks

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

	protected int m_iMinSpawnDistanceSq = m_iMinSpawnDistance * m_iMinSpawnDistance;
	protected int m_iMaxSpawnDistanceSq = m_iMaxSpawnDistance * m_iMaxSpawnDistance;

	protected int m_iDespawnBufferDistanceSq = m_iDespawnBufferDistance * m_iDespawnBufferDistance;
	protected int m_iMinDespawnDistanceSq = m_iMinDespawnDistance * m_iMinDespawnDistance;
	protected int m_iMaxDespawnDistanceSq = m_iMaxDespawnDistance * m_iMaxDespawnDistance;

	protected ref array<SCR_AmbientPatrolSpawnPointComponent> m_aPatrols = {};
	protected ref array<IEntity> m_aPlayers = {};

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

		RefreshPlayerList();

		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		int fractionOfVD = GetGame().GetViewDistance() * 0.3;
		m_iSpawnDistanceSq = ClampSpawnDistanceSq(fractionOfVD * fractionOfVD);
		m_iDespawnDistanceSq = ClampDespawnDistanceSq(m_iSpawnDistanceSq + m_iDespawnBufferDistanceSq);

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
		SCR_AmbientPatrolSpawnPointComponent spawnpoint = m_aPatrols[spawnpointIndex];
		if (!spawnpoint || (spawnpoint.GetMembersAlive() == 0 && !spawnpoint.GetIsSpawned()))
			return;

		ChimeraWorld world = GetWorld();
		const WorldTimestamp currentTime = world.GetServerTimestamp();
		if (spawnpoint.GetRespawnTimestamp().Greater(currentTime))
			return;

		if (!spawnpoint.GetIsSpawned())
		{
			spawnpoint.SpawnPatrol();
			return;
		}

		bool playersNear;
		bool playersVeryNear;
		bool playersFar = true;
		vector location = spawnpoint.GetOwner().GetOrigin();
		int distance;

		int spawnDistanceSq = m_iSpawnDistanceSq;
		int spawnPointSpawnDistance = spawnpoint.GetSpawnDistanceOverride();
		if (spawnPointSpawnDistance >= 0)
			spawnDistanceSq = ClampSpawnDistanceSq(spawnPointSpawnDistance * spawnPointSpawnDistance);

		int despawnDistanceSq = m_iDespawnDistanceSq;
		int spawnPointDespawnDistance = spawnpoint.GetDespawnDistanceOverride();
		if (spawnPointDespawnDistance >= 0)
			despawnDistanceSq = ClampDespawnDistanceSq(spawnPointDespawnDistance * spawnPointDespawnDistance);

		// Define if any player is close enough to spawn or if all players are far enough to despawn
		foreach (IEntity player : m_aPlayers)
		{
			if (!player)
				continue;

			distance = vector.DistanceSq(player.GetOrigin(), location);

			if (distance > despawnDistanceSq)
				continue;

			playersFar = false;

			if (distance > spawnDistanceSq)
				continue;

			playersNear = true;

			if (distance > SPAWN_RADIUS_BLOCK_SQ)
				continue;

			playersVeryNear = true;
			break;
		}

		bool isAIOverLimit;
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (aiWorld)
		{
			int maxChars = aiWorld.GetLimitOfActiveAIs();

			if (maxChars <= 0)
				isAIOverLimit = true;
			else
				isAIOverLimit = ((float)aiWorld.GetCurrentNumOfActiveAIs() / (float)maxChars) > spawnpoint.GetAILimitThreshold();
		}

		if (!isAIOverLimit && !playersVeryNear)
			spawnpoint.SetIsPaused(false);

		if (playersNear && !spawnpoint.GetIsPaused() && !spawnpoint.IsGroupActive())
		{
			// Do not spawn the patrol if the AI threshold setting has been reached
			if (isAIOverLimit)
			{
				spawnpoint.SetIsPaused(true);	// Make sure a patrol is not spawned too close to players when AI limit suddenly allows spawning of this group
				return;
			}

			spawnpoint.ActivateGroup();
			return;
		}

		// Delay is used so dying players don't see the despawn happen
		if (spawnpoint.GetIsSpawned() && playersFar && spawnpoint.IsGroupActive())
		{
			const WorldTimestamp despawnT = spawnpoint.GetDespawnTimestamp();
			if (despawnT == 0)
			{
				spawnpoint.SetDespawnTimestamp(currentTime.PlusMilliseconds(DESPAWN_TIMEOUT));
			}
			else if (currentTime.Greater(despawnT))
			{
				spawnpoint.DeactivateGroup();
			}
		}
		else
		{
			spawnpoint.SetDespawnTimestamp(null);
		}
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
