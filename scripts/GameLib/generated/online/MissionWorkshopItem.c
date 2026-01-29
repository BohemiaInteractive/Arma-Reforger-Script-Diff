/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class MissionWorkshopItem: Managed
{
	private void MissionWorkshopItem();
	void ~MissionWorkshopItem();

	//! Returns Id of mission header.
	proto external ResourceName Id();
	//! Returns pointer to Header of this mission.
	proto external Class GetHeader();
	//! Returns ID of WorkshopItem that owns this mission.
	proto external string GetOwnerId();
	//! Returns WorkshopItem that owns this mission.
	proto external WorkshopItem GetOwner();
	//! Returns thumbnail image for this mission.
	proto external BackendImage Thumbnail();
	//! Returns author name of this mission.
	proto external string Author();
	//! Requests reload of game with addons and launch mission.
	proto external void Play();
	//! Requests reload of game with addons and launch server with this mission.
	proto external void Host(DSConfig config);
	//! Returns time since last time this mission was played. Returns -1 if never played.
	proto external int GetTimeSinceLastPlay();
	//! Returns true if mission is set as favorite of current user.
	proto external bool IsFavorite();
	//! Get name of the item
	proto external string Name();
	//! Get description of the item
	proto external string Description();
	//! Set favorite state on mission for current user.
	proto external void SetFavorite(bool isFavorite);
	//! Returns player count defined for this mission.
	proto external int GetPlayerCount();
}

/*!
\}
*/
