/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

sealed class WorkshopCatalogueApi
{
	private void WorkshopCatalogueApi();
	private void ~WorkshopCatalogueApi();

	/*!
	Returns current state of the Workshop
	\warning if state is DISABLED then it will not be enabled
	         it might be intentionally globally disabled due
	         to some configuration or due to some unrecoverable
	         error.
	*/
	static proto WorkshopInitState GetState();
	/*!
	Sets callback on which:
	    - OnSuccess will be invoked when workshop becomes or is READY.
	    - OnError   will be invoked when workshop becomes or is DISABLED.

	\note Callback will be invoked only once and then it will never be invoked
	      again because workshop is at that point state which will be kept
	      until application restart.
	*/
	static proto void OnReady(BackendCallback callback);
	//! Returns maximum supported size of catalogue for pages/offsets.
	static proto int GetMaxSize();
	/*!
	Will set size of catalogue which determines how many items fit into single page/offset.

	\note Configured size must correspond with how many items can be visible in UI at once for optimal functionality of WorkshopCatalogueApi.
	      This will clear cache so UI should be also cleared and request new data,
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
	\throws VME - if size was not yet set via SetSize()
	            - if no order by was set yet via AppendOrderBy()
	*/
	static proto void RequestPage(notnull BackendCallback pCallback, int page);
	/*!
	Will request load of data for offset from first item
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Offset 0 represent first item. Int is casted into uint32.
	\note This method should be used for scrolling UI - there is no fixed position where page begins and ends.
				It can be used even for paging UI implementation but it is recommended to use RequestPage() method for simplicity.
	\throws VME - if size was not yet set via SetSize()
	            - if no order by was set yet via AppendOrderBy()
	*/
	static proto void RequestOffset(notnull BackendCallback pCallback, int offset);
	/*!
	Will request forced refresh of currently loaded page/offset.
	Items will be cleared from cache and created again from received data
	which might be possibly different.
	\throws VME - if size was not yet set via SetSize()
	            - if no order by was set yet via AppendOrderBy()
	*/
	static proto void RequestRefresh(notnull BackendCallback pCallback);
	/*!
	Will set pointer to filters object for the catalogue.
	Filters should be set only once at the beginning and then you can just modify local instance of filters.
	If set to null then WorkshopCatalogueApi will provide all assets without any filtering.
	*/
	static proto void SetFilters(WorkshopCatalogueFilters filters);
	/*!
	Will return pointer to object of currently applied filters for the catalogue.
	\note Can return null if no filters are set in the catalogue.
	      In that case catalogue will provide all items without any filtering.
	*/
	static proto WorkshopCatalogueFilters GetFilters();
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	You cannot order by more than 3 categories.
	\throws VME - if field was already appended for sorting.
	            - if you try to exceed limit of categories to order by.
	            - if API is disabled for the project.
	*/
	static proto bool AppendOrderBy(EBackendCatalogueOrderDir orderDir, EWorkshopCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list
	static proto void ClearOrderBy();
	//! Getter for items of currently loaded page/offset.
	static proto void GetItems(out notnull array<WorkshopAssetBase> assetsOut);
	//! Returns count of how many pages of data exists with current configuration of catalogue.
	static proto int GetPageCount();
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
	//! Returns currently PREPARING/ACTIVE job or null.
	static proto WorkshopJob GetCurrentJob();
	/*!
	Returns array of queued jobs including the active (first item in array).
	Items are sorted by priority and can be changed by WorkshopJob::SetQueuePriority(int priority).
	*/
	static proto void GetQueuedJobs(out notnull array<WorkshopJob> jobsOut);
	//! Returns total count of queued jobs.
	static proto int GetQueuedJobsCount();
	//! Will pause all currently queued and active jobs
	static proto void PauseAllJobs();
	/*!
	Returns array of paused jobs. Jobs are not sorted in any specific way and priority cannot be changed.
	Previously unfinished jobs are restored after game launch and init of workshop.
	*/
	static proto void GetPausedJobs(out notnull array<WorkshopJob> jobsOut);
	//! Returns total count of paused jobs.
	static proto int GetPausedJobsCount();
	/*!
	Returns list of locally downloaded assets.
	\note List is currently not sorted in any specific way and might change during runtime
	\warning Do not use for listing as pages or scrolling of locally downloaded assets.
	         For this operation we will add filter for local assets so that they can be listed via
	         RequestPage() or RequestOffset().
	*/
	static proto void GetLocalAssets(out notnull array<WorkshopAssetBase> assetsOut);
	//! Returns count of how many assets does user have blocked.
	static proto int GetBlockedAssetsCount();
	/*!
	Returns count of how many assets does user have subscribed.
	\warning this only counts subscribed and not downloaded.
	*/
	static proto int GetSubscribedAssetsCount();
	//! Returns count of how many assets did user submit some rating.
	static proto int GetRatedAssetsCount();
}

/*!
\}
*/
