/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

class WorkshopCatalogueFilters: Managed
{
	/*!
	Full-text query search with 200 characters of length limit.
	\note Defaults to "" (any).
	\throws VME - if length is exceeded
	*/
	proto external void SetSearchText(string searchText);
	//! Returns current full-text query search filter.
	proto external string GetSearchText();
	/*!
	Filter any/subscribed assets.
	\note Defaults to ANY.
	      This filter will show even assets with unlisted visibility since you already
	      know its GUID because you have it subscribed.
	\warning FALSE to hide all subscribed assets is not allowed and will throw VME.
	*/
	proto external void SetSubscribed(EBackendCatalogueBoolFlag val);
	//! Returns current only subscribed filter.
	proto external EBackendCatalogueBoolFlag GetSubscribed();
	/*!
	Filter only assets which are blocked.
	\note Defaults to false (any).
	      This filter will show even assets with unlisted visibility since you already
	      know its GUID because you have it blocked.
	\warning FALSE to hide all blocked assets is not allowed and will throw VME.
	*/
	proto external void SetBlocked(EBackendCatalogueBoolFlag val);
	//! Returns current only blocked filter.
	proto external EBackendCatalogueBoolFlag GetBlocked();
	/*!
	Filter only assets which are rated with any ratings.
	\note Defaults to false (any).
	      This filter will show even assets with unlisted visibility since you already
	      know its GUID because you have it rated.
	\warning FALSE to hide all rated assets is not allowed and will throw VME.
	*/
	proto external void SetRated(EBackendCatalogueBoolFlag val);
	//! Returns current only blocked filter.
	proto external EBackendCatalogueBoolFlag GetRated();
	//! Filter only assets owned by specific author. GameIdentity UUID is copied from the user profile instance.
	proto external void SetAuthor(WorkshopUserProfile author);
	/*!
	Filter only assets owned by specific author identified by his GameIdentity UUID.
	\note Use this only when you dont have any instances of WorkshopUserProfile for the desired author.
	      Otherwise prefer the SetAuthor variant.
	*/
	proto external void SetAuthorUUID(UUID authorUUID);
	//! Returns current author filter.
	proto external UUID GetAuthorUUID();
	/*!
	Filter assets which have any kind author or only assets with/without official author.
	\note Defaults to any.
	*/
	proto external void SetAuthorIsOfficial(EBackendCatalogueBoolFlag val);
	//! Returns current official author filter - any/with/without official author.
	proto external EBackendCatalogueBoolFlag GetAuthorIsOfficial();
	/*!
	Filter only assets with specific IDs.
	\note this filter will show even assets with unlisted visibility since you provided its ID.
	\warning Entries with invalid format will be ignored.
	         Duplicate entries will be de-duplicated.
	*/
	proto external void SetAssetIds(array<string> assetIds);
	/*!
	Filter only assets which tags.
	\note Nested array will be treated as a logical OR between the tags e.g:
	      [["vehicle"], ["small", "medium"], ["slow", "fast"]] would result in this filter: "vehicle" && ("small" || "medium") && ("slow" || "fast").
	      Defaults to empty array (any).
	\throws VME - if some OR logic operation array is null or empty
	            - if some tag does not meet or exceeds the character limits (MIN 2 and MAX 30)
	*/
	proto external void SetIncludeTags(array<ref array<string>> tags);
	/*!
	Filter only assets without tags.
	\note See SetIncludesTags for the input type explanation
	      Defaults to empty array (any).
	\throws VME - if some OR logic operation array is null
	            - if some tag does not meet or exceeds the character limits (MIN 2 and MAX 30)
	*/
	proto external void SetExcludeTags(array<ref array<string>> tags);
}

/*!
\}
*/
