/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! A ban that targets a game server.
sealed class BanTargetServer: BanTargetBase
{
	//! Returns the server id.
	proto external string GetServerId();
	//! Returns name of this server at the time when ban was created.
	proto external string GetName();
}

/*!
\}
*/
