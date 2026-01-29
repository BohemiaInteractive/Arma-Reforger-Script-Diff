/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class GroupMembership: Managed
{
	private void GroupMembership();
	private void ~GroupMembership();

	//! Fetch details of this membership from backend
	proto external void RequestDetails(notnull BackendCallback pCallback);
	//! Kick this member from the Group. Marks this membership as no longer valid
	proto external void Kick(notnull BackendCallback pCallback);
}

/*!
\}
*/
