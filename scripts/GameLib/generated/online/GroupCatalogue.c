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
* GroupCatalogue serves as manager (register new group) as well as browser.
	Handles logic of listing, paging and quick access to groups cached in memory.
*/
sealed class GroupCatalogue
{
	private void GroupCatalogue();
	private void ~GroupCatalogue();

	/*!
	Initializes the Group Catalogue by fetching all data needed from backend for proper functionality.
	This must be called before use and before calling InitMyGroups()!

	Fetch all game-specific Gameplay Approaches, Focuses, Game Regions and Roles.
	currently joined groups IDs and their data and the user's membership data of the joined groups from the backend.
	*/
	proto external void InitCatalogue(notnull BackendCallback pCallback);
	/*!
	Fetch currently joined groups IDs and their data and the user's membership data of the joined groups from the backend.
	This must be called AFTER InitCatalogue().
	*/
	proto external void InitMyGroups(notnull BackendCallback pCallback);
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
	proto external int GetJoinedGroups(out notnull array<GroupData> outJoinedGroups);
	/*!
	Create and register a new Group
	\param pManifest holds data needed to register a new Group
	*/
	proto external void RegisterGroup(notnull BackendCallback pCallback, notnull GroupManifest pManifest);
	proto external GroupData GetActiveGroup();
	//! Get item count on current page
	proto external int GetPageItemCount();
	//! Get current page number
	proto external int GetPage();
	//! Set number of items per page
	proto external void SetPageSize(int iCount);
	//! Get total item count on all pages
	proto external int GetTotalItemCount();
	//! Get page count
	proto external int GetPageCount();
	//! Returns max possible size of page that is allowed to be set
	proto external int MaxPageSize();
	//! Get page content
	proto external int GetPageItems(out notnull array<GroupData> outGroups);
}

/*!
\}
*/
