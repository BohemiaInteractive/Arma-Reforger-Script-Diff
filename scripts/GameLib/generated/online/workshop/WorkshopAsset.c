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
	Returns comment catalogue of this asset.
	\note Only a single instance of BackendCommentCatalogue exists. Therefore, only comments
	      of a single asset can be listed at a time. Calling this getter for a different asset will
	      clear the previously loaded comment catalogue.
	*/
	proto external BackendCommentCatalogue GetCommentCatalogue();
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
