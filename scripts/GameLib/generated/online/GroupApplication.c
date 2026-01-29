/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Serves as both Application and Invitation
sealed class GroupApplication: Managed
{
	private void GroupApplication();
	private void ~GroupApplication();

	proto external bool IsInvitation();
	//! Approve the application. If there are free member spots at the time of approving, the user joins the Group.
	proto external void Approve(notnull BackendCallback pCallback);
	//! Reject the application. The user doesn't join the Group.
	proto external void Reject(notnull BackendCallback pCallback);
}

/*!
\}
*/
