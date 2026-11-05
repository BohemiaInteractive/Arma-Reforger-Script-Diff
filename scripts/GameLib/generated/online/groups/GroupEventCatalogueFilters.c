/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

class GroupEventCatalogueFilters: Managed
{
	//! Only events whose start is at or after this instant. Empty/default Timestamp means unrestricted.
	proto external Timestamp GetStartsAfter();
	proto external void SetStartsAfter(Timestamp startsAfter);
	//! Only events whose end is at or before this instant. Empty/default Timestamp means unrestricted.
	proto external Timestamp GetEndsBefore();
	proto external void SetEndsBefore(Timestamp endsBefore);
}

/*!
\}
*/
