/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class BaseWorkshopItem: Managed
{
	private void BaseWorkshopItem();
	void ~BaseWorkshopItem();

	//! Returns true if item is currently processing with Workshop - is downloading.
	proto external bool IsProcessed();
	//! Get name of the item
	proto external string Name();
	//! Returns thumbnail image of this item.
	proto external BackendImage Thumbnail();
	//! Returns latest Revision of this item.
	proto external Revision GetLatestRevision();
	//! Returns currently downloaded Revision of item.
	proto external Revision GetActiveRevision();
	//! Returns currently pending/downloading Revision
	proto external Revision GetPendingDownload();
	//! Returns currently downloading Revision (will return null if it is paused)
	proto external Revision GetDownloadingRevision();
	//! Returns pseudo Revision for local non-workshop item.
	proto external Revision GetLocalRevision();
	/*!
	Provides array of Revision history of X latest revisions.
	Limit of how many revisions is provided is specified by Workshop so it may differ.
	\param[out] revisions - array containing X latest revisions for this item.
	\returns int - count of revisions
	*/
	proto external int GetRevisions(out notnull array<Revision> revisions);
	//! Get description of the item
	proto external string Description();
	//! Returns Id of item.
	proto external string Id();
	//! Returns Summary text of this item.
	proto external string Summary();
	//! Returns current state flags
	proto external int GetStateFlags();
}

/*!
\}
*/
