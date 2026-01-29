/*
===========================================
Do not modify, this script is generated
===========================================
*/

enum BattlEyeKickReason
{
	//! Inknown reason
	UNKNOWN,
	//! "Client not responding" - the client is not acknowledging packets sent from the server
	CLIENT_NOT_RESPONDING,
	//! "Query Timeout" - the client is not responding to scan queries sent from the master server
	QUERY_TIMEOUT,
	//! "Game restart required" - the client doesn't have the BE Service (protection) enabled
	GAME_RESTART_REQUIRED,
	//! "Bad Service Version" - the client is running an unsupported BE Service version
	BAD_SERVICE_VERSION,
	//! "Disallowed Program #X" - the client is running a blocked program, which doesn't necessarily indicate cheating (-> no ban), e.g. Cheat Engine
	DISALLOWED_PROGRAM,
	//! "Corrupted Memory #0" - the integrity of the protection failed to validate
	CORRUPTED_MEMORY,
	//! "Corrupted Data #X" - the integrity of the game was violated
	CORRUPTED_DATA,
	//! "WinAPI Failure #X" - a critical Windows API call failed
	WINAPI_FAILURE,
	//! "Global Ban #XXXXXX" - the player has been banned by BE
	GLOBAL_BAN,
	//! "Admin Ban #REASON" - the player has been banned by server administrator
	ADMIN_BAN,
	//! "Admin Kick (REASON)" - the player has been kicked by server administrator
	ADMIN_KICK,
	//! "Missing GameID/MasterPort server config settings"
	INVALID_SERVER_CONFIG,
}
