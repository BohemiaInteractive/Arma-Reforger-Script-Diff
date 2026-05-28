/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

class GroupApplicationCatalogueFilters: Managed
{
	//! Sets IsInvitation filter
	proto external void SetIsInvitation(EBackendCatalogueBoolFlag eIsInvitation);
	//! Returns current IsInvitation filter
	proto external EBackendCatalogueBoolFlag IsInvitation();
	//! Sets Group ID filter
	proto external void SetGroupId(int iGroupId);
	//! Returns current Group ID filter
	proto external int GetGroupId();
	//! Sets Game Identity filter
	proto external void SetGameIdentity(UUID GameIdentity);
	//! Returns current Game Identity filter
	proto external UUID GetGameIdentity();
}

/*!
\}
*/
