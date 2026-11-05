/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

/*!
Filter for the /servers/bans/list request (ServerBanFilter on the wire).

All fields are optional; an omitted field is not filtered on. Script creates
instances via ServerBanEntryCatalogueFilter.CreateInstance().
*/
sealed class ServerBanEntryCatalogueFilter: Managed
{
	//! Full-text search — only bans whose target name contains this text are returned.
	proto external void SetSearchText(string sSearchText);
	//! Ban lifetime filter. Default is ACTIVE.
	proto external void SetStatus(EBanStatus eStatus);
	//! Restrict to bans with the given scope action (SERVER system is implied).
	proto external void SetScopeAction(EBanScopeAction eAction);
	proto external void SetBanLists(notnull array<BanList> banLists);
	proto external void SetIdentityIds(notnull array<UUID> identityIds);
}

/*!
\}
*/
