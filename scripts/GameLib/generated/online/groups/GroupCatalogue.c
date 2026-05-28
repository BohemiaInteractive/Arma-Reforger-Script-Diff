/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
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

	//! Returns application catalogue
	proto external GroupApplicationCatalogue GetApplicationCatalogue();
	/*!
	Initializes the Group Catalogue by fetching all data needed from backend for proper functionality.
	This must be called before use and before calling InitMyGroups()!

	Fetch all Group member Roles and their permissions.
	*/
	proto external void InitCatalogue(notnull BackendCallback pCallback);
	/*!
	Fetch currently joined groups and the user's membership data of the joined groups from the backend.
	This must be called AFTER InitCatalogue() to properly fill Group Member Roles and their permissions.
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
	Will request load of data for offset from first item
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Offset 0 represent first item. Int is casted into uint32.
	\note This method should be used for scrolling UI - there is no fixed position where page begins and ends.
				It can be used even for paging UI implementation but it is recommended to use RequestPage() method for simplicity.
	*/
	proto external void RequestOffset(BackendCallback pCallback, int uOffset, bool bClearCache);
	/*!
	Returns Groups the user is currently member of.
	\note Data must be fetched using InitMyGroups() before use.
	*/
	proto external int GetJoinedGroups(out notnull array<GroupData> outJoinedGroups);
	//! Get all defined group member roles.
	proto external int GetRoles(notnull out array<GroupMemberRole> outRoles);
	/*!
	Create and register a new Group
	\param pManifest holds data needed to register a new Group
	*/
	proto external void RegisterGroup(notnull BackendCallback pCallback, notnull GroupManifest pManifest);
	//! Group the user wants to play for
	proto external GroupData GetActiveGroup();
	/*!
	Will set pointer to filters object for the catalogue.
	Filters should be set only once at the beginning and then you can just modify local instance of filters.
	If set to null then no filters will be applied.
	*/
	proto external void SetFilters(GroupCatalogueFilters pFilters);
	/*!
	Will return pointer to object of currently applied filters for the catalogue.
	\note Can return null if no filters are set in the catalogue.
	*/
	proto external GroupCatalogueFilters GetFilters();
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	\throw VME if field was already appended for sorting.
	*/
	proto external void AppendOrderBy(EBackendCatalogueOrderDir orderDir, EGroupCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list
	proto external void ClearOrderBy();
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
