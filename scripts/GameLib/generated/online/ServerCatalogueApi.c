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
Catalogue for listing servers from backend.
User needs to be authenticated (see BackendAuthenticatorApi for more) to be able to send requests.

You cab switch to different modes which will determine source
from which it will search for servers - Internet / LAN
This only influences list of available servers but details will be still fetched from the backend.

Catalogue has functionality to cache and pre-load data for faster availability but only for consecutive data.
This works for limited amount of previously loaded or single next page in direction you are requesting pages.
When requesting page 1 we will automatically request page 2 and afterwards when requesting page 2 it will load
immediately. Page 1 will stay cached in case user will want to switch back.

You can request data either by specific pages or by offset. Both basically works same but pages have fixed offsets
by currently set size to prevent need to manually calculate where every individual page begins on the offset.

\note You must first initialize size of the catalogue before you can request any data.
      Refer to SetSize() method for more details.
*/
sealed class ServerCatalogueApi
{
	private void ServerCatalogueApi();
	private void ~ServerCatalogueApi();

	//! Returns maximum supported size of catalogue for pages/offsets.
	static proto int GetMaxSize();
	/*!
	Will set size of catalogue which determines how many items fit into single page/offset.

	\note Configured size must correspond with how many items can be visible in UI at once for optimal functionality of ServerCatalogueApi.
	      This will clear cache so UI should be also cleared and request new data,
	*/
	static proto void SetSize(int size);
	//! Returns currently set size of catalogue for pages/offsets.
	static proto int GetSize();
	/*!
	Returns server that client is currently in process of joining to.
	It is set by initiating RequestJoin() on ServerInfo and should be cleared during join transition in native c++ implementaiton.
	*/
	static proto ServerInfo GetJoiningServer();
	/*!
	Returns server which was used in some of the last operations.
	- last joined server
	\note Server persist between game reloads but not game restarts/crashes.
	*/
	static proto ServerInfo GetLastServer();
	//! Returns current mode of catalogue which determines its source of data
	static proto EServerCatalogueMode GetMode();
	/*!
	Will request load of data for specific page of catalogue.
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Pages are indexed from 0. Int is casted into uint32.
	\note This method should be used for paging UI where single page with fixed size and offset is visible at the time.
	      It should not be used for scrolling implementation which can possibly show items from 2 pages at once which can introduce issues.
	*/
	static proto void RequestPage(notnull BackendCallback pCallback, int page);
	/*!
	Will request load of data for offset from first item
	If data are already cached then callback is invoked immediately with this method and no request is sent to the backend.

	Offset 0 represent first item. Int is casted into uint32.
	\note This method should be used for scrolling UI - there is no fixed position where page begins and ends.
	      It can be used even for paging UI implementation but it is recommended to use RequestPage() method for simplicity.
	*/
	static proto void RequestOffset(notnull BackendCallback pCallback, int offset);
	/*!
	Will request forced refresh of currently loaded page/offset.
	Items will be cleared from cache and created again from received data
	which might be possibly different.
	*/
	static proto void RequestRefresh(notnull BackendCallback pCallback);
	/*!
	Will set BackendCallback used for soft refresh of currently shown page/offset.
	Difference with directly requesting refresh is that it will just update data
	without any clear and is done automatically in intervals.
	UI should refresh shown data every time when OnSuccess is invoked on the callback.
	*/
	static proto void SetSoftRefreshCallback(notnull BackendCallback pCallback);
	/*!
	Will set pointer to filters object for the catalogue.
	Filters should be set only once at the beginning and then you can just modify local instance of filters.
	If set to null then ServerCatalogueApi will provide all servers which might not be desirable.
	*/
	static proto void SetFilters(ServerCatalogueFilters filters);
	/*!
	Will return pointer to object of currently applied filters for the catalogue.
	\note Can return null if no filters are set in the catalogue.
	      In that case ServerCatalogueApi will provide all servers registered on the backend.
	*/
	static proto ServerCatalogueFilters GetFilters();
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	\throw VME if field was already appended for sorting.
	*/
	static proto bool AppendOrderBy(EBackendCatalogueOrderDir orderDir, EServerCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list
	static proto void ClearOrderBy();
	//! Getter for items of currently loaded page/offset.
	static proto void GetItems(out array<ServerInfo> serversOut);
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
}

/*!
\}
*/
