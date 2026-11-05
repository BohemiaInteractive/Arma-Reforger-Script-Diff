/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! A ban that targets a player.
sealed class BanTargetUser: BanTargetBase
{
	//! Returns Game Identity ID of this user.
	proto external UUID GetIdentityId();
	//! Returns name of this user at the time when ban was created.
	proto external string GetName();
	//! Returns current name of this user.
	proto external string GetCurrentName();
}

/*!
\}
*/
