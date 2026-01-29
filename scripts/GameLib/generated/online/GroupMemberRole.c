/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class GroupMemberRole: Managed
{
	private void GroupMemberRole();
	private void ~GroupMemberRole();

	proto external string GetName();
	//! Fetch permissions of this role from backend
	proto external void RequestPermissions(notnull BackendCallback pCallback);
	proto external void HasPermission(ERolePermission ePermission);
	//! Returns all permissions defined for this role as bit mask
	proto external ERolePermission GetPermissions();
}

/*!
\}
*/
