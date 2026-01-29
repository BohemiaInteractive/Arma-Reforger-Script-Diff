/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class ClientLobbyApi
{
	private void ClientLobbyApi();
	private void ~ClientLobbyApi();

	/*
	\brief Update current position.
	*/
	proto external void Scroll(int position, BackendCallback callback);
	/*
	\brief Get room data for current position.
	*/
	proto external int Rooms(out notnull array<Room> rooms);
	/*
	\brief Get rooms searched by SearchTarget
	*/
	proto external int Target(out notnull array<Room> rooms);
	/*
	\brief Search first batch of rooms.
	*/
	proto external void SearchRooms(JsonApiStruct searchParams, BackendCallback callback);
	/*
	\brief Search for a specific room and keep cached data
	*/
	proto external int SearchTarget(JsonApiStruct searchParams, BackendCallback callback);
	/*
	\brief Get rooms searched by theirs Ids
	*/
	proto external void GetRoomsByIds(GetRoomsIds searchParams, BackendCallback callback);
	/*
	\brief Get rooms searched by theirs Host Ids
	*/
	proto external void GetRoomsByHostIds(GetRoomsIds searchParams, BackendCallback callback);
	/*
	\brief View size is expected size of rooms from Rooms(rooms).
	*/
	proto external void SetViewSize(int size);
	/*
	\brief The total room count by the current filter.
	*/
	proto external int TotalRoomCount();
	/*
	\brief Set callback for periodical room refresh
	*/
	proto external void SetRefreshCallback(BackendCallback callback);
	/*!
	Set a time span between page updates (in ms).
	\note Minimal value is 20000 ms.
	*/
	proto external void SetRefreshRate(int iRate);
	/*
	\brief Get current room that client is currently attempting to join
	*/
	proto external Room GetJoinRoom();
	/*
	\brief Get room that client was invited into
	*/
	proto external Room GetInviteRoom();
	/*
	\brief Clear the room from GetInviteRoom()
	*/
	proto external void ClearInviteRoom();
	/*
	\brief Returns true if invitation failed
	*/
	proto external bool InvitationFailed();
	/*
	\brief Get parameters created in the current run.
	*/
	proto external JsonApiStruct GetParameters();
	/*
	\brief Serialize params before reload
	*/
	proto external void StoreParams();
	/*
	\brief Get serialized parameters
	*/
	proto external string GetStrParams();
	/*
	\brief Clear serialized data
	*/
	proto external void ClearParams();
	/*
	\brief	In case of a disconnection you can retrieve the previous room ID.
			It should be cleared after that to prevent acquiring the same id twice.
	*/
	proto external string GetPreviousRoomId();
	/*
	\brief Clears PreviousRoomId
	*/
	proto external void ClearPreviousRoomId();
	/*
	\brief	Get automatically detected IP of the network adapter.
			TODO: This will be obsolete when the proper LAN server support is ready
	*/
	proto external string GetMyIP();
	/*
	\brief Find the room by its ID in cached data
	*/
	proto external Room GetRoomById(string id);
	/*
	\brief True if ping data to the ping sites are ready to use
	*/
	proto external bool IsPingAvailable();
	/*
	\brief Retrieve the pings sites and start to measure latency.
	*/
	proto external void MeasureLatency(BackendCallback callback);
	/*
	\brief Triggers OnError/OnTimeout in case the previous listen server initialization failed.
	*/
	proto external void ProcessLastHostError(BackendCallback callback);
}

/*!
\}
*/
