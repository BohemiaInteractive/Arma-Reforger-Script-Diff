/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Network
\{
*/

class PlayerManager
{
	static ref ScriptInvoker<bool> s_OnPlayerNameCacheUpdateInvoker = new ScriptInvoker<bool>();

	/*!
	Kicks player from the game with option to specify timeout for reconnection (temporary ban).
	\param playerId Player to be kicked
	\param reason Reason for kick
	\param timeout Time in seconds, default 0, -1 means infinite
	*/
	proto external void KickPlayer(int iPlayerId, PlayerManagerKickReason reason, int timeout = 0);
	//! Returns the number of connected players
	proto external int GetPlayerCount();
	//! Returns the number of all players (both connected and disconnected)
	proto external int GetAllPlayerCount();
	//! Returns the number of disconnected players
	proto external int GetDisconnectedPlayerCount();
	//! Returns the list of connected players
	proto external int GetPlayers(out notnull array<int> outPlayers);
	//! Returns the list of all players (both connected and disconnected)
	proto external int GetAllPlayers(out notnull array<int> outPlayers);
	//! Returns the list of disconnected players
	proto external int GetDisconnectedPlayers(out notnull array<int> outPlayers);
	//! Returns the name of a given player. Empty is player can't be found
	proto external string GetPlayerName(int iPlayerId);
	/*!
	Requests player name cache update.
	Invokes OnPlayerNameCacheUpdate when finished.
	*/
	static proto bool RequestPlayerNameCacheUpdate(array<string> aIdentityIDs);
	//! Get name of the player with the given identity. Empty if not connected and not in player name cache.
	proto external owned string GetPlayerNameByIdentity(string playerIdentity);
	//! Returns the entity controlled by a given player
	proto external IEntity GetPlayerControlledEntity(int iPlayerId);
	/*!
	Returns respawn component attached to player controller of given player.
	Note: Only locally owned respawn component is available on remote clients.
	\return RespawnComponent instance or nullptr if none.
	*/
	proto external RespawnComponent GetPlayerRespawnComponent(int iPlayerId);
	//! Returns the playerId of a player based on a given entity. 0 if not found.
	proto external int GetPlayerIdFromControlledEntity(IEntity controlled);
	proto external int GetPlayerIdFromEntityRplId(int entityRplId);
	//! Returns the PlayerController of a given player.
	proto external PlayerController GetPlayerController(int iPlayerId);
	//! Returns the platform kind of a given player.
	proto external PlatformKind GetPlatformKind(int iPlayerId);
	//! Returns true if a given player is connected.
	proto external bool IsPlayerConnected(int iPlayerId);
	//! Returns True if the provided player have given role
	proto external bool HasPlayerRole(int iPlayerId, EPlayerRole role);
	//! Returns player's roles as flags
	proto external EPlayerRole GetPlayerRoles(int iPlayerId);
	/*!
	Sets role of specified player. Can be called only by authority.
	Returns true if completed successfully.
	*/
	proto external bool GivePlayerRole(int iPlayerId, EPlayerRole role);
	/*!
	Clears role of specified player. Can be called only by authority.
	Returns true if completed successfully.
	*/
	proto external bool ClearPlayerRole(int iPlayerId, EPlayerRole role);
	/*!
	Shows platform specific UI with the profile of specified player.
	Will do nothing if the specified user is not using same platform.
	\param[in] playerId Player which profile will be shown.
	*/
	proto external void ShowUserProfile(int iPlayerId);
	/*!
	\return true if profile of specified player could be shown.
	*/
	proto external bool IsUserProfileAvailable(int iPlayerId);
	/*!
	Invokes platform UI for selection of players to invite into game/party.
	Fails if no multiplayer activiy is set.
	Xbox: Puts console into constrained mode
	*/
	proto external bool ShowMultiplayerActivityInvite();
	/*!
	\return true if other players could be invited to the session using
	ShowMultiplayerActivityInvite
	*/
	proto external bool IsMultiplayerActivityInviteAvailable();

	// callbacks

	//! Event invoked as a result of RequestPlayerNameCacheUpdate
	static event protected void OnPlayerNameCacheUpdate(bool success) { s_OnPlayerNameCacheUpdateInvoker.Invoke(success); };
}

/*!
\}
*/
