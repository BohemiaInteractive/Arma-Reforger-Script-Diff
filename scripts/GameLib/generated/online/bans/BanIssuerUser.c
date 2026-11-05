/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! A ban issued by a player.
sealed class BanIssuerUser: BanIssuerBase
{
	//! Returns Game Identity ID of this user.
	proto external UUID GetIdentityId();
	//! Returns the username of the issuer at the time the ban was created.
	proto external string GetName();
}

/*!
\}
*/
