/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

/*!
Named role with a set of permissions stored as a bitfield.

Roles are created and managed on the server through PlayerRoleManagerApi and
assigned to players identified by their identity UUID. Each role stores a unique
string ID, a display name, and a permission bitfield sized to match the total
number of EPlayerPermissions entries at initialization time.

A role that has the SUPER_USER permission implicitly grants all other permissions.
Use HasPermission to check whether a role grants a specific permission (including
the SUPER_USER override).

Roles are not directly constructible from script. Use PlayerRoleManagerApi.CreateRole
to create new roles.
*/
sealed class PlayerRole
{
	private void PlayerRole();
	private void ~PlayerRole();

	//! Returns the string ID of this role (e.g. "admin").
	proto external string GetId();
	//! Returns the display name of this role.
	proto external string GetName();
	/*!
	Returns true if the role has the given permission.
	If the role has the SUPER_USER permission, this always returns true regardless
	of the specific permission being queried.
	*/
	proto external bool HasPermission(EPlayerPermissions permissionIndex);
	/*!
	Sets the bit for the given permission.
	\throws VME if permissionIndex is out of range.
	*/
	proto external void AddPermission(EPlayerPermissions permissionIndex);
	/*!
	Clears the bit for the given permission.
	\throws VME if permissionIndex is out of range.
	*/
	proto external void RemovePermission(EPlayerPermissions permissionIndex);
	//! Clears all permission bits and resets the assigned permission count to zero.
	proto external void ClearAllPermissions();
	//! Returns count of explicitly assigned permissions on this role.
	proto external int GetAssignedPermissionCount();
	//! Populates permissionsOut with explicitly assigned permission indices. Permissions implied by SUPER_USER are not included.
	proto external void GetPermissions(out notnull array<EPlayerPermissions> permissionsOut);
}

/*!
\}
*/
