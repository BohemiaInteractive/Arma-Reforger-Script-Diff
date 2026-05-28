/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

//! Workshop Catalogue ordering values.
enum EWorkshopCatalogueOrderBy
{
	//! ordered by assets IDs.
	ID,
	//! ordered by assets names.
	NAME,
	//! ordered by assets popularities prioritizing recently newly released.
	POPULARITY_NEW,
	//! ordered by assets popularities regardless of when it was released.
	POPULARITY_ALL_TIME,
	//! ordered by assets overall ratings.
	RATING,
	//! ordered by how many people rated assets.
	RATING_COUNT,
	//! ordered by how many people have currently subscribed those assets (should have downloaded right now).
	SUBSCRIBER_COUNT,
	//! ordered by how many people have ever downloaded those assets.
	UNIQUE_DOWNLOAD_COUNT,
	//! ordered by total size of latest revision of assets.
	LATEST_REVISION_SIZE,
	//! ordered by time when assets had latest update - latest revision created.
	LATEST_REVISION_CREATED_AT,
	//! ordered by time when the assets were created.
	CREATED_AT,
}

/*!
\}
*/
