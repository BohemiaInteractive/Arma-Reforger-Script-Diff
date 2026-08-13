/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

class GroupAuditLogCatalogueFilters: Managed
{
	//! Return currently set maxDays filter
	proto external int GetMaxDays();
	//! Limit results to entries created within the last 'maxDays' days. Defaults to 90.
	proto external void SetMaxDays(int maxDays);
}

/*!
\}
*/
