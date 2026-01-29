class SCR_GameModeCleanSweepClass : SCR_BaseGameModeClass
{
}

class SCR_GameModeCleanSweep : SCR_BaseGameMode
{
	[Attribute("6")]
	protected int m_iMinAiGroups;

	[Attribute("10")]
	protected int m_iMaxAiGroups;

	[Attribute("1.5")]
	protected float m_fAiGroupsPerPlayer;

	[Attribute("", UIWidgets.ResourceNamePicker, "What group type should spawn")]
	protected ResourceName m_GroupType;

	[Attribute(defvalue: "{EBD2A7DA0A3C6E17}UI/layouts/Menus/CleanSweep/CleanSweepAreaSelection.layout", desc: "Layout for area selection")]
	protected ResourceName m_AreaSelectionLayout;

	[Attribute("0")]
	protected bool m_bSwapSides;

	protected Widget m_wRoot;
	protected Widget m_wAreaSelectionWidget;
	protected TextWidget m_wText;

	protected ref array<IEntity> m_aEnemySoldiers = {};
	protected ref array<SCR_AIGroup> m_aGroups;
	protected ref array<SCR_SpawnPoint> m_aEnemySpawnPoints;
	protected ref array<SCR_SpawnPoint> m_aPlayerSpawnPoints;
	protected AIWaypoint m_AttackWP;

	protected int m_iGameMasterID = -1;

	[RplProp(onRplName: "OnAreaChanged")]
	protected int m_iAreaID = INVALID_AREA_INDEX;

	protected const int INVALID_AREA_INDEX = -1;
	protected const string ENEMY_PRESENCE_ELIMINATED_TEXT = "Enemy presence eliminated!\nNew area is being selected.\nPlease wait.";

	//------------------------------------------------------------------------------------------------
	protected void OnAreaChanged()
	{
		if (m_wAreaSelectionWidget)
			m_wAreaSelectionWidget.RemoveFromHierarchy();

		if (!m_aEnemySpawnPoints)
			m_aEnemySpawnPoints = {};
		else
			m_aEnemySpawnPoints.Clear();

		if (!m_aPlayerSpawnPoints)
			m_aPlayerSpawnPoints = {};
		else
			m_aPlayerSpawnPoints.Clear();

		if (m_RplComponent && m_RplComponent.IsProxy())
			return;

		// Spawn setup:
		// Select random spawn point

		array<SCR_CleanSweepArea> activeAreas = {};
		int activeAreasCount = GetActiveAreas(activeAreas);
		if (activeAreasCount < 1)
		{
			Print("CleanSweep has not enough active playable areas. GameMode will not function.", LogLevel.ERROR);
			return;
		}

		if (!activeAreas.IsIndexValid(m_iAreaID))
			return;

		SCR_CleanSweepArea area = activeAreas[m_iAreaID];
		if (!area)
			return;

		// Move waypoint to the area
		AIWaypoint wp = AIWaypoint.Cast(GetWorld().FindEntityByName("WP1"));
		if (wp)
			wp.SetOrigin(area.GetOrigin());

		vector centerPos = area.GetOrigin();
		float rangeSq = area.m_Range * area.m_Range;
		array<IEntity> entities = {};
		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPoints();
		GetWorld().GetActiveEntities(entities);

		foreach (SCR_SpawnPoint spawnPoint : spawnPoints)
		{
			if (vector.DistanceSqXZ(centerPos, spawnPoint.GetOrigin()) < rangeSq)
			{
				FactionKey faction = spawnPoint.GetFactionKey();
				if (faction == "USSR")
				{
					if (!m_bSwapSides)
						m_aEnemySpawnPoints.Insert(spawnPoint);
					else
						m_aPlayerSpawnPoints.Insert(spawnPoint);
				}
				if (faction == "US")
				{
					if (!m_bSwapSides)
						m_aPlayerSpawnPoints.Insert(spawnPoint);
					else
						m_aEnemySpawnPoints.Insert(spawnPoint);
				}
			}
		}

		foreach (IEntity entity : entities)
		{
			// Vehicle spawn component ??
			SCR_VehicleSpawner vehicleSpawner = SCR_VehicleSpawner.Cast(entity.FindComponent(SCR_VehicleSpawner));
			if (!vehicleSpawner)
				continue;

			if (vector.DistanceSqXZ(centerPos, entity.GetOrigin()) < rangeSq)
				vehicleSpawner.PerformSpawn();
		}

		SpawnEnemies();
		m_bAutoPlayerRespawn = true;

		RespawnPlayers();
	}

	//------------------------------------------------------------------------------------------------
	protected void RespawnPlayers()
	{
		array<int> players = {};
		int count = GetGame().GetPlayerManager().GetPlayers(players);
		PlayerController playerController;
		SCR_CleanSweepNetworkComponent networkComponent;
		foreach (int player : players)
		{
			playerController = GetGame().GetPlayerManager().GetPlayerController(player);
			if (!playerController)
				continue;

			networkComponent = SCR_CleanSweepNetworkComponent.Cast(playerController.FindComponent(SCR_CleanSweepNetworkComponent));
			if (!networkComponent)
				continue;

			networkComponent.CommitSuicide();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] swapSides
	void SetSwapSides(bool swapSides)
	{
		m_bSwapSides = swapSides;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] targetAreaID
	void SetArea(int targetAreaID)
	{
		m_iAreaID = targetAreaID; // check if index is valid?
		OnAreaChanged();
		Replication.BumpMe();

		if (m_RplComponent && !m_RplComponent.IsProxy())
			GetGame().GetCallqueue().CallLater(CheckActiveAreaState, 1, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckActiveAreaState()
	{
		ChimeraCharacter character;
		DamageManagerComponent damageManager;
		for (int i = m_aEnemySoldiers.Count() -1; i >= 0; i--)
		{
			if (!m_aEnemySoldiers[i])
			{
				m_aEnemySoldiers.Remove(i);
			}
			else
			{
				character = ChimeraCharacter.Cast(m_aEnemySoldiers[i]);
				if (!character)
				{
					m_aEnemySoldiers.Remove(i);
					continue;
				}

				damageManager = character.GetDamageManager();
				if (!damageManager)
					return;

				if (damageManager.GetState() != EDamageState.DESTROYED)
					return;
			}
		}

		if (m_RplComponent && !m_RplComponent.IsProxy())
		{
			ReplicatedShowHint(0, 5);

			if (RplSession.Mode() != RplMode.Dedicated)
				ShowHint(0, 5);

			m_iAreaID = INVALID_AREA_INDEX;
			Replication.BumpMe();
			GetGame().GetCallqueue().Remove(CheckActiveAreaState);
			ShowAreaSelectionToGameMaster();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (RplSession.Mode() != RplMode.Dedicated && m_iAreaID == INVALID_AREA_INDEX && m_wAreaSelectionWidget)
			GetGame().GetInputManager().ActivateContext("MenuContext");
	}

	//------------------------------------------------------------------------------------------------
	protected void ReplicatedShowHint(int hintID, float showTime)
	{
		Rpc(RPC_ShowHint, hintID, showTime);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_ShowHint(int hintID, float time)
	{
		ShowHint(hintID, time);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowHint(int hintID, float showTime)
	{
		if (!m_wText || !m_wRoot)
			return;

		switch (hintID)
		{
			case 0:
			{
				m_wText.SetText(ENEMY_PRESENCE_ELIMINATED_TEXT);
				break;
			}
		}

		AnimateWidget.Opacity(m_wRoot, 1, 1);

		GetGame().GetCallqueue().CallLater(this.HideHint, showTime * 1000);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideHint()
	{
		if (m_wRoot)
			AnimateWidget.Opacity(m_wRoot, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnEnemies()
	{
		if (m_RplComponent && m_RplComponent.IsProxy())
			return;

		if (m_aEnemySpawnPoints.IsEmpty())
			return;

		// Clean up groups
		foreach (SCR_AIGroup group : m_aGroups)
		{
			delete group;
		}

		m_aGroups.Clear();
		m_aEnemySoldiers.Clear();

		ArmaReforgerScripted game = GetGame();
		int enemyGroupsCount = Math.ClampInt(Math.Ceil(game.GetPlayerManager().GetPlayerCount() * m_fAiGroupsPerPlayer), m_iMinAiGroups, m_iMaxAiGroups);

		RandomGenerator generator = new RandomGenerator();
		SCR_SpawnPoint spawnPoint;
		EntitySpawnParams params;
		Resource res;
		SCR_AIGroup newGrp;
		array<AIAgent> agents;
		for (int i; i < enemyGroupsCount; i++)
		{
			generator.SetSeed(Math.RandomInt(0, 100));

			spawnPoint = m_aEnemySpawnPoints.GetRandomElement();
			if (!spawnPoint)
				return;

			vector position = generator.GenerateRandomPointInRadius(0, 2, spawnPoint.GetOrigin());
			position[1] = spawnPoint.GetOrigin()[1];
			params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = position;

			res = Resource.Load(m_GroupType);
			if (!res.IsValid())
			{
				Print("Invalid Prefab " + m_GroupType, LogLevel.WARNING);
				continue;
			}

			newGrp = SCR_AIGroup.Cast(game.SpawnEntityPrefab(res, null, params));
			if (!newGrp)
				continue;

			m_aGroups.Insert(newGrp);

			agents = {};
			newGrp.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (agent)
					m_aEnemySoldiers.Insert(agent.GetControlledEntity());
			}

			if (m_AttackWP)
				newGrp.AddWaypoint(m_AttackWP);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called after a player gets killed.
	//! \param[in] playerId PlayerId of victim player
	//! \param[in] player Entity of victim player if any
	//! \param[in] killerEntity Entity of killer instigator if any
	//! \param[in] killer instigator of the kill
	protected override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		m_aEnemySoldiers.RemoveItemOrdered(playerEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetPlayersCenter()
	{
		array<int> players = {};
		int count = GetGame().GetPlayerManager().GetPlayers(players);
		if (count < 1)
			return vector.Zero;

		vector center = vector.Zero;
		int countedEntities;

		IEntity controlledEntity;
		foreach (int player : players)
		{
			controlledEntity = SCR_PossessingManagerComponent.GetPlayerMainEntity(player);
			if (!controlledEntity)
				continue;

			center += controlledEntity.GetOrigin();

			countedEntities++;
		}

		if (countedEntities != 0)
			center /= countedEntities;

		return center;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetActiveAreas(notnull array<SCR_CleanSweepArea> activeAreas)
	{
		array<SCR_CleanSweepArea> areas = SCR_CleanSweepArea.GetInstances();
		if (!areas)
			return 0;

		int areasCount;
		foreach (SCR_CleanSweepArea area : areas)
		{
			++areasCount;
			if (!area || !area.m_Active)
				continue;

			activeAreas.Insert(area);
		}

		return areasCount;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitializeServer()
	{
		m_AttackWP = AIWaypoint.Cast(GetWorld().FindEntityByName("WP1"));
		m_aGroups = {};
		m_aPlayerSpawnPoints = {};
		m_aEnemySpawnPoints = {};
	}

	//------------------------------------------------------------------------------------------------
	protected PlayerController GetGameMaster()
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(m_iGameMasterID);
		if (!playerController)
			return PickNewGameMaster();

		return playerController;
	}

	//------------------------------------------------------------------------------------------------
	protected PlayerController PickNewGameMaster(int exclude = -1)
	{
		PlayerController playerController;

		array<int> players = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		playerManager.GetPlayers(players);
		foreach (int player : players)
		{
			if (player == exclude)
				continue;

			playerController = playerManager.GetPlayerController(player);
			if (playerController)
			{
				m_iGameMasterID = player;
				break;
			}
		}

		return playerController;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowAreaSelectionToGameMaster()
	{
		PlayerController playerController = GetGameMaster();
		if (!playerController)
				return;

		SCR_CleanSweepNetworkComponent networkComponent = SCR_CleanSweepNetworkComponent.Cast(playerController.FindComponent(SCR_CleanSweepNetworkComponent));
		if (!networkComponent)
			return;

		networkComponent.ShowAreaSelectionScreen();
	}

	//------------------------------------------------------------------------------------------------
	//! Used by SCR_CleanSweepNetworkComponent
	void ShowAreaSelectionUI()
	{
		if (m_iAreaID != INVALID_AREA_INDEX || m_AreaSelectionLayout.GetPath() == string.Empty)
			return;

		if (!m_wAreaSelectionWidget)
			m_wAreaSelectionWidget = GetGame().GetWorkspace().CreateWidgets(m_AreaSelectionLayout);

		XComboBoxWidget selectionBox = XComboBoxWidget.Cast(m_wAreaSelectionWidget.FindAnyWidget("SelectionBox"));
		if (!selectionBox)
			return;

		array<SCR_CleanSweepArea> activeAreas = {};
		GetActiveAreas(activeAreas);

		foreach (SCR_CleanSweepArea activeArea : activeAreas)
		{
			selectionBox.AddItem(activeArea.GetName());
		}

		selectionBox.SetCurrentItem(0);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		Replication.BumpMe();
		if (m_iAreaID != INVALID_AREA_INDEX)
			return;

		ShowAreaSelectionToGameMaster();
		//GetGame().GetCallqueue().CallLater(ShowAreaSelectionToGameMaster, 10000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		Replication.BumpMe();
		if (m_iAreaID != INVALID_AREA_INDEX || m_iGameMasterID != playerId)
			return;

		PickNewGameMaster(m_iGameMasterID);
		ShowAreaSelectionToGameMaster();
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_bAutoPlayerRespawn = false;

		ArmaReforgerScripted game = GetGame();
		if (m_RplComponent && !m_RplComponent.IsProxy())
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(game.GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
		}

		if (m_RplComponent && !m_RplComponent.IsProxy())
			InitializeServer();

		m_wRoot = game.GetWorkspace().CreateWidgets("{DE9F713BE2C5D190}UI/layouts/HUD/HintFrame.layout");
		if (!m_wRoot)
		{
			Print("Cannot create hint layout", LogLevel.ERROR);
			return;
		}

		m_wText = TextWidget.Cast(m_wRoot.FindAnyWidget("MessageText"));
		TextWidget title = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleText"));
		if (title)
			title.SetText("CLEAN SWEEP");

		m_wRoot.SetOpacity(0);
	}
}
