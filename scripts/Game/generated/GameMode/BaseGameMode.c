/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameMode
\{
*/

class BaseGameModeClass: GenericEntityClass
{
}

class BaseGameMode: GenericEntity
{
	//! Gets called after world is initialized but before first ticks
	event protected void OnGameStart();
	/*
	Event is called when player connecting Session hosting current Game Mode where is required authentication verification via. platform services
	AuditSuccess() call specifically mean this verification was successful
	Basically audited player has access to persistency/ etc. related functionality provided by online services.
	\param iPlayerID is index of player in game, equal to the one assigned at PlayerController
	*/
	event protected void OnPlayerAuditSuccess( int iPlayerID ) {};
	/*
	Event is called when player connecting Session hosting current Game Mode
	AuditFail() call may be called under two occassion:
	1) verification is required but failed (account is not valid, player is banned, internet issues)
	2) player cannot be verified as authentication is not required or possible - where it may be valid behavior (server online connectivity turned off for example)
	Basically non-audited player cannot access persistency/ etc. related functionality provided by online services.
	\param iPlayerID is index of player in game, equal to the one assigned at PlayerController
	*/
	event protected void OnPlayerAuditFail( int iPlayerID) {};
	/*
	Event is called when player connected to Session was kicked and did not reconnected in time
	This mean that slot reservation can be canceled.
	\param iPlayerID is index of player in game, equal to the one assigned at PlayerController
	*/
	event protected void OnPlayerAuditTimeouted( int iPlayerID) {};
	/*
	Event is called when player reconnected successfully back to Session after kick
	This mean that slot reservation need to be finished (closed).
	\param iPlayerID is index of player in game, equal to the one assigned at PlayerController
	*/
	event protected void OnPlayerAuditRevived( int iPlayerID) {};
	/*!
	Called after a player is connected. Server-only.
	\param playerId PlayerId of connected player.
	*/
	event protected void OnPlayerConnected(int playerId);
	/*!
	Called on every machine after a player is registered (identity, name etc.). Always called after OnPlayerConnected.
	\param playerId PlayerId of registered player.
	*/
	event protected void OnPlayerRegistered(int playerId);
	/*!
	Called after a player is disconnected.
	\param playerId PlayerId of disconnected player.
	\param cause Cause code
	\param timeout Kick timeout in seconds (-1 is infinity)
	*/
	event protected void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout);
	/*!
	Called after a player is spawned.
	\param playerId PlayerId of spawned player.
	\param controlledEntity Spawned entity for this player.
	*/
	event protected void OnPlayerSpawned(int playerId, IEntity controlledEntity);
	/*!
	Called after a player is killed, but before 'OnPlayerKilled'.

	The default behavior is such that this method returns true, therefore each kill is handled automatically, resulting
	in a 'OnPlayerKilled' callback. When this method is overridden to return false, the callback is not called automatically.

	This custom behavior can be used to suppress 'OnPlayerKilled' callbacks in specific scenarios, e.g. when a player is
	possessing an AI, in which case it might not be desirable to raise this callback, but rather handle the kill manually,
	by the user: e.g. by returning the player's control of their previous entity.

	Return true to handle kill automatically, resulting in an immediate 'OnPlayerKilled' callback,
	otherwise the kill callback is suppressed, allowing custom user handling.

	\param playerId PlayerId of victim player.
	\param player Entity of victim player (if any).
	\param killerEntity entity of the instigator. If killerEntity is null, you can use instigator.GetInstigatorEntity() if appropiate.
	\param killer Entity of killer instigator (if any).

	\return True to handle kill automatically (raise 'OnPlayerKilled'), false to not handle automatically (don't raise).
	*/
	event protected bool HandlePlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer) { return true; };
	/*!
	Called after a player gets killed (but only after 'SCR_HandlePlayerKilled' returns true).
	\param playerId PlayerId of victim player.
	\param player Entity of victim player if any.
	\param killerEntity entity of the instigator. If killerEntity is null, you can use instigator.GetInstigatorEntity() if appropiate.
	\param killer Entity of killer instigator if any.
	*/
	event protected void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer);
	/*!
	When a controllable entity is spawned, this event is raised.
	Controllable entity is such that has BaseControllerComponent and can be
	possessed either by a player, an AI or stay unpossessed.
	\param entity Spawned entity that raised this event
	*/
	event protected void OnControllableSpawned(IEntity entity) {};
	/*
	When a controllable entity is destroyed (damaged enough), this event is raised.
	Controllable entity is such that has BaseControllerComponent and can be
	possessed either by a player, an AI or stay unpossessed.
	\param entity Destroyed entity that raised this event
	\param killerEntity entity of the instigator. If killerEntity is null, you can use instigator.GetInstigatorEntity() if appropiate.
	\param instigator Instigator that destroyed our victim
	*/
	event protected void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator instigator) {};
	/*
	Prior to a controllable entity being DELETED, this event is raised.
	Controllable entity is such that has BaseControllerComponent and can be
	possessed either by a player, an AI or stay unpossessed.
	\param entity Entity about to be deleted
	*/
	event protected void OnControllableDeleted(IEntity entity) {};
	/*!
	Called when player role changes.
	\param playerId Player whose role is being changed.
	\param roleFlags Roles as a flags
	*/
	event protected void OnPlayerRoleChange(int playerId, EPlayerRole roleFlags) {};
	//! Event called once loading of all entities of the world have been finished. (still within the loading)
	event void OnWorldPostProcess(World world) {};
}

/*!
\}
*/
