/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

sealed class ServerPlayerInfo
{
	private void ServerPlayerInfo();
	private void ~ServerPlayerInfo();

	//! Returns unique identity ID of the user.
	proto external UUID GetIdentityId();
	//! Returns name of the user.
	proto external string GetName();
	//! Returns platform kind of the user.
	proto external PlatformKind GetPlatformKind();
	//! Returns game-specific data for this player. May be null if not assigned or the game does not implement player data.
	proto external ServerPlayerGameDataBase GetGameData();
}

/*!
\}
*/
