/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class WorkshopAuthor: WorkshopCatalogue
{
	private void WorkshopAuthor();
	private void ~WorkshopAuthor();

	/*
	\brief Get author name.
	\return name of author
	*/
	proto external string Name();
	/*
	\brief Is this author blocked for client
	\return true if author is blocked
	*/
	proto external bool IsBlocked();
	/*
	\brief Add author to clients block list for workshop - authors items will not be shown
	\param pCallback Is script callback where you will receive result/error when request finishes
	*/
	proto external void AddBlock(BackendCallback callback);
	/*
	\brief Remove author from clients block list for workshop
	\param pCallback Is script callback where you will receive result/error when request finishes
	*/
	proto external void RemoveBlock(BackendCallback callback);
	/*
	\brief Get locally downloaded workshop items of this author
	\param items Array of Workshop Items
	\return Current count of downloaded items from this author
	*/
	proto external int GetOfflineItems(out notnull array<WorkshopItem> items);
}

/*!
\}
*/
