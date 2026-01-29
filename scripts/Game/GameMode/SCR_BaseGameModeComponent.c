[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SCR_BaseGameModeComponentClass : ScriptComponentClass
{
}

//! Interface for game mode extending components.
//! Must be attached to a GameMode entity.
class SCR_BaseGameModeComponent : ScriptComponent
{
	//! The game mode entity this component is attached to.
	protected SCR_BaseGameMode m_pGameMode;

	//------------------------------------------------------------------------------------------------
	//! \return Returns game mode this component is attached to.
	SCR_BaseGameMode GetGameMode()
	{
		return m_pGameMode;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on all machines when the world ends.
	void OnGameEnd();

	//------------------------------------------------------------------------------------------------
	//! Called when game mode state changes called on all machines.
	//! \param[in] state New state game mode transitioned into.
	void OnGameStateChanged(SCR_EGameModeState state);
	
	//------------------------------------------------------------------------------------------------
	//! Called on every machine when game mode starts.
	//! This can be immediate (if no pre-game period is set) or can happen after
	//! a certain delay, as deemed appropriate by the authority.
	void OnGameModeStart();

	//------------------------------------------------------------------------------------------------
	//! Called when game mode ends.
	//! \param[in] data End game data with game logic.
	void OnGameModeEnd(SCR_GameModeEndData data);

	//------------------------------------------------------------------------------------------------
	//! Called after a player is connected. Server-only.
	//! \param[in] playerId PlayerId of connected player.
	void OnPlayerConnected(int playerId);
	
	//------------------------------------------------------------------------------------------------
	//! Event is called when player connecting Session hosting current Game Mode where is required authentication verification via. platform services
	//! AuditSuccess() call specifically mean this verification was successful
	//! Basically audited player has access to persistency/ etc. related functionality provided by online services.
	//! \param[in] playerId is index of player in game, equal to the one assigned at PlayerController
	void OnPlayerAuditSuccess(int playerId);
	
	//------------------------------------------------------------------------------------------------
	//! Event is called when player connecting Session hosting current Game Mode
	//! AuditFail() call may be called under two occasions:
	//! 1) verification is required but failed (account is not valid, player is banned, internet issues)
	//! 2) player cannot be verified as authentication is not required or possible - where it may be valid behavior (server online connectivity turned off for example)
	//! Basically non-audited player cannot access persistency/ etc. related functionality provided by online services.
	//! \param[in] playerId is index of player in game, equal to the one assigned at PlayerController
	void OnPlayerAuditFail(int playerId);
	
	//------------------------------------------------------------------------------------------------
	//! Event is called when player connected to Session was kicked and did not reconnected in time
	//! This mean that slot reservation can be canceled.
	//! \param[in] playerId is index of player in game, equal to the one assigned at PlayerController
	void OnPlayerAuditTimeouted(int playerId);
	
	//------------------------------------------------------------------------------------------------
	//! Event is called when player reconnected successfully back to Session after kick
	//! This mean that slot reservation need to be finished (closed).
	//! \param[in] playerId is index of player in game, equal to the one assigned at PlayerController
	void OnPlayerAuditRevived(int playerId);

	//------------------------------------------------------------------------------------------------
	//! Called on every machine after a player is registered (identity, name etc.). Always called after OnPlayerConnected.
	//! \param[in] playerId PlayerId of registered player.
	void OnPlayerRegistered(int playerId);

	//------------------------------------------------------------------------------------------------
	//! Called after a player is disconnected.
	//! \param[in] playerId PlayerId of disconnected player.
	//! \param[in] cause Reason player disconnected
	//! \param[in] timeout Timeout for when players are allowed to connect again. -1 means Ban without an assigned timeout
	void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] requestComponent
	//! \param[in] handlerComponent
	//! \param[in] data
	//! \param[in] entity
	//! \return
	bool PreparePlayerEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] requestComponent
	//! \param[in] handlerComponent
	//! \param[in] entity
	//! \param[in] data
	//! \param[in] reason
	void OnSpawnPlayerEntityFailure_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnData data, SCR_ESpawnResult reason);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] requestComponent
	//! \param[in] handlerComponent
	//! \param[in] data
	//! \param[in] entity
	void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity);
	
	//------------------------------------------------------------------------------------------------
	//! See SCR_BaseGameMode.HandlePlayerKilled.
	bool HandlePlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator instigator)
	{
		return true; // by default, handle automatically
	}

	//------------------------------------------------------------------------------------------------
	//! Called after a player gets killed.
	//! \param[in] instigatorContextData Holds the data of the victim and killer
	void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData);
	
	//------------------------------------------------------------------------------------------------
	//! Called after player gets killed in cases where the kill was handled by the game mode,
	//! supressing the default OnPlayerKilled behaviour. See also SCR_BaseGameMode.HandlePlayerKilled().
	//! \param[in] playerId PlayerId of victim player.
	//! \param[in] playerEntity Entity of victim player if any.
	//! \param[in] killerEntity Entity of killer instigator if any.
	//! \param[in] instigator Instigator of the kill
	void OnPlayerKilledHandled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator instigator);

	//------------------------------------------------------------------------------------------------
	//! Called after a player gets deleted.
	//! \param[in] playerId Player ID
	//! \param[in] player Player entity
	void OnPlayerDeleted(int playerId, IEntity player);

	//------------------------------------------------------------------------------------------------
	//! Called when player role changes.
	//! \param[in] playerId Player whose role is being changed.
	//! \param[in] roleFlags Roles as a flags
	void OnPlayerRoleChange(int playerId, EPlayerRole roleFlags);

	//------------------------------------------------------------------------------------------------
	//! Called once loading of all entities of the world have been finished (still within the loading)
	//! \param[in] world Loaded world
	void OnWorldPostProcess(World world);

	//------------------------------------------------------------------------------------------------
	//! When a controllable entity is spawned, this event is raised.
	//! \param[in] entity Spawned entity that raised this event
	void OnControllableSpawned(IEntity entity);

	//------------------------------------------------------------------------------------------------
	//! When a controllable entity is destroyed, this event is raised.
	//! \param[in] instigatorContextData Holds the data of the victim and killer
	void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData);
	
	//------------------------------------------------------------------------------------------------
	//! Prior to a controllable entity being DELETED, this event is raised.
	//! Controllable entity is such that has BaseControllerComponent and can be
	//! possessed either by a player, an AI or stay unpossessed.
	//! \param[in] entity Entity about to be deleted
	void OnControllableDeleted(IEntity entity);

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_BaseGameModeComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_pGameMode = SCR_BaseGameMode.Cast(ent);
		if (!m_pGameMode)
		{
			string message = string.Format("%1 is attached to entity '%2' type=%3, required type=%4! This is not allowed!", Type().ToString(), ent.GetName(), ent.ClassName(), "SCR_BaseGameMode");
			Debug.Error(message);
			Print(message, LogLevel.WARNING);
		}
	}
}
