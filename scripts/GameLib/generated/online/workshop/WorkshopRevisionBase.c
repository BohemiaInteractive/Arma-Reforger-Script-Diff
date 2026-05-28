/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

sealed class WorkshopRevisionBase
{
	private void WorkshopRevisionBase();
	private void ~WorkshopRevisionBase();

	//! Returns true if this is latest revision of its asset.
	proto external bool IsLatest();
	//! Return true if this is current revision of its asset.
	proto external bool IsCurrent();
	//! Returns version numbers as a string.
	proto external string GetVersion();
	/*!
	Returns total size of this revision in KibiBytes.
	\note If size is less than 1 KB it is always rounded to 1.

	\warning This does not represent how many data will user download
	         but can be used to determine how much space will this revision
	         take on disk.
	         TODO: For download and update there will be method to calculate delta patch size
	               which will specify how much data will need to be downloaded in relation
	               to data user already has downloaded in this or other assets.
	*/
	proto external int GetSizeKB();
	/*!
	Will request download of this revision.
	\warning State of the asset must be at least SUBSCRIBED before you can download this revision.
	\throws VME - if download is requested while asset is not subscribed.
	            - if download is requested while this asset is already processing different job
	            - if download is requested for revision which is already downloaded - is current revision
	\returns WorkshopJob of this task through which download process can be tracked and further controlled.
	*/
	proto external WorkshopJob RequestDownload(notnull BackendCallback callback);
	/*!
	Will request missing details about this revision. Generally when you have partial revision from assets
	or outdated data when local asset is loaded at the start of the game.
	After missing data are successfully fetched you can also retrieve full WorkshopRevision via GetWorkshopRevision().
	\note If all details are already loaded then this method finished immediately without sending any request.

	*/
	proto external void RequestDetails(notnull BackendCallback callback);
	/*!
	Returns full WorkshopRevision structure of this if it was previously requested with successful result.
	\warning Casting this into WorkshopRevision can be also possible but could introduce undesired behavior
	         due to missing validation checks.
	\throws VME - if you attempt to get full asset when it was previously not requested.
	*/
	proto external WorkshopRevision GetWorkshopRevision();
}

/*!
\}
*/
