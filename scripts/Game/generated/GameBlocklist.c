/*
===========================================
Do not modify, this script is generated
===========================================
*/

class GameBlocklist
{
	ref ScriptInvoker<bool> OnBlockListUpdateInvoker = new ScriptInvoker<bool>();
	ref ScriptInvoker<Room, array<BlockedRoomPlayer>> OnGetBlockedPlayersInRoomInvoker = new ScriptInvoker<Room, array<BlockedRoomPlayer>>();

	/*!
	Requests update of block list using BlockListApi.
	Invokes OnBlockListUpdate when finished.
	*/
	proto external void UpdateBlockList();
	/*!
	Requests to check blokced players in a room. Invokes
	OnGetBlockedPlayersInRoom when finished. Deleting a Room parameter
	or calling before completion cancels previous request
	*/
	proto external void CheckBlockedPlayersInRoom(notnull Room room);
	/*!
	Fills the provided array with the informations about blocked players.
	\return Number of blocked players
	*/
	proto external int GetBlockedPlayers(notnull array<BlockListItem> outItems);
	/*!
	Blocks given player using BlockListApi. Uses provided callback as result.
	In case of full block list, error EBERR_STORAGE_IS_FULL is reported.
	*/
	proto external void Block(BackendCallback callback, int otherPlayerID);
	/*!
	Removes given player from game block list. Invokes callback when requestt completes.
	\return false on failure or when player is not blocked throught a game block list.
	*/
	proto external bool Unblock(BackendCallback callback, int otherPlayerID);
	/*!
	Determines if otherPlayer is blocked by this player. All player interactions
	are restricted if one player is blocked. Returns false in case of invalid argument.
	\param identity Game backend identity id
	\param platform Platform of other player
	\param platformIdHash Hash of platform specific identifier
	*/
	proto external bool IsBlockedIdentity(string identity, PlatformKind platform, string platformIdHash);

	// callbacks

	//! Event invoked as a result of UpdateBlockList
	event protected void OnBlockListUpdate(bool success) { OnBlockListUpdateInvoker.Invoke(success); };
	/*!
	Event invoked as a result of CheckBlockedPlayersInRoom.
	\param blockedPlayers is null in case of failed request
	*/
	event protected void OnCheckedBlockedPlayersInRoom(Room room, array<BlockedRoomPlayer> blockedPlayers) { OnGetBlockedPlayersInRoomInvoker.Invoke(room, blockedPlayers); };
}
