/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_UserSessionData
\{
*/

//! Session record of a user - current online presence.
sealed class UserSessionData: Managed
{
	private void UserSessionData();
	private void ~UserSessionData();

	//! User ID this session belongs to.
	proto external UUID GetIdentityId();
	//! User's online status
	proto external EUserOnlineStatus GetStatus();
	//! Platform of the user's current client
	proto external PlatformKind GetPlatformType();
	//! User's display username
	proto external string GetUsername();
}

/*!
\}
*/
