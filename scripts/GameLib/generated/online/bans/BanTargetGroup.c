/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! A ban that targets a group.
sealed class BanTargetGroup: BanTargetBase
{
	//! Returns the group UUID.
	proto external UUID GetGroupId();
	//! Returns name of this group at the time when ban was created.
	proto external string GetName();
}

/*!
\}
*/
