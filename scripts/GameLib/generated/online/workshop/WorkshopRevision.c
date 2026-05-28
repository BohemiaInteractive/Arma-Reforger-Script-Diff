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
Final structure of the workshop asset which contains all data and functionality.
*/
sealed class WorkshopRevision: WorkshopRevisionBase
{
	private void WorkshopRevision();
	private void ~WorkshopRevision();

	//! Returns full changelog of this revision.
	proto external string GetChangelog();
	/*!
	Will request download of manifest for this and other revision into memory which is necessary for calculation of patch size between them.
	\note If both manifests are already cached in memory then this method will call OnSuccess immediately and no request is performed.
	*/
	proto external void RequestDeltaSize(notnull BackendCallback callback, notnull WorkshopRevisionBase other);
	/*!
	Will calculate delta patch size from this to other revision - how much data will be needed to get from this to other revision.
	\warning This does not represent exact amount of how much data will be downloaded because data could be possibly reused
	         if they exist in different places.
	         But it can still indicate how much data at most would user need to download.
	         Exact size to download has to be deeply calculated and will be provided by active job.
	*/
	proto external int GetPatchSizeKB(notnull WorkshopRevisionBase other);
	/*!
	Returns pointer to the license object of this revision.
	\note if this returns null then revision does not have any specific
	      license and the most restrictive rules apply for this revision.
	*/
	proto external WorkshopLicense GetLicense();
}

/*!
\}
*/
