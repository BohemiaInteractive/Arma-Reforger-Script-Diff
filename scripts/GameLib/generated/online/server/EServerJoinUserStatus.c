/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

//! Describes the readiness of the user for server joining. Used internally to communicate user state to the backend.
sealed enum EServerJoinUserStatus
{
	//! User is ready to join the server.
	READY,
	//! User is busy without specifying reason and is not ready to join.
	BUSY,
}

/*!
\}
*/
