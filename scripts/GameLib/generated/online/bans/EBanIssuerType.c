/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! Who issued the ban.
enum EBanIssuerType
{
	//! init value
	NONE,
	//! game could not determine who was the issuer
	UNKNOWN,
	//! Issued by a player via the client API
	USER,
	//! Issued by/via a server (RCON, automatic server-side system)
	SERVER,
	//! Issued by the official publisher (moderation tools)
	PUBLISHER,
}

/*!
\}
*/
