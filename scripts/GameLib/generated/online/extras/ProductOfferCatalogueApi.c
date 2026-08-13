/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_extras
\{
*/

sealed class ProductOfferCatalogueApi
{
	private void ProductOfferCatalogueApi();
	private void ~ProductOfferCatalogueApi();

	/*!
	Sets filters for the catalogue.
	If set to null then no filters will be applied.
	*/
	static proto void SetFilters(ProductOfferCatalogueFilters pFilters);
	//! Returns currently set filters, or null if none are set.
	static proto ProductOfferCatalogueFilters GetFilters();
	/*!
	Will append new order by field for sorting catalogue. Once field is appended it cannot be reused until clear.
	\throws VME - if field was already appended for sorting.
	           - if you try to exceed limit of categories to order by.
	*/
	static proto bool AppendOrderBy(EBackendCatalogueOrderDir orderDir, EProductOfferCatalogueOrderBy orderBy);
	//! Will clear ordering of catalogue list.
	static proto void ClearOrderBy();
	/*!
	Will set size of catalogue which determines how many items fit into a single page/offset.
	\note This will clear cache so UI should be cleared and request new data.
	*/
	static proto void SetSize(int size);
	//! Returns currently set size of catalogue for pages/offsets.
	static proto int GetSize();
	/*!
	Will request load of data for a specific page of catalogue.
	If data are already cached then callback is invoked immediately.
	\throws VME - if size was not yet set via SetSize()
	           - if no order by was set yet via AppendOrderBy()
	*/
	static proto void RequestPage(notnull BackendCallback pCallback, int uPage);
	/*!
	Will request load of data for an offset from the first item.
	If data are already cached then callback is invoked immediately.
	\throws VME - if size was not yet set via SetSize()
	           - if no order by was set yet via AppendOrderBy()
	*/
	static proto void RequestOffset(notnull BackendCallback pCallback, int uOffset);
	/*!
	Will request forced refresh of the currently loaded page/offset.
	\throws VME - if size was not yet set via SetSize()
	           - if no order by was set yet via AppendOrderBy()
	*/
	static proto void RequestRefresh(notnull BackendCallback pCallback);
	//! Returns count of how many pages exist with the current configuration.
	static proto int GetPageCount();
	//! Returns index of the currently loaded page. Index 0 is the first page.
	static proto int GetPageNumber();
	//! Returns total count of items with the current configuration.
	static proto int GetTotalItemCount();
	//! Returns index of the first item in the currently loaded offset.
	static proto int GetOffsetIndex();
	//! Returns count of items on the currently loaded page/offset.
	static proto int GetCurrentItemCount();
	//! Getter for items of the currently loaded page/offset.
	static proto int GetItems(out notnull array<ProductOffer> offersOut);
	/*!
	Fetches all labels from the backend.
	\note This should be called before using the catalogue to properly view labels of all products.
	*/
	static proto void RequestLabels(notnull BackendCallback pCallback);
}

/*!
\}
*/
