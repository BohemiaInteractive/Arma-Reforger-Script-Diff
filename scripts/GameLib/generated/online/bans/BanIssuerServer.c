/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! A ban issued by or via a game server.
sealed class BanIssuerServer: BanIssuerBase
{
	//! Returns the unique identifier of the server that issued the ban.
	proto external string GetId();
	//! Returns the name of the server that issued the ban.
	proto external string GetName();
}

/*!
\}
*/
