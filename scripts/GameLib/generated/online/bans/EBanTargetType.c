/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! What kind of entity a ban targets.
sealed enum EBanTargetType
{
	//! init value
	NONE,
	//! A user/player, always identified by a Game Identity (gid)
	USER,
	//! A group (GroupApi)
	GROUP,
	//! A server
	SERVER,
	//! backend could not map the target to a known kind
	UNKNOWN,
}

/*!
\}
*/
