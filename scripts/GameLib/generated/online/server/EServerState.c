/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

//! Determines different states of registered server lifetime.
sealed enum EServerState
{
	//! Server registration to backend is disabled (current session is either singleplayer or unregistered multiplayer).
	DISABLED,
	//! Server is supposed to be registered but is currently in offline state and trying to authenticate.
	OFFLINE,
	//! Server is authenticated but not yet registered.
	AUTHENTICATED,
	//! Server is authenticated, registered and is now performing runtime heartbeats.
	RUNTIME,
}

/*!
\}
*/
