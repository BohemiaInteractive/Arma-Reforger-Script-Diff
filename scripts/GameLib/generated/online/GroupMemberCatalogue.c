/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class GroupMemberCatalogue
{
	private void GroupMemberCatalogue();
	private void ~GroupMemberCatalogue();

	/*!
	Will request load of data for specific page of catalogue.
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Pages are indexed from 0. Int is casted into uint32.
	\note This method should be used for paging UI where single page with fixed size and offset is visible at the time.
				It should not be used for scrolling implementation which can possibly show items from 2 pages at once which can introduce issues.
	*/
	proto external void RequestPage(BackendCallback pCallback, int uPage, bool bClearCache);
	//! Will forcefully request refresh for current page/offset. Always clears already present items.
	proto external void RequestRefresh(BackendCallback pCallback);
	/*!
	Will request load of data for specific page of catalogue.
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Pages are indexed from 0. Int is casted into uint32.
	\note This method should be used for paging UI where single page with fixed size and offset is visible at the time.
				It should not be used for scrolling implementation which can possibly show items from 2 pages at once which can introduce issues.
	*/
	proto external void RequestOffset(BackendCallback pCallback, int uOffset, bool bClearCache);
	//! Get item count on current page
	proto external int GetPageItemCount();
	//! Get current page number
	proto external int GetPage();
	//! \brief Set number of items per page
	proto external void SetPageSize(int iCount);
	//!  Get total item count on all pages
	proto external int GetTotalItemCount();
	//! Get page count
	proto external int GetPageCount();
	//! Returns max possible size of page that is allowed to be set
	proto external int MaxPageSize();
	//! Get page content
	proto external int GetPageItems(out notnull array<GroupMembership> outMembers);
}

/*!
\}
*/
