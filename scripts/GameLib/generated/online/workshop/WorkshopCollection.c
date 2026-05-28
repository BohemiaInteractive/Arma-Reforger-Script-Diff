/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

/*!
This structure represent collection of multiple workshop assets alongside specific or latest revisions.

\note Currently this is used only for modded servers to easily group necessary assets for connection.
*/
sealed class WorkshopCollection
{
	private void WorkshopCollection();
	private void ~WorkshopCollection();

	/*!
	Returns total count of how many assets are in the collection.
	\warning assets which encountered error are also included in this count.
	*/
	proto external int GetCount();
	//! Returns count of how many assets/revision encountered workshop side errors.
	proto external int GetErrorsCount();
	/*!
	Returns list of sorted collection items.
	\note Items are sorted by load order and items with errors always first.
	*/
	proto external void GetItems(out notnull array<WorkshopCollectionItem> itemsOut);
}

/*!
\}
*/
