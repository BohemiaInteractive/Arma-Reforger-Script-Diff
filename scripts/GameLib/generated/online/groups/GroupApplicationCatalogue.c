/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

sealed class GroupApplicationCatalogue
{
	private void GroupApplicationCatalogue();
	private void ~GroupApplicationCatalogue();

	/*!
	Delete applications / invitations of the given IDs.
	\note If the ID belongs to an application, the caller must be the owner of the application.
	      If the ID belongs to an invitation, the caller must have INVITATION_CREATE permission to be able to delete it
	*/
	proto external void DeleteApplications(notnull BackendCallback callback, notnull array<int> deleteIds);
	/*!
	Will request load of data for specific page of catalogue.
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Pages are indexed from 0. Int is casted into uint32.
	\note This method should be used for paging UI where single page with fixed size and offset is visible at the time.
	\throws VME - if size was not yet set via SetPageSize
							- if no order by was set yet via AppendOrderBy
	*/
	proto external void RequestPage(notnull BackendCallback pCallback, int iPage, bool bClearCache);
	/*!
	Will request forced refresh of currently loaded page/offset.
	Items will be cleared from cache and created again from received data
	which might be possibly different.
	\throws VME - if size was not yet set via SetPageSize
							- if no order by was set yet via AppendOrderBy
	*/
	proto external void RequestRefresh(notnull BackendCallback pCallback);
	/*!
	Will set pointer to filters object for the catalogue.
	Filters should be set only once at the beginning and then you can just modify local instance of filters.
	If set to null then no filters will be applied.
	*/
	proto external void SetFilters(GroupApplicationCatalogueFilters pFilters);
	/*!
	Will return pointer to object of currently applied filters for the catalogue.
	\note Can return null if no filters are set in the catalogue.
	*/
	proto external GroupApplicationCatalogueFilters GetFilters();
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	\throw VME if field was already appended for sorting.
	*/
	proto external void AppendOrderBy(EBackendCatalogueOrderDir orderDir, EGroupApplicationCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list
	proto external void ClearOrderBy();
	/*!
	Will request load of data for offset from first item
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Offset 0 represent first item. Int is casted into uint32.
	\note This method should be used for scrolling UI - there is no fixed position where page begins and ends.
	\throws VME - if size was not yet set via SetPageSize
							- if no order by was set yet via AppendOrderBy
	*/
	proto external void RequestOffset(notnull BackendCallback pCallback, int iOffset, bool bClearCache);
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
	proto external int GetPageItems(out notnull array<GroupApplication> outApplications);
}

/*!
\}
*/
