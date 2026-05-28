/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

sealed class GroupMembership: Managed
{
	private void GroupMembership();
	private void ~GroupMembership();

	//! Returns group of the membership
	proto external GroupData GetGroup();
	//! Returns membership ID
	proto external int GetId();
	//! Returns nickname of the member
	proto external string GetNickname();
	//! Returns time when the membership was created
	proto external int CreatedAt();
	//! Returns Role of this member
	proto external GroupMemberRole GetRole();
	//! Fetch details of this membership from backend
	[Obsolete("All details needed are already fetched by GroupMemberCatalogue.")]
	proto external void RequestDetails(notnull BackendCallback pCallback);
	//! Kick the member from the Group. Marks the membership as no longer valid
	proto external void Kick(notnull BackendCallback pCallback);
	/*!
	Update member's role to a new one.
	\note The user updating cannot promote other user to a
	      role with higher priority than their own role and must have sufficient permissions to do so.
	\param pCallback Script callback where you will receive the result
	\param iRoleId ID of the new role
	*/
	proto external void UpdateRole(notnull BackendCallback pCallback, int iRoleId);
}

/*!
\}
*/
