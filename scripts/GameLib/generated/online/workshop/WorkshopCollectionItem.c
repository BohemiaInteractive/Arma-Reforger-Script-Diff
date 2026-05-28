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
This is data structure which encapsulates individual asset and its specific revision.
If we failed to fetch details for this asset or revision then result will not be OK.
*/
sealed class WorkshopCollectionItem
{
	private void WorkshopCollectionItem();
	private void ~WorkshopCollectionItem();

	/*!
	Returns GUID of this asset related to this collection item.
	\note this should return GUID always even if we failed to fetch asset from workshop.
	*/
	proto external string GetGUID();
	/*!
	Returns name of asset related to this collection item.
	\note Name can be sometimes available even if we failed to fetch details about asset and revision.
	\warning It is not always guaranteed that it will be possible to get name of asset.
					 If this returns empty string the you should always at least fall back to show GUID of failed asset.
	*/
	proto external string GetName();
	/*!
	Returns version of revision related to this collection item.
	\warning It is not always guaranteed that there will be some version since it might have been
					 desired that workshop should resolve latest version but failed due to some reason.
					 If this returns empty string then it means that version was not specified and workshop could not resolve it.
	*/
	proto external string GetVersion();
	/*!
	Returns result for this collection item.
	\warning if it returns anything else than EACODE_ERROR_OK then item encountered error.
	*/
	proto external EApiCode GetResult();
	/*!
	Returns Asset of this collection item.
	\warning Returns null if result is not OK.
	*/
	proto external WorkshopAssetBase GetAsset();
	/*!
	Returns Revision of this collection item.
	\warning Returns null if result is not OK.
	*/
	proto external WorkshopRevisionBase GetRevision();
}

/*!
\}
*/
