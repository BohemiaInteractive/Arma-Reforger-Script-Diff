/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class WorkshopCatalogue
{
	private void WorkshopCatalogue();
	private void ~WorkshopCatalogue();

	//! Returns max possible size of page that is allowed to be set
	proto external int MaxPageSize();
	/*
	\brief Request specific page with items
	\param pCallback Is script callback where you will receive result/error when request finishes
	\param pParams Are params of PageParams type to specify which page to request
	\param bClearCache Specify is cached pages should be cleared and force request to be sent again to retrieve latest data
	*/
	proto external void RequestPage(BackendCallback pCallback, notnull PageParams pParams, bool bClearCache);
	/*
	\brief Get total item count on all pages
	\returns total item count
	*/
	proto external int GetTotalItemCount();
	/*
	\brief Get page count
	\returns page count
	*/
	proto external int GetPageCount();
	//! \brief Set number of items per page
	[Obsolete("Use SetPageSize() instead!")]
	proto external void SetPageItems(int iCount);
	//! \brief Set number of items per page
	proto external void SetPageSize(int iCount);
	/*
	\brief Get current page number
	\return Current page number
	*/
	proto external int GetPage();
	/*
	\brief Get item count on actual page
	\return Current count of items on active page
	*/
	proto external int GetPageItemCount();
	/*
	\brief Get page content.
	\param items Array of Workshop Items
	\return Current count of items on active page
	*/
	proto external int GetPageItems(out notnull array<WorkshopItem> items);
}

/*!
\}
*/
