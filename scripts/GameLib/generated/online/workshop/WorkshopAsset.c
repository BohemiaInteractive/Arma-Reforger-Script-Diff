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
sealed class WorkshopAsset: WorkshopAssetBase
{
	private void WorkshopAsset();
	private void ~WorkshopAsset();

	//! Returns full description of this asset.
	proto external string GetDescription();
	/*!
	Provides array of all revisions which are still available on the workshop
	and can be listed with details or even downloaded.

	\note List is sorted from latest to most old revision.
	      First element should always be equal to the LatestRevision
	      except if it is not know and is null.
	*/
	proto external void GetAvailableRevisions(out notnull array<WorkshopRevisionBase> revisionsOut);
	/*!
	Provides array of all screenshots available on this asset.
	\note Screenshot can be possibly null in case of some issue or other reason
	      and should be properly handled by either skipping or using some error
	      image.
	*/
	proto external void GetScreenshots(out notnull array<BImage> screenshotsOut);
}

/*!
\}
*/
