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
Base structure for all assets from workshop containing partial data of the full asset.
*/
sealed class WorkshopAssetBase
{
	private void WorkshopAssetBase();
	private void ~WorkshopAssetBase();

	/*!
	Specifies if this asset should be enabled. If asset is set to enabled then it will
	be later loaded when possible. This behavior depends on type of the asset and would be
	generally handled internally.

	\note If asset is currently in state other than ENABLED/DISABLED then this method
	      will cache value and set it later when possible.
	      For example if state is downloading and you set enabled to false (DISABLED) then when it
	      finishes the download it will be set to DISABLED.
	      Method WorkshopAssetBase::GetEnabledState() will provide information to which state
	      it is expected to be set if possible.
	      Reference to WorkshopAssetBase::GetState() to see current state and EWorkshopAssetState for
	      possible states.
	*/
	proto external void SetEnabledState(bool enabled);
	/*!
	Returns desired state the asset will set as default state when possible.
	Only possible values this method returns are ENABLED or DISABLED which
	can be changed via WorkshopAssetBase::SetEnabledState() method.
	*/
	proto external EWorkshopAssetState GetEnabledState();
	/*!
	Returns current state of locally available (downloaded) revision of this asset.
	Some operations are not possible to perform in specific states which will be always
	mentioned in relevant methods.

	\note Headless client (Dedicated Server) will always consider all assets SUBSCRIBED
	      so it does not differentiate between SUBSCRIBED and NONE state.
	*/
	proto external EWorkshopAssetState GetState();
	/*!
	Returns true if asset has corrupted data and should not be loaded.
	If it is true then GetState() should also return CORRUPTED state or other of higher priority.
	*/
	proto external bool IsCorrupted();
	/*!
	Returns true if asset has some data locally on the drive.
	\note Asset can have data even in state in which it would not be expected to have them.
	      States like NONE or BLOCKED should expect no data but if they are present they would
	      be considered as HANGING DATA which should be either removed or change state into
	      appropriate one for which it is valid to have some data present.
	*/
	proto external bool HasData();
	/*!
	Returns currently pending job of this asset.
	\note Should only return value when state of the asset is PENDING_JOB. Otherwise returns null.
	*/
	proto external WorkshopJob GetPendingJob();
	/*!
	Returns GUID of this asset.
	\note This should serve as informative data for the user to see since
				loading or operations with assets is handled by workshop systems.
	*/
	proto external string GetGUID();
	//! Returns name of this asset.
	proto external string GetName();
	//! Returns short summary text of this asset.
	proto external string GetSummary();
	/*!
	Returns visibility of this asset.
	\note Almost all assets will have public visibility and only author, administrator
	      or user with additional data or permissions might see asset with different visibility.
	      Refer to filters in WorkshopCatalogueFilters to see which options would make assets
	      which are not by default PUBLIC visible in listing.
	*/
	proto external EWorkshopAssetVisibility GetVisibility();
	//! Returns array containing all tag objects on this asset.
	proto external void GetTags(out notnull array<WorkshopAssetTag> tagsOut);
	//! Returns total count of how many tags are on this asset.
	proto external int GetTagsCount();
	//! Returns object containing data about author of this asset.
	proto external WorkshopUserProfile GetAuthor();
	/*!
	Return latest revision available on the workshop.
	\note if this returns null then it means that only locally cached data were
	      loaded and latest data from the workshop were not yet fetched.

	      TODO: These data should be generally fetched during INIT of WorkshopCatalogue
	*/
	proto external WorkshopRevisionBase GetLatestRevision();
	/*!
	Return revision which is already downloaded and locally available.

	This will return null if no revision is downloaded or if some revision
	is being downloaded without any previous revision present.

	During update this will still return previous revision until update is finished.
	*/
	proto external WorkshopRevisionBase GetCurrentRevision();
	/*!
	Returns preview image of the asset.
	\note Preview should be required by the workshop so this should always return some image.
	      If it returns null then it is expected to be handled as error of the image but without
	      blocking core functionality of asset.
	*/
	proto external BImage GetPreviewImage();
	/*!
	Returns icon image of the asset.
	\note Icon is optional for the asset so it should be expected to return both some value or null
	      and be valid states.
	*/
	proto external BImage GetIconImage();
	/*!
	This will delete all locally available data of asset on the drive.
	Returns true if delete was successful or if there are no data and false if it failed to delete some data.
	\note If this fails then asset will be marked as CORRUPTED and user might need to delete
	      data manually via systems file manager. This might be caused by insufficient permissions
	      or some other process blocking the deletion of data.
	\warning This does not unsubscribe asset which must be done separately via RequestSubscribe.
	*/
	proto external bool DeleteData();
	/*!
	Will request change of subscribe state to this asset.

	When asset is successfully subscribed then this will switch current state into SUBSCRIBED and
	in that state the download of revision for this asset will be allowed.
	If asset is unsubscribed then current state is set back to NONE.

	\warning Subscribing to blocked asset will automatically unblock the asset.
	         Scripts and UI should inform user about this operation before actually requesting it.
	         Headless client (Dedicated Server) cannot perform this operation!

	\throws VME - if there is already pending request
	            - if you attempt to change subscription into state it currently is in.
	            - if requested on Headless client
	*/
	proto external void RequestSubscribe(notnull BackendCallback callback, bool subscribed);
	//! Returns count of how many users have currently subscribed this asset.
	proto external int GetSubscriberCount();
	//! Returns count of how many unique users have ever downloaded this asset.
	proto external int GetUniqueDownloadCount();
	/*!
	Will request submit of rating to this asset.
	\param rating - normalized floating-point value in the range 0.0f to 1.0f
	\note rating is represented as normalized value but this is just an abstraction and in UI or later calculations
	      can be used in different formats:
	      - Percentage - 0.0f = 0% to 1.0f = 100%
	      - Like/Dislike - 0.0f = dislike / 1.0f = like
	      - Stars (0-5) - normalized scale with 0.2f steps, 0.0f = 0 stars, 0.2f = 1 star, 0.4f = 2 stars...
	      - Stars (0-5 with half steps) - same as previous with finer granularity (0.1f = half star)

	\warning Headless client (Dedicated Server) cannot perform this operation!

	\throws VME - if there is already pending request
	            - if rating is out of ranges
	            - if requested on Headless client
	*/
	proto external void RequestRate(notnull BackendCallback callback, float rating);
	/*!
	Returns current rating of this asset calculated from all ratings submitted by other users
	as value 0.0f to 0.1f.
	Refer to note in WorkshopAssetBase::RequestRate for more info how rating values work.
	*/
	proto external float GetRating();
	/*!
	Returns current personal rating of this asset as value 0.0f to 0.1f.
	If user did not submit any rating for this asset then returns -1.
	Refer to note in WorkshopAssetBase::RequestRate for more info how rating values work.
	*/
	proto external float GetMyRating();
	//! Returns total count of how many users submitted rating.
	proto external int GetRatingCount();
	/*!
	Will request change of blocked state to this asset.

	When asset is successfully blocked then this will switch current state into BLOCKED.
	If asset was previously SUBSCRIBED and downloaded then it should be deleted.
	If asset is unblocked then current state is set back to NONE.

	\warning Blocking asset which is subscribed will automatically unsubscribe it!
	         Scripts and UI should inform user about this operation before actually requesting it.
	         Headless client (Dedicated Server) cannot perform this operation!

	\throws VME - if you attempt to change block into state it currently is in.
	            - if requested on Headless client
	*/
	proto external void RequestBlock(notnull BackendCallback callback, bool block);
	/*!
	Will request submit of report against this asset and can be also optionally blocked.

	\param type - type category of the report - needs to be defined by game projects.
	\param description - optional description of the report from user limited to max 1000 characters.
	\param block - optionally asset can be also blocked alongside the report but this cannot be used for unblock.

	\warning Headless client (Dedicated Server) cannot perform this operation!

	\throws VME - if length of the description exceeds limits.
	            - if requested on Headless client
	*/
	proto external void RequestReport(notnull BackendCallback callback, EWorkshopReportTypes type, string description, bool block);
	/*!
	Will request missing details about this asset. Generally when you have partial asset from browsing workshop list
	or when local asset is loaded at the start of the game.
	After missing data are successfully fetched you can also retrieve full WorkshopAsset via GetWorkshopAsset().
	\note If all details are already loaded then this method finished immediately without sending any request.

	*/
	proto external void RequestDetails(notnull BackendCallback callback);
	/*!
	Returns full WorkshopAsset structure of this if it was previously requested with successful result.
	\warning Casting this into WorkshopAsset can be also possible but could introduce undesired behavior
	         due to missing validation checks.
	\throws VME - if you attempt to get full asset when it was previously not requested.
	*/
	proto external WorkshopAsset GetWorkshopAsset();
}

/*!
\}
*/
