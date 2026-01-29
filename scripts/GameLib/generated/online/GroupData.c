/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
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
	//! Request details of this Group from backend.
	proto external void RequestDetails(notnull BackendCallback pCallback);
	//! Update Group data
	proto external void UpdateGroup(notnull BackendCallback pCallback, notnull GroupManifest pManifest);
	/*!
	if enlistment status is set to:
	  1. IMMEDIATE - User joins the group
	  2. APPROVAL - Creates a membership application to be evaluated
	  3. FREEZE - The request fails
	*/
	proto external void ApplyForMembership(notnull BackendCallback pCallback);
	/*!
	Leave the group.

	\note Owner cannot leave the group. Either pass the ownership to someone else and then leave or disband the group.
	*/
	proto external void Leave(notnull BackendCallback pCallback);
	/*!
	Disband the group. This will make every member to lose their membership and will delete this group from backend.
	*/
	proto external void Disband(BackendCallback pCallback);
	//! Fetch pending applications and invitations from backend
	proto external void RequestApplications(notnull BackendCallback pCallback);
	//! Return filtered array of either applications or invitations
	proto external int GetApplications(out notnull array<GroupApplication> outApplications, bool bInvitations);
	/*!
	Set active group - the group the user wants to play for.
	The user must be a member of the requested group.
	/param pCallback Script callback where you will receive the result
	*/
	proto external void SetActive(notnull BackendCallback pCallback);
	//! Returns current number of members. Must be fetched via RequestDetails()
	proto external int GetMemberCount();
	/*!
	Get names of all Gameplay Focuses defined for this Group
	Gameplay Focuses must be first fetched from backend by GroupCatalogue::RequestGameplayFocuses()
	*/
	proto external int GetGameplayFocuses(out notnull array<string> outFocuses);
	/*!
	Get names of all Gameplay Approaches defined for this Group
	Gameplay Approaches must be first fetched from backend by GroupCatalogue::RequestGameplayApproaches()
	*/
	proto external int GetGameplayApproaches(out notnull array<string> outApproaches);
	/*!
	Get names of all Game Regions defined for this Group
	Game Regions must be first fetched from backend by GroupCatalogue::RequestGameRegions()
	*/
	proto external int GetGameRegions(out notnull array<string> outRegions);
	proto external string GetEnlistmentStatus();
}

/*!
\}
*/
