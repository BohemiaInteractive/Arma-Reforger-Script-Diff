/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

/*!
GroupMemberRole primarily defines permissions of a group member. All roles are currently fetched
from the backend as they are game-specific and are owned by GroupCatalogue.
Use GroupCatalogue::GetRoles() to get all defined roles.
*/
sealed class GroupMemberRole: Managed
{
	private void GroupMemberRole();
	private void ~GroupMemberRole();

	proto external string GetName();
	//! Fetch permissions of this role from backend
	proto external void RequestPermissions(notnull BackendCallback pCallback);
	proto external bool HasPermission(ERolePermission ePermission);
	//! Returns all permissions defined for this role as bit mask
	proto external ERolePermission GetPermissions();
	//! Returns priority of this role
	proto external int GetPriority();
}

/*!
\}
*/
