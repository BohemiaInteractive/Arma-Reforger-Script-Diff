/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class BackendCommentCatalogue
{
	private void BackendCommentCatalogue();
	private void ~BackendCommentCatalogue();

	/*!
	Post top-level comment in the selected thread.
	\note This will fail when called on BackendComment.GetThreadCatalogue()
	      To post replies use BackendComment.Reply() instead
	*/
	proto external void CreateComment(notnull BackendCallback callback, string text);
	//! Returns max possible size of page that is allowed to be set
	proto external int GetMaxSize();
	/*!
	Will set size of catalogue which determines how many items fit into single page/offset.

	\note Configured size must correspond with how many items can be visible in UI at once for optimal functionality of the catalogue.
				This will clear cache so UI should be also cleared and request new data,
	*/
	proto external void SetSize(int size);
	//! Returns currently set size of catalogue for pages/offsets.
	proto external int GetSize();
	/*!
	Will request load of data for specific page of catalogue.
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Pages are indexed from 0. Int is casted into uint32.
	\note This method should be used for paging UI where single page with fixed size and offset is visible at the time.
				It should not be used for scrolling implementation which can possibly show items from 2 pages at once which can introduce issues.
	\throws VME - if size was not yet set via SetSize()
							- if no order by was set yet via AppendOrderBy()
	*/
	proto external void RequestPage(notnull BackendCallback pCallback, int page);
	/*!
	Will request load of data for offset from first item
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Offset 0 represent first item. Int is casted into uint32.
	\note This method should be used for scrolling UI - there is no fixed position where page begins and ends.
				It can be used even for paging UI implementation but it is recommended to use RequestPage() method for simplicity.
	\throws VME - if size was not yet set via SetSize()
							- if no order by was set yet via AppendOrderBy()
	*/
	proto external void RequestOffset(notnull BackendCallback pCallback, int offset);
	/*!
	Will request forced refresh of currently loaded page/offset.
	Items will be cleared from cache and created again from received data
	which might be possibly different.
	\throws VME - if size was not yet set via SetSize()
							- if no order by was set yet via AppendOrderBy()
	*/
	proto external void RequestRefresh(notnull BackendCallback pCallback);
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	You cannot order by more than 3 categories.
	\throws VME - if field was already appended for sorting.
							- if you try to exceed limit of categories to order by.
	*/
	proto external bool AppendOrderBy(EBackendCatalogueOrderDir orderDir, EBackendCommentCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list
	proto external void ClearOrderBy();
	//! Will clear ordering of replies
	proto external void ClearRepliesOrderBy();
	//! Getter for items of currently loaded page/offset.
	proto external void GetItems(out notnull array<BackendComment> commentsOut);
	//! Returns count of how many pages of data exists with current configuration of catalogue.
	proto external int GetPageCount();
	/*!
	Returns index number of currently loaded page.Index 0 is first page.
	Use alongside RequestPage() and GetPageCount() to determine if you are at the end.
	*/
	proto external int GetPageNumber();
	//! Return total count of how many items exists with current configuration of catalogue.
	proto external int GetTotalItemCount();
	/*!
	Returns index number of currently loaded offset - index of first item in offset. Index 0 is the first item of catalogue.
	Use alongside RequestOffset() and GetTotalItemCount() to determine if you are at the end.
	*/
	proto external int GetOffsetIndex();
	/*!
	Returns count of items on currently loaded page / offset.
	\note If this value is less that currently configured size of the catalogue then you are at the end.
	*/
	proto external int GetCurrentItemCount();
}

/*!
\}
*/
