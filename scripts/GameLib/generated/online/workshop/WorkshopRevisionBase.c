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
	//! Returns the time when this revision was created
	proto external Timestamp GetCreatedAt();
	//! Returns count of how many dependencies this revision has.
	proto external int GetDependencyCount();
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
	\note Creates a download job for the asset: EWJT_DOWNLOAD (fresh), EWJT_UPDATE (newer revision present)
	      or EWJT_DOWNGRADE (older revision present), depending on what is already installed.
	\note At most one job exists per asset. If a job of the same task is already in history for this asset it is
	      reused; a job of a different task is deleted and a new one is created.
	\warning State of the asset must be at least SUBSCRIBED before you can download this revision.
	\throws VME - if download is requested while asset is not subscribed.
	            - if download is requested while this asset is already processing different job
	            - if download is requested for revision which is already downloaded - is current revision
	\returns WorkshopJob of this task through which download process can be tracked and further controlled.
	*/
	proto external WorkshopJob RequestDownload(notnull BackendCallback callback);
	/*!
	Will request a read-only validation of the locally installed data of this asset.
	      The validation mode is decided internally: offline (validate the manifest against
	      its own header) when not authenticated or running with -noBackend, otherwise full online
	      (validate against backend-authoritative manifest data).
	\note Creates a validation job for the asset.
	\note At most one job exists per asset. If a job of the same task is already in history for this asset it is
	      reused; a job of a different task is deleted and a new one is created.
	\note A validation job cannot be truly paused - pausing and resuming it restarts the validation from the
	      beginning.
	\warning Asset must have local data present.
	         Fails if the asset is already processing a different job.
	\returns WorkshopJob of this task through which the validation can be tracked.
	         The result is stored on the asset - see WorkshopAssetBase::IsCorrupted()/GetValidationRepairBytes().
	*/
	proto external WorkshopJob RequestValidation(notnull BackendCallback callback);
	/*!
	Will repair the locally installed data of this asset by re-downloading only the corrupted or missing fragments.
	\note Creates a repair job for the asset (EWJT_REPAIR).
	\note At most one job exists per asset. If a job of the same task is already in history for this asset it is
	      reused; a job of a different task is deleted and a new one is created.
	\warning Asset must have local data present.
	         Fails if the asset is already processing a different job.
	\returns WorkshopJob of this task through which the repair can be tracked.
	*/
	proto external WorkshopJob RequestRepair(notnull BackendCallback callback);
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
