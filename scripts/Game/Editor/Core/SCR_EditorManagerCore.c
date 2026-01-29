//! @ingroup Editor_Core GameCore

//! Core component to manage SCR_EditorManagerEntity.
[BaseContainerProps(configRoot: true)]
class SCR_EditorManagerCore : SCR_GameCoreBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Individual editor manager", "et")]
	private ResourceName m_EditorManagerPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, "Server entity prefab", "et")]
	private ResourceName m_EditorServerPrefab;
	
	[Attribute(desc: "Editor mode prefabs")]
	private ref array<ref SCR_EditorModePrefab> m_ModePrefabs;
	
	[Attribute("300", desc: "When player disconnects, editor modes will be restored if the player reconnects with this timeout (in seconds).")]
	protected int m_iRestoreOnReconnectTimeout;
	
	private SCR_EditorManagerEntity m_EditorManager;
	private ref map<int, SCR_EditorManagerEntity> m_aEditorEntities;
	private SCR_EditorSettingsEntity m_SettingsEntity;
	private SCR_EditorServerEntity m_ServerEntity;
	private ref SCR_SortedArray<SCR_EditorModePrefab> m_ModePrefabsSorted = new SCR_SortedArray<SCR_EditorModePrefab>;
	private ref array<ref SCR_EditorManagerDisconnectData> m_DisconnectedModes;
	
	//! Called when an editor manager is created after a player connects
	//! \param editorManager Created editor manager
	ref ScriptInvoker Event_OnEditorManagerCreatedServer = new ScriptInvoker;

	//! Called when an editor manager is deleted after a player disconnects
	//! \param editorManager Deleted editor manager
	ref ScriptInvoker Event_OnEditorManagerDeletedServer = new ScriptInvoker;

	//! Called when an editor manager is initialized on owner's machine
	//! \param editorManager Created editor manager
	ref ScriptInvoker Event_OnEditorManagerInitOwner = new ScriptInvoker;

	//! Called when user sends a ping.
	//! \param playerID ID of player who sent the ping
	ref ScriptInvoker Event_OnEditorManagerPing = new ScriptInvoker;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Custom functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected SCR_EditorManagerEntity CreateEditorManager(int playerID)
	{
		if (RplSession.Mode() == RplMode.Client || (m_SettingsEntity && m_SettingsEntity.IsEditorDisabled()))
			return null;
		
		if (m_aEditorEntities && m_aEditorEntities.Contains(playerID))
		{
			Debug.Error2("SCR_EditorManagerCore.CreateEditorManager()", string.Format("Attempt for duplicate registration of editor manager for playerID = %1!", playerID));
			return null;
		}
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
		{
			Debug.Error2("SCR_EditorManagerCore.CreateEditorManager()", string.Format("Cannot create editor manager entity, PlayerController for playerID = %1 does not exist yet!", playerID));
			return null;
		}
		
		//--- Get player replication ID (not identical to player ID!)
		RplIdentity playerRplID = playerController.GetRplIdentity();
		
		//--- Check for duplicates, exit when found
		for (int i = 0; i < m_aEditorEntities.Count(); i++)
		{
			PlayerController compareController = GetGame().GetPlayerManager().GetPlayerController(m_aEditorEntities.GetKey(i));
			if (compareController && compareController.GetRplIdentity() == playerRplID)
			{
				Debug.Error2("SCR_EditorManagerCore.CreateEditorManager()", string.Format("Cannot register editor manager for playerID = %1 because manager for playerID = %2 is already registered on the same machine!", playerID, m_aEditorEntities.GetKey(i)));
				return null;
			}
		}
		
		//--- Create the entity
		ResourceName prefab = m_EditorManagerPrefab;
		if (m_SettingsEntity)
			prefab = m_SettingsEntity.GetPrefab(m_EditorManagerPrefab);

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab)));
		if (!editorManager)
		{
			Debug.Error2("SCR_EditorManagerCore.CreateEditorManager()", string.Format("Prefab %1 is not SCR_EditorManagerEntity!", prefab.GetPath()));
			delete editorManager;
			return null;
		}
		
		//--- Set entity ownership (not for editor of host player)
		if (playerRplID != RplIdentity.Local())
		{
			RplComponent rpl = RplComponent.Cast(editorManager.FindComponent(RplComponent));
			if (!rpl)
			{
				Debug.Error2("SCR_EditorManagerCore.CreateEditorManager()", string.Format("Prefab %1 is missing RplComponent component!", prefab.GetPath()));
				delete editorManager;
				return null;
			}
			rpl.Give(playerRplID);
		}
		m_aEditorEntities.Insert(playerID, editorManager);
		
		//--- Initialize the entity
		editorManager.InitServer(playerID);
		Event_OnEditorManagerCreatedServer.Invoke(editorManager);
		
		//--- Get active player roles
		EPlayerRole playerRoles;
		array<EPlayerRole> allRoles = {};
		for(int i, count = SCR_Enum.GetEnumValues(EPlayerRole, allRoles); i < count; i++)
		{
			if (GetGame().GetPlayerManager().HasPlayerRole(playerID, allRoles[i]))
				playerRoles |= allRoles[i];
		}
		OnPlayerRoleChange(playerID, playerRoles);
		
		//--- Set base editor modes. If player reconnects, restore their previous modes.
		EEditorMode modes = GetBaseModes(EEditorModeFlag.DEFAULT);
		
		//--- Restore modes player had before disconnecting
		if (m_DisconnectedModes)
		{
			ChimeraWorld world = GetGame().GetWorld();
			WorldTimestamp currentTime = world.GetServerTimestamp();
			for (int i = m_DisconnectedModes.Count() - 1; i >= 0; i--)
			{
				if (currentTime.Greater(m_DisconnectedModes[i].m_Time.PlusSeconds(m_iRestoreOnReconnectTimeout)))
				{
					//--- Expired, ignore
					m_DisconnectedModes.Remove(i);
				}
				else if (m_DisconnectedModes[i].m_iPlayerID == playerID)
				{
					//--- Not expired, restore modes
					modes = m_DisconnectedModes[i].m_Modes;
					m_DisconnectedModes.Remove(i);
				}
			}
			if (m_DisconnectedModes.IsEmpty())
				m_DisconnectedModes = null;
		}
		
		editorManager.AddEditorModes(EEditorModeAccess.BASE, modes, true);
		
		//--- Finish initualization
		editorManager.PostInitServer();

		return editorManager;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteEditorManager(int playerID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		if (m_aEditorEntities.Contains(playerID))
		{
			SCR_EditorManagerEntity editorManager = m_aEditorEntities.Get(playerID);
			
			//--- Remember editor modes so they can be restored upon reconnection
			if (!m_DisconnectedModes)
				m_DisconnectedModes = {};
			
			bool isAdmin = SCR_Global.IsAdmin(playerID);
			
			EEditorMode modes = editorManager.GetModes();
			EEditorMode persistentModes;
			foreach (SCR_EditorModePrefab prefab: m_ModePrefabs)
			{
				if (
					SCR_Enum.HasFlag(modes, prefab.GetMode()) //--- Player has the mode
					&& SCR_Enum.HasFlag(prefab.GetFlags(), EEditorModeFlag.PERSISTENT) //--- Mode is marked as persistent
					&& (!isAdmin || !SCR_Enum.HasFlag(prefab.GetFlags(), EEditorModeFlag.ADMIN)) //--- If player is admin, don't preserve modes added with admin rights
				)
				{
					persistentModes |= prefab.GetMode();
				}
			}
			
			if (persistentModes != 0)
				m_DisconnectedModes.Insert(new SCR_EditorManagerDisconnectData(playerID, persistentModes));
		
			m_aEditorEntities.Remove(playerID);
			Event_OnEditorManagerDeletedServer.Invoke(editorManager);
			delete editorManager;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get the list of all editor entities
	//! \param[out] outEditorEntities Array of editor entities
	int GetEditorEntities(out notnull array<SCR_EditorManagerEntity> outEditorEntities)
	{
		outEditorEntities.Clear();
		if (m_aEditorEntities)
		{
			for (int i = 0; i < m_aEditorEntities.Count(); i++)
			{
				SCR_EditorManagerEntity editorManager = m_aEditorEntities.GetElement(i);
				if (editorManager)
					outEditorEntities.Insert(editorManager);
			}
		}
		return outEditorEntities.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the local instance of the editor manager. Shows an error when it already exists.
	//! \param entity Editor manager
	bool SetEditorManager(SCR_EditorManagerEntity entity)
	{
		if (m_EditorManager)
		{
			Print("Attempting to define multiple instances of SCR_EditorManagerEntity!", LogLevel.ERROR);
			return false;
		}
		m_EditorManager = entity;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get local instance of the editor manager.
	//! \return Editor manager
	SCR_EditorManagerEntity GetEditorManager()
	{
		return m_EditorManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Get editor manager of given player
	//! \param playerID Player ID
	//! \return Editor manager
	SCR_EditorManagerEntity GetEditorManager(int playerID)
	{		
		if (m_aEditorEntities && m_aEditorEntities.Contains(playerID))
			return m_aEditorEntities.Get(playerID);
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get editor server entity.
	//! \return Editor server entity
	SCR_EditorServerEntity GetEditorServer()
	{
		if (Replication.IsClient())
		{
			Debug.Error2("SCR_EditorManagerCore.GetEditorServer()", "Editor server entity is available only on server!");
			return null;
		}
		return m_ServerEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign entity with world-specific editor settings.
	//! Will not do anything if settings are already assigned.
	//! \param entity Settings entity
	void SetSettingsEntity(SCR_EditorSettingsEntity entity)
	{
		if (m_SettingsEntity)
			return;

		m_SettingsEntity = entity;
	}

	//------------------------------------------------------------------------------------------------
	//! Get world-specific editor settings entity.
	//! \return Settings entity
	SCR_EditorSettingsEntity GetSettingsEntity()
	{
		return m_SettingsEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get base mode prefab.
	//! \param modeType Mode type
	//! \return SCR_EditorModePrefab
	SCR_EditorModePrefab GetBaseModePrefab(EEditorMode modeType)
	{
		array<SCR_EditorModePrefab> prefabs = {};
		GetBaseModePrefabs(prefabs);
		foreach (SCR_EditorModePrefab modePrefab: prefabs)
		{
			if (modePrefab.GetMode() == modeType)
				return modePrefab;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all base mode types.
	//! \param flags Flags filter
	//! \param coreOnly When true, base mode prefab override on SCR_EditorSettingsEntity will be ignored
	//! \return Values of base mode types
	EEditorMode GetBaseModes(EEditorModeFlag flags = -1, bool coreOnly = false)
	{
		EEditorMode modes;
		array<SCR_EditorModePrefab> prefabs = {};
		for (int i, count = GetBaseModePrefabs(prefabs, flags, coreOnly); i < count; i++)
		{
			modes |= prefabs[i].GetMode();
		}
		return modes;
	}

	//------------------------------------------------------------------------------------------------
	//! Get prefabs of all base mode types.
	//! \param flags Flags filter
	//! \param coreOnly When true, base mode prefab override on SCR_EditorSettingsEntity will be ignored
	//! \return Prefabs of base mode types
	int GetBaseModePrefabs(out notnull array<SCR_EditorModePrefab> outPrefabs, EEditorModeFlag flags = -1, bool coreOnly = false)
	{
		EEditorMode defaultModes;
		bool isOverride = !coreOnly && m_SettingsEntity && m_SettingsEntity.GetBaseModes(defaultModes);
		
		int count = m_ModePrefabs.Count();
		foreach (SCR_EditorModePrefab prefab: m_ModePrefabs)
		{
			if (isOverride)
			{
				//--- Override, use mode even when flags are not matching
				if (SCR_Enum.HasFlag(defaultModes, prefab.GetMode()))
					outPrefabs.Insert(prefab);
			}
			else
			{
				//--- No override, check by flags
				if (flags == -1 || SCR_Enum.HasPartialFlag(prefab.GetFlags(), flags))
					outPrefabs.Insert(prefab);
			}
		}
		return outPrefabs.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get sorted prefabs of all base mode types.
	//! \param flags Flags filter
	//! \param coreOnly When true, base mode prefab override on SCR_EditorSettingsEntity will be ignored
	//! \return Sorted prefabs of base mode types
	int GetBaseModePrefabs(out notnull SCR_SortedArray<SCR_EditorModePrefab> outPrefabs, EEditorModeFlag flags = -1, bool coreOnly = false)
	{
		EEditorMode defaultModes;
		bool isOverride = !coreOnly && m_SettingsEntity && m_SettingsEntity.GetBaseModes(defaultModes);
		
		SCR_EditorModePrefab prefab;
		int count = m_ModePrefabsSorted.Count();
		for (int i; i < count; i++)
		{
			prefab = m_ModePrefabsSorted[i];
			if (
				(!isOverride || SCR_Enum.HasFlag(defaultModes, prefab.GetMode()))
				&& (flags == -1 || SCR_Enum.HasPartialFlag(prefab.GetFlags(), flags))
			)
			{
				outPrefabs.Insert(m_ModePrefabsSorted.GetOrder(i), prefab);
			}
		}
		return outPrefabs.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get UI info of given default mode.
	//! \param modeType Mode type
	//! \return UI info
	SCR_EditorModeUIInfo GetDefaultModeInfo(EEditorMode modeType)
	{
		foreach (SCR_EditorModePrefab modeEntity: m_ModePrefabs)
		{
			if (modeEntity.GetMode() == modeType)
				return modeEntity.GetInfo();
		}
		return null;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Event handlers
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerRegistered(int playerID)
	{
		//--- Create and register editor manager
		CreateEditorManager(playerID);
		
		//--- If an entity was already created for player, initialize it (e.g., when there is no respawn menu)
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (controlledEntity)
			OnPlayerSpawn(playerID, controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDisconnected(int playerID)
	{
		//--- Delete and unregister editor manager
		DeleteEditorManager(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawn(int playerID, IEntity controlledEntity)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager(playerID);
		if (editorManager)
		{
			editorManager.SetCanOpen(true, EEditorCanOpen.ALIVE);
			editorManager.AutoInit();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager(instigatorContextData.GetVictimPlayerID());
		if (editorManager)
			editorManager.SetCanOpen(false, EEditorCanOpen.ALIVE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDeleted(int playerID, IEntity player)
	{
		OnPlayerKilled(new SCR_InstigatorContextData(playerID, player, null, Instigator.CreateInstigatorGM(), true));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerRoleChange(int playerId, EPlayerRole roleFlags)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager(playerId);
		if (editorManager)
		{
			EEditorModeFlag flags;
		
			//--- Game running in Workbench and player ID is host
#ifdef WORKBENCH
			if (playerId == SCR_PlayerController.GetLocalPlayerId())
				flags |= EEditorModeFlag.WORKBENCH;
#endif
			//--- User is admin in multiplayer (suppress it in SP to avoid MP-specific modes)
			if (SCR_Global.IsAdminRole(roleFlags) && Replication.IsRunning())
				flags |= EEditorModeFlag.ADMIN;
			
			EEditorMode modes = GetBaseModes(flags, true);
			editorManager.SetEditorModes(EEditorModeAccess.ROLE, modes, false);
		}
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayFromCameraPos()
	{
		//--- Ignore when the world contains game mode, player is registered by it alrady
		if (GetGame().GetGameMode())
			return;
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
		{
			int playerID = playerController.GetPlayerId();
			if (playerID > 0)
			{
				CreateEditorManager(playerID);
				return;
			}
			//--- Player not initialized yet, try again next frame
			GetGame().GetCallqueue().CallLater(OnPlayFromCameraPos, 1);
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	protected void OnReconnectOnPreviousCharacter(notnull SCR_ReconnectData reconnectData)
	{
		int playerID = reconnectData.m_iPlayerId;
		
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (controlledEntity)
			OnPlayerSpawn(playerID, controlledEntity);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default Functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void OnGameStart()
	{		
		//--- Server only
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		//--- Create server entity
		m_ServerEntity = SCR_EditorServerEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_EditorServerPrefab)));
		
		//--- Create settings entity, unless it exists in the world
		if (!m_SettingsEntity)
		{
			m_SettingsEntity = SCR_EditorSettingsEntity.Cast(GetGame().SpawnEntity(SCR_EditorSettingsEntity));
			m_SettingsEntity.SetBaseModes(GetBaseModes(EEditorModeFlag.DEFAULT, true), false);
			//m_SettingsEntity.EnableBaseOverride(true);
		}
		
		//--- Initialize entity list (ignore if already done)
		if (m_aEditorEntities)
			return;

		m_aEditorEntities = new map<int, SCR_EditorManagerEntity>();
				
		//--- Get lobby
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			//--- Add game mode event handlers
			gameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
			gameMode.GetOnPostCompPlayerDisconnected().Insert(OnPlayerDisconnected);
			gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawn);
			gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
			gameMode.GetOnPlayerRoleChange().Insert(OnPlayerRoleChange);
		}
		else
		{ 
			//--- No game mode detected, run on local player
			PlayerController controller = GetGame().GetPlayerController();
			if (controller && controller.GetPlayerId() > 0) 
			{
				OnPlayerRegistered(controller.GetPlayerId());
			}
			else
			{
				//--- No local player found, force the editor
				//OnPlayerRegistered(-1); //--- Disabled, there should be a chance to have an empty world for debugging purposes
				return;
			}
		}
		
		// Listen to reconnect 
		if (SCR_ReconnectComponent.GetInstance())
			SCR_ReconnectComponent.GetInstance().GetOnReconnect().Insert(OnReconnectOnPreviousCharacter);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		m_EditorManager = null;
		m_aEditorEntities = null;
		Event_OnEditorManagerCreatedServer = new ScriptInvoker;
		Event_OnEditorManagerDeletedServer = new ScriptInvoker;
		Event_OnEditorManagerInitOwner = new ScriptInvoker;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditorManagerCore()
	{
		//--- Cache array of modes sorted by custom order
		foreach (SCR_EditorModePrefab prefabData: m_ModePrefabs)
		{
			m_ModePrefabsSorted.Insert(prefabData.GetInfo().GetOrder(), prefabData);
		}
		
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_EDITOR, "Editor", "");
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EditorManagerCore()
	{
		Event_OnEditorManagerCreatedServer = null;
		Event_OnEditorManagerDeletedServer = null;
		Event_OnEditorManagerInitOwner = null;
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR);
	}
}

class SCR_EditorManagerDisconnectData
{
	int m_iPlayerID;
	WorldTimestamp m_Time;
	EEditorMode m_Modes;

	//------------------------------------------------------------------------------------------------
	void SCR_EditorManagerDisconnectData(int playerID, EEditorMode modes)
	{
		m_iPlayerID = playerID;
		m_Modes = modes;
		ChimeraWorld world = GetGame().GetWorld();
		m_Time = world.GetServerTimestamp(); //--- Can use absolute value, we don't need precision
	}
}
