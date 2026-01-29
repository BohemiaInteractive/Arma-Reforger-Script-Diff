[EntityEditorProps(category: "GameScripted/DataCollection/", description: "Main component used for collecting player data.")]
class SCR_DataCollectorComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_DataCollectorComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ref array<ref SCR_DataCollectorModule> m_aModules;
	
	[Attribute()]
	protected bool m_bOptionalKicking;
	
	[Attribute("3", desc: "Optional kicking: Penalty score for killing a friendly player.")]
	protected int m_iOptionalKickingFriendlyPlayerKillPenalty;
	
	[Attribute("1", desc: "Penalty score for killing a friendly AI.")]
	protected int m_iOptionalKickingFriendlyAIKillPenalty;
	
	[Attribute("10", desc: "Penalty score limit for a kick from the match.")]
	protected int m_iOptionalKickingKickPenaltyLimit;
	
	[Attribute("1800", desc: "Ban duration after a kick (in seconds, -1 for a session-long ban).")]
	protected int m_iOptionalKickingBanDuration;
	
	[Attribute("900", desc: "How often penalty score subtraction happens (in seconds).")]
	protected int m_iOptionalKickingPenaltySubtractionPeriod;
	
	[Attribute("2", desc: "How many penalty points get substracted after each subtraction period.")]
	protected int m_iOptionalKickingPenaltySubtractionPoints;
	
	protected ref SCR_LocalPlayerPenalty m_OptionalKicking;

	protected ref map<int, ref SCR_PlayerData> m_mPlayerData = new map<int, ref SCR_PlayerData>();

	protected ref SCR_SessionData m_SessionData;

	protected IEntity m_Owner;

	protected ref SCR_DataCollectorUI m_UiHandler;

	protected ref map<FactionKey, ref array<float>> m_mFactionScore = new map<FactionKey, ref array<float>>();

#ifdef ENABLE_DIAG
	protected bool m_bLocalEntityListening = false;
	protected int m_iInitializingTimer = 0;
	protected bool m_bVisualDisplay = false;
#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnGameModeEnd();
		}
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerID;
		PlayerController playerController;
		SCR_DataCollectorCommunicationComponent communicationComponent;

		// Here we add to the faction the scores of all the players who haven't disconnected yet
		SCR_ChimeraCharacter playerChimera;
		Faction faction;

		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			playerID = m_mPlayerData.GetKey(i);

			// We update the duration of the session here because it should not be connected to any module
			m_mPlayerData.Get(playerID).CalculateSessionDuration();

			playerChimera = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
			if (!playerChimera)
				continue;

			faction = playerChimera.GetFaction();

			if (!faction)
				continue;

			AddStatsToFaction(faction.GetFactionKey(), m_mPlayerData.Get(playerID).CalculateStatsDifference());
		}

		// We replicate the faction stats now, so they can be found in the client's machine
		array<FactionKey> factionKeys = {};
		array<float> factionValues = {};
		int valuesSize = 0;

		foreach (FactionKey key, array<float> value : m_mFactionScore)
		{
			factionKeys.Insert(key);
			factionValues.InsertAll(value);
			if (valuesSize == 0)
				valuesSize = value.Count();
		}

		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			playerID = m_mPlayerData.GetKey(i);
			playerController = playerManager.GetPlayerController(playerID);
			if (!playerController)
				continue;

			communicationComponent = SCR_DataCollectorCommunicationComponent.Cast(playerController.FindComponent(SCR_DataCollectorCommunicationComponent));
			if (!communicationComponent)
				continue;

			communicationComponent.SendData(m_mPlayerData.Get(playerID), factionKeys, factionValues, valuesSize);
		}

		// Get Session data
		SCR_SessionDataEvent dataEvent = m_SessionData.GetSessionDataEvent();

		dataEvent.name_reason_end = SCR_Enum.GetEnumName(EGameOverTypes, data.GetEndReason());

		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			Faction winFaction = factionManager.GetFactionByIndex(data.GetWinnerFactionId());
			if (winFaction)
				dataEvent.name_winner_faction = winFaction.GetFactionKey();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_EDataStats of all players belonging to a faction added
	//! Some of those stats don't make sense as they are (ie. Rank, LevelOfExperience)
	//! Bear that in mind when using these
	//! \param[in] key
	//! \return
	array<float> GetFactionStats(FactionKey key)
	{
		return m_mFactionScore.Get(key);
	}

	//------------------------------------------------------------------------------------------------
	//! GetAllFactionStats can be used in weird user cases
	//! Most of the time GetFactionStats should be enough
	//! Also, I am returning the pointer to the original attribute, so technically, this is unsafe. May make safer (or remove) in the future.
	//! Use responsibly!
	//! \return
	map<FactionKey, ref array<float>> GetAllFactionStats()
	{
		return m_mFactionScore;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] key
	//! \param[in] stats
	void AddStatsToFaction(FactionKey key, notnull array<float> stats)
	{
		array<float> factionStats = m_mFactionScore.Get(key);
		//If the faction doesn't exist in our map yet, just create it and initialize it with these stats
		if (!factionStats)
		{
			m_mFactionScore.Insert(key, stats);
		}
		else
		{
			int statsCount = stats.Count();

			if (statsCount == 0 || factionStats.Count() != statsCount)
			{
				Print("ERROR WHEN ADDING FACTIONSTATS IN DATA COLLECTOR: Size of faction stats is different than expected size! Expected size was" + statsCount + " but real size was "+ factionStats.Count(), LogLevel.WARNING);
				return;
			}

			for (int i = 0; i < statsCount; i++)
			{
				factionStats[i] = factionStats[i] + stats[i];
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! When game shuts down, store the profile of every player who hasn't disconnected yet
	override void OnGameEnd()
	{	
		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			SCR_PlayerData playerData = GetPlayerData(m_mPlayerData.GetKey(i), false);
			if (playerData)
				playerData.StoreProfile();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] type
	//! \return found module or null if not found
	SCR_DataCollectorModule FindModule(typename type)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			if (m_aModules[i].Type() == type)
				return m_aModules[i];
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMaster()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return (rplComponent && rplComponent.IsMaster());
	}

	//------------------------------------------------------------------------------------------------
	Managed GetSessionDataStats()
	{
		if (!m_SessionData)
			return null;

		return m_SessionData.GetDataEventStats();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	Managed GetPlayerDataStats(int playerID)
	{
		SCR_PlayerData playerData = GetPlayerData(playerID);

		return playerData.GetDataEventStats();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	void RemovePlayer(int playerID)
	{
		m_mPlayerData.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] createNew
	//! \param[in] requestFromBackend
	//! \return
	SCR_PlayerData GetPlayerData(int playerID, bool createNew = true, bool requestFromBackend = true)
	{
		SCR_PlayerData playerData = m_mPlayerData.Get(playerID);
		if (!playerData && createNew)
		{
			playerData = new SCR_PlayerData(playerID, true, requestFromBackend);
			m_mPlayerData.Insert(playerID, playerData);
		}

		return playerData;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayers(out notnull array<int> outPlayers)
	{
		if (m_mPlayerData.IsEmpty())
			return 0;

		for (int i = m_mPlayerData.Count() - 1; i >= 0; i--)
		{
			outPlayers.Insert(m_mPlayerData.GetKey(i));
		}

		return m_mPlayerData.Count();
	}

	//------------------------------------------------------------------------------------------------
	SCR_SessionData GetSessionData()
	{
		return m_SessionData;
	}

	//------------------------------------------------------------------------------------------------
	// OnAuditSuccess is the moment when the player has not only connected, but also been authenticated
	protected override void OnPlayerAuditSuccess(int playerId)
	{
		Print("Player with id " + playerId + " was auditted succesfully and admitted on the Data Collector", LogLevel.DEBUG);
		//We create the player's PlayerData here
		GetPlayerData(playerId);

		//And then let the modules handle the newly connected player if they need to
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnPlayerAuditSuccess(playerId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerConnected(int playerId)
	{
		if (m_bOptionalKicking)
			m_OptionalKicking.OnPlayerConnected(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		SCR_PlayerData playerDisconnectedData = GetPlayerData(playerId);
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnPlayerDisconnected(playerId);
		}

		playerDisconnectedData.StoreProfile();

		// ADD STATS TO FACTION
		// Here we add the stats of the individual player who desconnected to the faction
		// We only do that if the game is not in POSTGAME state, because if it is we already added this player's stats to the faction in the OnGameModeEnd method

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode.GetState() != SCR_EGameModeState.POSTGAME)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			SCR_ChimeraCharacter playerChimera = SCR_ChimeraCharacter.Cast(player);
			if (playerChimera)
			{
				Faction faction = playerChimera.GetFaction();
				if (faction)
					AddStatsToFaction(faction.GetFactionKey(), playerDisconnectedData.CalculateStatsDifference());
			}
		}

		// DONE ADDING STATS TO THE FACTION
		//We cannot remove this instance of data from the player collector because the event has not been sent yet to the Database for tracking purposes
		//m_mPlayerData.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		int playerId = requestComponent.GetPlayerId();
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnPlayerSpawned(playerId, entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		int playerId = instigatorContextData.GetVictimPlayerID();
		IEntity playerEntity = instigatorContextData.GetVictimEntity();
		IEntity killerEntity = instigatorContextData.GetKillerEntity();
		Instigator instigator = instigatorContextData.GetInstigator();
		
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnPlayerKilled(playerId, playerEntity, killerEntity, instigator, instigatorContextData);
		}
		
		if (m_bOptionalKicking)
			m_OptionalKicking.OnControllableDestroyed(playerEntity, killerEntity, instigator, instigatorContextData);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAIKilled(IEntity AIEntity, IEntity killerEntity, notnull Instigator instigator, notnull SCR_InstigatorContextData instigatorContextData)
	{
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnAIKilled(AIEntity, killerEntity, instigator, instigatorContextData);
		}
		
		if (m_bOptionalKicking)
			m_OptionalKicking.OnControllableDestroyed(AIEntity, killerEntity, instigator, instigatorContextData);
	}

	//------------------------------------------------------------------------------------------------
	// This method is a hack to process killings when the dead entity is an AI
	// Because there is no "OnAiKilled" method
	protected override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		IEntity entity = instigatorContextData.GetVictimEntity();
		
		if (!SCR_ChimeraCharacter.Cast(entity))
		{
			// Spoiler - it was a vehicle, not an error
			// Print("Error: The OnControllableDestroyed method from the Data Collector was invoked with an IEntity that is not a chimera character."
			// + "Dead entity is" + entity +", and killerEntity is "+killerEntity, LogLevel.ERROR);
			// PrintFormat("Dead: %1", entity);
			// PrintFormat("KillerEntity: %1", killerEntity);
			return;
		}

		//If playerId is not 0 it means that the entity killed was a player
		//Therefore it will be handled by the OnPlayerKilled event
		//so we don't need to do anything else
		if (instigatorContextData.GetVictimPlayerID() > 0)
			return;

		OnAIKilled(entity, instigatorContextData.GetKillerEntity(), instigatorContextData.GetInstigator(), instigatorContextData);
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//! \param[in] from
	//! \param[in] to
	void OnPlayerEntityChanged(IEntity from, IEntity to)
	{
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.OnControlledEntityChanged(from, to);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! To debug this component we need to listen to the OnEntityChanged invoker from the local player
	protected void ListenToLocalControllerEntityChanged()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (playerController)
		{
			m_bLocalEntityListening = true;
			playerController.m_OnControlledEntityChanged.Insert(OnPlayerEntityChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Prototyping method. ENABLE_DIAG CLI or #define required
	protected void CreateStatVisualizations()
	{
		if (!m_UiHandler)
			m_UiHandler = new SCR_DataCollectorUI();

		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].CreateVisualization();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_DataCollectorUI GetUIHandler()
	{
		return m_UiHandler;
	}
#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeStart()
	{
		StartDataCollectorSession();
		m_SessionData = new SCR_SessionData();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use this method to disable all the modules.
	protected void DisableModules()
	{
		m_aModules.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! We call StartDataCollectorSession when the backend session is ready (OnGameModeStart)
	//! because that's the time when we can check whether the server has writing privileges
	//! If the server has no writing privileges, we don't bother tracking their performance
	void StartDataCollectorSession()
	{
		//If it's no server, disable all tracking
		if (!IsMaster())
		{
#ifndef ENABLE_DIAG
			DisableModules();
			m_bOptionalKicking = false;
			return;
#endif
		}
		
		//Init all modules
		foreach (SCR_DataCollectorModule module : m_aModules)
		{
			module.InitModule();
		}
		
		bool writingRights = false;
		BackendApi ba = GetGame().GetBackendApi();
		
		if (ba)
		{
			SessionStorage baStorage = ba.GetStorage();
			if (baStorage)
				writingRights = baStorage.GetOnlineWritePrivilege();
		}
		
		//Local storage or online backend storage?
		if (!writingRights)
			Print("DataCollectorComponent: StartDataCollectorSession: This server has no writing privileges. Will use local storage instead.", LogLevel.DEBUG);
		else
			Print("DataCollectorComponent: StartDataCollectorSession: Using online backend storage.", LogLevel.DEBUG);
		
		if (!m_Owner)
			Print("DataCollectorComponent: StartDataCollectorSession: m_Owner is null. Can't add the EntityEvent.FRAME flag thus data collector will not work properly.", LogLevel.ERROR);
		else
			SetEventMask(m_Owner, EntityEvent.FRAME); //Activate the FRAME flag
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
#ifdef ENABLE_DIAG
		
		// Hotfix: AND I AM DISABLING THIS, because it causes other invokers to be duplicit after respawn
		// TODO: Make something more serious
		/*
		//I am doing this because the OnAudit invoker is only used on server-side
		//Here we want to be able to debug the tracking of career stuff on client-side
		//since there's no event for it, we need to check periodically until
		//a local player controller is found
		if (!m_bLocalEntityListening)
		{
			if (m_iInitializingTimer % 100)
			{
				ListenToLocalControllerEntityChanged();
			}
			m_iInitializingTimer++;
		}
		*/

		if (m_bVisualDisplay != DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_DATA_COLLECTION_ENABLE_DIAG))
		{
			if (!m_UiHandler)
				CreateStatVisualizations();

			m_bVisualDisplay = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_DATA_COLLECTION_ENABLE_DIAG);
			m_UiHandler.SetVisible(m_bVisualDisplay);
		}
#endif

		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			m_aModules[i].Update(timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		//If there is a data collector instance already, return
		if (GetGame().GetDataCollector())
			return;
		
		m_OptionalKicking = new SCR_LocalPlayerPenalty(m_iOptionalKickingFriendlyPlayerKillPenalty, m_iOptionalKickingFriendlyAIKillPenalty, m_iOptionalKickingKickPenaltyLimit, m_iOptionalKickingBanDuration, m_iOptionalKickingPenaltySubtractionPeriod, m_iOptionalKickingPenaltySubtractionPoints);

		//Register the data collector
		GetGame().RegisterDataCollector(this);

		m_Owner = owner;
	}
}
