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
Paged catalogue of BanEntry records for a server's bans.

Script receives an instance via ServerBanManagerApi.GetServerBans() and calls
paging methods directly on it. C++ owns the instance lifetime through
ServerBanManager.
*/
sealed class ServerBanEntryCatalogue: Managed
{
	private void ServerBanEntryCatalogue();
	private void ~ServerBanEntryCatalogue();

	/*!
	Set the filter used to narrow down which bans are listed.
	Pass null to remove any filter and list all bans.
	\note Changing the filter clears any loaded results; request a page again afterwards.
	*/
	proto external void SetFilters(ServerBanEntryCatalogueFilter pFilter);
	//! Returns the currently active ban listing filter, or null if none is set.
	proto external ServerBanEntryCatalogueFilter GetFilters();
	//! Returns the maximum allowed number of ban entries per page.
	proto external int GetMaxSize();
	/*!
	Set how many ban entries to load per page.
	\note Changing the page size clears any loaded results; request a page again afterwards.
	*/
	proto external void SetSize(int size);
	//! Returns the currently set number of ban entries loaded per page.
	proto external int GetSize();
	/*!
	Load a specific page of ban entries. Pages are zero-indexed.
	Use GetPageCount() to know the total number of pages.
	*/
	proto external void RequestPage(notnull BackendCallback pCallback, int page);
	/*!
	Load ban entries starting from a specific item index.
	Offset 0 is the first ban entry.
	\note Suitable for scrolling UIs where two pages can be visible at once.
	*/
	proto external void RequestOffset(notnull BackendCallback pCallback, int offset);
	//! Reload the currently displayed ban entries, discarding any locally stored results.
	proto external void RequestRefresh(notnull BackendCallback pCallback);
	//! Fills the output array with the ban entries on the currently loaded page.
	proto external void GetItems(out array<BanEntry> itemsOut);
	//! Returns the total number of ban entries matching the current filter across all pages.
	proto external int GetTotalItemCount();
	//! Returns the total number of pages available with the current page size and filter.
	proto external int GetPageCount();
	/*!
	Returns the index of the currently loaded page. Index 0 is the first page.
	Use alongside RequestPage() and GetPageCount() to navigate pages.
	*/
	proto external int GetPageNumber();
	/*!
	Returns the number of ban entries on the currently loaded page.
	\note If this is less than the configured page size, you have reached the last page.
	*/
	proto external int GetCurrentItemCount();
}

/*!
\}
*/
