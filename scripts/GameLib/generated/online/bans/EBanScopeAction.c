/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! Action a ban applies to, across all systems.
enum EBanScopeAction
{
	//! Not set.
	NONE,
	//! Could not be resolved to a known action.
	UNKNOWN,
	//! All actions.
	GLOBAL,
	//! Banned from joining.
	JOIN,
	//! Banned from hosting.
	HOST,
	//! Banned from publishing.
	PUBLISH,
	//! Banned from creating.
	CREATE,
}

/*!
\}
*/
