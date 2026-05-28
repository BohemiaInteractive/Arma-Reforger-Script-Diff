/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_notifications
\{
*/

sealed class NotificationCatalogueApi
{
	private void NotificationCatalogueApi();
	private void ~NotificationCatalogueApi();

	//! Fetch profile settings from the backend. This includes profile ID and disabled categories.
	static proto void RequestProfileSettings(notnull BackendCallback pCallback);
	//! Mark all unread notifications as read
	static proto void MarkAllNotificationsAsRead(notnull BackendCallback pCallback);
	//! Mark selected notifications as read
	static proto void MarkNotificationsAsRead(notnull BackendCallback pCallback, notnull array<int> aMarkAsReadIds);
	//! Delete all notifications
	static proto void DeleteAllNotifications(notnull BackendCallback pCallback);
	//! Delete selected notifications
	static proto void DeleteNotifications(notnull BackendCallback pCallback, notnull array<int> aDeleteIds);
	//! Disable selected categories
	static proto void UpdateSettings(notnull BackendCallback pCallback, notnull array<string> aDisableCategories);
	//! Returns how many notifications were deleted
	static proto int GetLastDeletedCount();
	//! Returns how many notifications were marked as read
	static proto int GetLastMarkedAsReadCount();
	/*!
	Returns currently disabled categories.
	\note The disabled categories must be fetched via RequestProfileSettings() first.
	*/
	static proto int GetDisabledCategories(out notnull array<string> outCategories);
	/*!
	Will request load of data for offset from first item
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Offset 0 represent first item. Int is casted into uint32.
	\note This method should be used for scrolling UI - there is no fixed position where page begins and ends.
				It can be used even for paging UI implementation but it is recommended to use RequestPage() method for simplicity.
	*/
	static proto void RequestOffset(BackendCallback pCallback, int iOffset, bool bClearCache);
	/*!
	Will set pointer to filters object for the catalogue.
	Filters should be set only once at the beginning and then you can just modify local instance of filters.
	If set to null then no filters will be applied.
	*/
	static proto void SetFilters(NotificationCatalogueFilters pFilters);
	/*!
	Will set pointer to filters object for the catalogue.
	Filters should be set only once at the beginning and then you can just modify local instance of filters.
	If set to null then no filters will be applied.
	*/
	static proto NotificationCatalogueFilters GetFilters();
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	\throw VME if field was already appended for sorting.
	*/
	static proto bool AppendOrderBy(EBackendCatalogueOrderDir orderDir, ENotificationCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list
	static proto void ClearOrderBy();
	//! Returns count of how many pages of data exists with current configuration of catalogue.
	static proto int GetPageCount();
	/*!
	Will set size of catalogue which determines how many items fit into single page/offset.

	\note Configured size must correspond with how many items can be visible in UI at once for optimal functionality of NotificationCatalogueApi.
				This will clear cache so UI should be also cleared and request new data.
	*/
	static proto void SetSize(int size);
	//! Returns currently set size of catalogue for pages/offsets.
	static proto int GetSize();
	/*!
	Will request load of data for specific page of catalogue.
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Pages are indexed from 0. Int is casted into uint32.
	\note This method should be used for paging UI where single page with fixed size and offset is visible at the time.
				It should not be used for scrolling implementation which can possibly show items from 2 pages at once which can introduce issues.
	*/
	static proto void RequestPage(BackendCallback pCallback, int uPage, bool bClearCache);
	/*!
	Will request forced refresh of currently loaded page/offset.
	Items will be cleared from cache and created again from received data
	which might be possibly different.
	*/
	static proto void RequestRefresh(BackendCallback pCallback);
	/*!
	Returns index number of currently loaded page.Index 0 is first page.
	Use alongside RequestPage() and GetPageCount() to determine if you are at the end.
	*/
	static proto int GetPageNumber();
	//! Return total count of how many items exists with current configuration of catalogue.
	static proto int GetTotalItemCount();
	/*!
	Returns index number of currently loaded offset - index of first item in offset. Index 0 is the first item of catalogue.
	Use alongside RequestOffset() and GetTotalItemCount() to determine if you are at the end.
	*/
	static proto int GetOffsetIndex();
	/*!
	Returns count of items on currently loaded page / offset.
	\note If this value is less that currently configured size of the catalogue then you are at the end.
	*/
	static proto int GetCurrentItemCount();
	//! Get page content
	static proto int GetPageItems(out notnull array<NotificationInfo> outNotifications);
}

/*!
\}
*/
