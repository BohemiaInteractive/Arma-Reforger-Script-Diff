[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PlayersManagerEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
Manager of editable entities which players control (i.e., their avatars).
*/
class SCR_PlayersManagerEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_DefaultPlayer;
	
	protected ref map<int, SCR_EditableEntityComponent> m_MainEntities;
	protected ref map<int, SCR_EditableEntityComponent> m_PossessedEntities;
	
	protected bool m_LocalPlayerHasSpawnedOnce = false;
	
	protected ref ScriptInvoker Event_OnDeath = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnSpawn = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnPossessed = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnConnected = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnDisconnected = new ScriptInvoker;
	
	/*!
	Get editable entity of given player ID.
	\param playerID Player ID
	\return Editable entity. Whent he player has none, null is returned.
	*/
	SCR_EditableEntityComponent GetPlayerEntity(int playerID)
	{
		if (!m_MainEntities) return null;
		SCR_EditableEntityComponent entity = null;
		m_MainEntities.Find(playerID, entity);
		return entity;
	}
	/*!
	If given editable entity is a player, get its ID.
	\param entity Editable entity
	\return Player ID. 0 is returned when the entity is not a player.
	*/
	int GetPlayerID(SCR_EditableEntityComponent entity)
	{
		if (!m_MainEntities)
			return 0;
		
		int playerID = SCR_MapHelper<int, SCR_EditableEntityComponent>.GetKeyByValue(m_MainEntities,entity);
		if (playerID != 0)
			return playerID;
		else
			return SCR_MapHelper<int, SCR_EditableEntityComponent>.GetKeyByValue(m_PossessedEntities,entity);
	}
	bool IsPossessed(SCR_EditableEntityComponent entity)
	{
		return SCR_MapHelper<int, SCR_EditableEntityComponent>.GetKeyByValue(m_PossessedEntities,entity) != 0;
	}
	/*!
	Extract a list of all players.
	\param players Map to be filled with player IDs and editable entities
	\return Number of entries
	*/
	int GetPlayers(notnull map<int, SCR_EditableEntityComponent> players)
	{
		if (!m_MainEntities)
		{
			players.Clear();
			return 0;
		}
		players.Copy(m_MainEntities);
		return m_MainEntities.Count(); 
	}
	/*!
	Get number of players.
	\return Number of players
	*/
	int GetPlayersCount()
	{
		return m_MainEntities.Count();
	}
	
	void TeleportPlayerToPosition(vector position)
	{
		SCR_StatesEditorComponent statesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		if (statesManager && statesManager.GetState() != EEditorState.SELECTING)
			return;
		
		Rpc(RPC_TeleportPlayerToPositionServer, position);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_TeleportPlayerToPositionServer(vector position)
	{
		// Verify player has editor open
		SCR_EditorManagerEntity editorManager = GetManager();
		if (!editorManager || !editorManager.IsOpened())
			return;
		
		IEntity player = SCR_PossessingManagerComponent.GetPlayerMainEntity(editorManager.GetPlayerID());
		if (!player)
			return;
		
		IEntity parent = player.GetRootParent();
		if (parent == player)
			// Player not attached to anything, execute where the player is local
			Rpc(TeleportPlayerToPositionOwner, editorManager.GetPlayerID(), position); 
		else
			// Player attached to something, execute on the server
			TeleportPlayerToPositionServer(player, editorManager.GetPlayerID(), position);
	}
	
	void TeleportPlayerToPositionServer(IEntity player, int playerId, vector position)
	{
		if (!SCR_Global.TeleportPlayer(playerId, position))
			return;
		
		/* Commented because the owner would not get instant reponse.
			 The original idea was to let the NwkMovementComponent replicate the change.

		// If the player is inside a vehicle we don't need to broadcast if NwkMovementComponent is present and active.
		// It will take care of the sync.
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccess)
		{
			IEntity vehicle = compartmentAccess.GetVehicle();
			if (vehicle)
			{
				NwkMovementComponent nwk = NwkMovementComponent.Cast(vehicle.FindComponent(NwkMovementComponent));
				if (nwk != null && nwk.IsSimulationEnabled())
					return;
			}
		}*/
		
		Rpc(TeleportPlayerToPositionBroadcast, playerId, position)
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void TeleportPlayerToPositionBroadcast(int playerId, vector position)
	{
		SCR_Global.TeleportPlayer(playerId, position);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void TeleportPlayerToPositionOwner(int playerId, vector position)
	{
		SCR_Global.TeleportPlayer(playerId, position);
	}
	
	/*!
	Get m_LocalPlayerHasSpawnedOnce to find out if local player has spawned at least once
	\return m_LocalPlayerHasSpawnedOnce
	*/
	bool HasLocalPlayerSpawnedOnce()
	{
		return m_LocalPlayerHasSpawnedOnce;
	}
	
	/*!
	Get script invoker called locally when any player dies.
	\return Script invoker
	*/
	ScriptInvoker GetOnDeath()
	{
		return Event_OnDeath;
	}
	/*!
	Get script invoker called locally when any player (re)spawns.
	\return Script invoker
	*/
	ScriptInvoker GetOnSpawn()
	{
		return Event_OnSpawn;
	}
	/*!
	Get script invoker called locally when any player starts/stops possessing an entity.
	\return Script invoker
	*/
	ScriptInvoker GetOnPossessed()
	{
		return Event_OnPossessed;
	}
	/*!
	Get script invoker called locally when any player connects.
	\return Script invoker
	*/
	ScriptInvoker GetOnConnected()
	{
		return Event_OnConnected;
	}
	/*!
	Get script invoker called locally when any player disconnects.
	\return Script invoker
	*/
	ScriptInvoker GetOnDisconnected()
	{
		return Event_OnDisconnected;
	}
	
	/*!
	Print out all players' editable entities.
	*/
	void Log()
	{
		if (!m_MainEntities) return;
		Print("--------------------------------------------------", LogLevel.DEBUG);
		Print(string.Format("--- Players (%2)", m_MainEntities.Count()), LogLevel.DEBUG);
		for (int i = 0; i < m_MainEntities.Count(); i++)
		{
			Print("Player ID: " + m_MainEntities.GetKey(i).ToString(), LogLevel.DEBUG);
			
			SCR_EditableEntityComponent entity = null;
			if (m_MainEntities.Find(m_MainEntities.GetKey(i), entity))
			{
				entity.Log("", true);
			}
			else
			{
				Print("Entity not found!", LogLevel.WARNING);
			}
		}
		Print("--------------------------------------------------", LogLevel.DEBUG);
	}
	
	protected void OnSpawnServer(int playerID, IEntity controlledEntity)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(controlledEntity);
		if (!entity)
			return;

		UpdatePlayerFaction(entity, controlledEntity);
		UpdatePlayerGroup(entity, controlledEntity, playerID);

		Rpc(OnSpawnOwner, playerID, Replication.FindId(entity));
	}

	protected void UpdatePlayerGroup(notnull SCR_EditableEntityComponent editableEntity, notnull IEntity controlledEntity, int playerID)
	{
		Faction entityFaction = editableEntity.GetFaction();
		
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (!controller)
			return;
		
		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent)
			return;
		
		if (groupComponent.GetGroupID() == -1)
			groupComponent.CreateAndJoinGroup(entityFaction);
	}
	
	protected void UpdatePlayerFaction(SCR_EditableEntityComponent editableEntity, IEntity controlledEntity)
	{
		FactionAffiliationComponent factionAffiliaton = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliaton)
			return;

		factionAffiliaton.SetAffiliatedFaction(factionAffiliaton.GetAffiliatedFaction());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void OnSpawnOwner(int playerID, int entityID)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity) return;
		
		//--- Get previous entity
		SCR_EditableEntityComponent entityPrev = null;
		m_MainEntities.Find(playerID, entityPrev);
		
		//--- No change, ignore
		if (entity == entityPrev) return;
		
		//Local player was spawned for the first time
		if (!m_LocalPlayerHasSpawnedOnce && SCR_PlayerController.GetLocalPlayerId() == playerID)
			m_LocalPlayerHasSpawnedOnce = true;
		
		//--- Set and send further
		m_MainEntities.Set(playerID, entity);
		Event_OnSpawn.Invoke(playerID, entity, entityPrev);
	}
	protected void OnPossessedServer(int playerID, IEntity controlledEntity, IEntity mainEntity, bool isPossessing)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(controlledEntity);
		if (!entity) return;

		Rpc(OnPossessedOwner, playerID, isPossessing, Replication.FindId(entity));
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void OnPossessedOwner(int playerID, bool isPossessing, int entityID)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity) return;
		
		if (isPossessing)
			m_PossessedEntities.Insert(playerID, entity);
		else
			m_PossessedEntities.Remove(playerID);
		
		Event_OnPossessed.Invoke(playerID, entity, isPossessing);
	}
	protected void OnDeathServer(notnull SCR_InstigatorContextData instigatorContextData)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(instigatorContextData.GetVictimEntity());
		if (!entity) 
			return;
		
		SCR_EditableEntityComponent killerEditableEntity = SCR_EditableEntityComponent.GetEditableEntity(instigatorContextData.GetKillerEntity());
		
		Rpc(OnDeathOwner, instigatorContextData.GetVictimPlayerID(), Replication.FindId(entity), Replication.FindId(killerEditableEntity));
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void OnDeathOwner(int playerID, int entityID, int killerID)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity) return;
		
		SCR_EditableEntityComponent killerEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(killerID));

		Event_OnDeath.Invoke(playerID, entity, killerEntity);
	}
	protected void OnConnectedServer(int playerID)
	{
		Rpc(OnConnectedOwner, playerID);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void OnConnectedOwner(int playerID)
	{
		m_MainEntities.Insert(playerID, null);
		Event_OnConnected.Invoke(playerID);
	}
	protected void OnDisconnectedServer(int playerID)
	{
		if (!Replication.IsRunning()) return;
		
		SCR_EditorBaseEntity owner = SCR_EditorBaseEntity.Cast(GetOwner());
		if (!owner) return;
		
		SCR_EditorManagerEntity manager = owner.GetManager();
		if (!manager) return;
		
		//--- Don't call the code on its own disconnection
		if (playerID != manager.GetPlayerID())
			Rpc(OnDisconnectedOwner, playerID);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void OnDisconnectedOwner(int playerID)
	{
		m_MainEntities.Remove(playerID);
		Event_OnDisconnected.Invoke(playerID);
	}
	protected void StopPosessing()
	{
		if (!GetManager())
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(GetManager().GetPlayerID()));
		
		if (playerController)
			playerController.SetPossessedEntity(null);
	}
	protected void OnLimitedChange(bool isLimited)
	{
		if (isLimited)
			StopPosessing();
	}
	protected void OnEntityRegistered(SCR_EditableEntityComponent entity)
	{
		//--- Update player entity when it's streamed in
		int playerID = entity.GetPlayerID();
		if (playerID != 0)
		{
			m_MainEntities.Set(playerID, entity);
			Event_OnSpawn.Invoke(playerID, entity, null);
		}
	}
	protected void SetServerHandler(bool toAdd)
	{
		if (RplSession.Mode() == RplMode.Client) return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		
		if (toAdd)
		{
			gameMode.GetOnPlayerSpawned().Insert(OnSpawnServer);
			gameMode.GetOnPlayerKilled().Insert(OnDeathServer);
			gameMode.GetOnPlayerConnected().Insert(OnConnectedServer);
			gameMode.GetOnPlayerDisconnected().Insert(OnDisconnectedServer);
			
			if (possessingManager)
				possessingManager.GetOnPossessed().Insert(OnPossessedServer);
		}
		else
		{
			gameMode.GetOnPlayerSpawned().Remove(OnSpawnServer);
			gameMode.GetOnPlayerKilled().Remove(OnDeathServer);
			gameMode.GetOnPlayerConnected().Remove(OnConnectedServer);
			gameMode.GetOnPlayerDisconnected().Remove(OnDisconnectedServer);
			
			if (possessingManager)
				possessingManager.GetOnPossessed().Remove(OnPossessedServer);
		}
	}
	override void EOnEditorOpen()
	{
		m_MainEntities = new map<int, SCR_EditableEntityComponent>;
		m_PossessedEntities = new map<int, SCR_EditableEntityComponent>;
		
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		
		//--- Add already registered players
		array<int> playerIDs = new array<int>;
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		IEntity owner;
		SCR_EditableEntityComponent entity;
		foreach (auto playerID: playerIDs)
		{
			owner = SCR_PossessingManagerComponent.GetPlayerMainEntity(playerID);
			entity = SCR_EditableEntityComponent.GetEditableEntity(owner);
			m_MainEntities.Insert(playerID, entity);
			
			if (possessingManager && possessingManager.IsPossessing(playerID))
			{
				owner = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
				entity = SCR_EditableEntityComponent.GetEditableEntity(owner);
				if (entity)
					m_PossessedEntities.Insert(playerID, entity);
			}
		}
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		core.Event_OnEntityRegistered.Insert(OnEntityRegistered);
	}
	override void EOnEditorClose()
	{
		m_MainEntities = null;
		m_PossessedEntities = null;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		core.Event_OnEntityRegistered.Remove(OnEntityRegistered);
	}
	
	override void EOnEditorOpenServerCallback()
	{
		SetServerHandler(true);
		
		//--- Stop possessing an entity upon opening the editor
		StopPosessing();
	}
	override void EOnEditorCloseServer()
	{
		SetServerHandler(false);
	}
	override void EOnEditorInit()
	{
		super.EOnEditorInit();
		
		if (SCR_PlayerController.GetLocalMainEntity())
			m_LocalPlayerHasSpawnedOnce = true;
	}
	override void EOnEditorInitServer()
	{
		//--- Stop possessing an entity upon changing editor mode (even outside of editor; happens when the current mode is removed)
		GetManager().GetOnModeChange().Insert(StopPosessing);
		GetManager().GetOnLimitedChange().Insert(OnLimitedChange);
	}
	override void EOnEditorDeleteServer()
	{
		if (GetManager())
		{
			GetManager().GetOnModeChange().Remove(StopPosessing);
			GetManager().GetOnLimitedChange().Remove(OnLimitedChange);
		}
		
		//--- Stop possessing an entity when editor is deleted
		StopPosessing();
	}
};