/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

sealed class GroupData: Managed
{
	private void GroupData();
	private void ~GroupData();

	//! Returns user's membership data if the user is a member of this group. Returns nullptr otherwise.
	proto external GroupMembership GetMyMembership();
	proto external string GetName();
	proto external string GetTag();
	proto external string GetDescription();
	proto external string GetContactInfo();
	proto external string GetPrimaryLanguage();
	proto external string GetRules();
	//! Request details of this Group from backend.
	proto external void RequestDetails(notnull BackendCallback pCallback);
	//! Update Group data
	proto external void UpdateGroup(notnull BackendCallback pCallback, notnull GroupManifest pManifest);
	/*!
	if the Group's enlistment status is set to:
	  1. IMMEDIATE - User joins the group
	  2. APPROVAL - Creates a membership application to be evaluated
	  3. FREEZE - The request fails
	*/
	proto external void ApplyForMembership(notnull BackendCallback pCallback);
	/*!
	Send invitation to join the Group to the user with given Game Identity.
	The user sending the invitation must have specific permission.
	\note If the invited user already applied for membership,
	      sending an invitation will automatically make the user join the group.
	*/
	proto external void CreateInvitation(notnull BackendCallback pCallback, UUID InvitedGameIdentity);
	/*!
	Leave the group.
	\note Owner cannot leave the group. Either pass the ownership to someone else and then leave or disband the group.
	*/
	proto external void Leave(notnull BackendCallback pCallback);
	//! Returns image of given type
	proto external BImage GetImage(EGroupImageType eType);
	/*!
	Called by the member with the highest role to transfer the role to another member.
	\note Must have ROLE_MANAGEMENT permission.
	\param pCallback Script callback where you will receive the result
	\param pMember Member to transfer the role to
	*/
	proto external void TransferTopRole(notnull BackendCallback pCallback, notnull GroupMembership pMember);
	/*!
	Disband the group. This will make every member to lose their membership and will delete this group from backend.
	*/
	proto external void Disband(notnull BackendCallback pCallback);
	/*!
	Set active group - the group the user wants to play for.
	The user must be a member of the requested group.
	/param pCallback Script callback where you will receive the result
	*/
	proto external void SetActive(notnull BackendCallback pCallback);
	proto external int GetId();
	proto external GroupMemberCatalogue GetGroupMemberCatalogue();
	//! Returns current number of members. Must be fetched via RequestDetails()
	proto external int GetMemberCount();
	/*!
	Get all Gameplay Focuses defined for this Group as bitmask
	\note Enums must be defined in the gamecode as bitmask flags (1, 2, 4, 8, 18...)
			 The values must be defined in the same order as on the backend side.
	*/
	proto external EGroupGameplayApproach GetGameplayApproaches();
	/*!
	Get all Gameplay Focuses defined for this Group as bitmask
	\note Enums must be defined in the gamecode as bitmask flags (1, 2, 4, 8, 16...)
	      The values must be defined in the same order as on the backend side.
	*/
	proto external EGroupGameplayFocus GetGameplayFocuses();
	/*!
	Get all Gameplay Focuses defined for this Group as bitmask
	\note Enums must be defined in the gamecode as bitmask flags (1, 2, 4, 8, 16...)
				The values must be defined in the same order as on the backend side.
	*/
	proto external EGroupGameRegion GetGameRegions();
	//! Returns current enlistment status of the Group
	proto external EGroupEnlistmentStatus GetEnlistmentStatus();
	proto external void Report(notnull BackendCallback pCallback, string sDescription, EGroupReportReason eReason);
}

/*!
\}
*/
