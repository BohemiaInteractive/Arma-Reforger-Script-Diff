/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

//! Serves as both Application and Invitation.
sealed class GroupApplication: Managed
{
	private void GroupApplication();
	private void ~GroupApplication();

	//! Returns ID of the Group this application belongs to
	proto external int GetGroupId();
	// Returns Group this application belongs to
	proto external GroupData GetGroup();
	//! Returns ID of the application
	proto external int GetId();
	//! Returns the remaining number of seconds until expiration
	proto external int GetExpireIn();
	//! Returns game identity of the user the application / invitation belongs to
	proto external UUID GetGameIdentity();
	//! Returns whether this application was created by inviting a player or a player applying for membership
	proto external bool IsInvitation();
	//! Approve the application. If the user hasn't reached maximum number of joined groups, they join the group.
	proto external void Approve(notnull BackendCallback pCallback);
	//! Reject the application. The user doesn't join the group.
	proto external void Reject(notnull BackendCallback pCallback);
}

/*!
\}
*/
