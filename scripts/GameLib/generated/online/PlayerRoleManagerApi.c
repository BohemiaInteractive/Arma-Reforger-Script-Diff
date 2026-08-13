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
Script API for the player role system.

Provides methods to query, create, remove and assign roles to players identified
by their identity UUID. All methods are static and operate on the singleton
PlayerRoleManager owned by BackendApi.

Roles can be either persistent (saved to storage across restarts) or temporary
(in-memory only, lost on restart). Both types are checked when querying permissions.

\note If the manager is not initialized (backend not running or the feature is
disabled), all methods return default values (nullptr, false, 0) without side effects.
*/
sealed class PlayerRoleManagerApi
{
	private void PlayerRoleManagerApi();
	private void ~PlayerRoleManagerApi();

	//! Returns the total number of defined roles, or -1 if the manager is not initialized.
	static proto int GetRoleCount();
	//! Populates rolesOut with all currently defined roles.
	static proto void GetRoles(out notnull array<PlayerRole> rolesOut);
	//! Returns the role with the given string ID, or null if not found.
	static proto PlayerRole FindRole(string roleId);
	/*!
	Creates a new role with the given ID and display name.
	\throws VME if a role with the same ID already exists.
	\return The newly created role, or null if role already exists or if the manager is not initialized.
	*/
	static proto PlayerRole CreateRole(string roleId, string roleName);
	/*!
	Removes a role from the manager and from all players that have it assigned.
	\return True if the role was found and removed.
	*/
	static proto bool RemoveRole(notnull PlayerRole role);
	//! Returns true if the player has the given role (checks both persistent and temporary).
	static proto bool PlayerHasRole(UUID identityId, PlayerRole role);
	//! Returns true if the player has the given permission through any assigned role.
	static proto bool PlayerHasPermission(UUID identityId, EPlayerPermissions permissionIndex);
	/*!
	Assigns a persistent role to a player. Persistent roles are saved to storage.
	Throws VME if the player already has this role assigned.
	\return True if the role was assigned.
	*/
	static proto bool AssignPersistentRoleToPlayer(UUID identityId, notnull PlayerRole role);
	/*!
	Removes a persistent role from a player.
	\return True if the role was found and removed.
	*/
	static proto bool RemovePersistentRoleFromPlayer(UUID identityId, notnull PlayerRole role);
	//! Clears all persistent roles from a player.
	static proto void ClearPersistentPlayerRoles(UUID identityId);
	/*!
	Assigns a temporary role to a player. Temporary roles are not persisted across restarts.
	Throws VME if the player already has this role assigned.
	\return True if the role was assigned.
	*/
	static proto bool AssignTempRoleToPlayer(UUID identityId, notnull PlayerRole role);
	/*!
	Removes a temporary role from a player.
	\return True if the role was found and removed.
	*/
	static proto bool RemoveTempRoleFromPlayer(UUID identityId, notnull PlayerRole role);
	//! Clears all temporary roles from a player.
	static proto void ClearTempPlayerRoles(UUID identityId);
	//! Clears all roles from a player (both persistent and temporary).
	static proto void ClearAllPlayerRoles(UUID identityId);
}

/*!
\}
*/
