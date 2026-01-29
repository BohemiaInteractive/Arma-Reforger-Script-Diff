/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class Room
{
	private void Room();
	private void ~Room();

	/*
	\brief Returns true for official dedicated server and false for community servers.
	*/
	proto external bool Official();
	/*
	\brief Does the server require a password to join.
	*/
	proto external bool PasswordProtected();
	/*
	\brief Returns false if the server is not ready to receive players (e.g. is restarting / has some network issues / just crashed etc.)
	*/
	proto external bool Joinable();
	/*
	\brief Load informations about hosted addons. User has to be authorized first.
	*/
	proto external void LoadDownloadList(BackendCallback callback);
	/*
	\brief Is the list of hosted addons ready or should we load it (LoadDownloadList)
	*/
	proto external bool IsDownloadListLoaded();
	/*
	\brief Returns supported game version of server
	*/
	proto external string GameVersion();
	/*
	\brief Returns region in which server is located
	*/
	[Obsolete()]
	proto external string Region();
	/*
	\brief Returns name of server
	*/
	proto external string Name();
	/*
	\brief Returns name of scenario on server
	*/
	proto external string ScenarioName();
	/*
	\brief Addon that contains the hosted scenario
	*/
	proto external Dependency HostScenarioMod();
	/*
	\brief Returns IP address of server
	*/
	proto external string HostAddress();
	/*
	\brief Specifies type of server. Check backend documentation for specific values.
	*/
	proto external string HostType();
	/*
	\brief BI-account name of the server owner (if known)
	*/
	[Obsolete()]
	proto external string OwnerName();
	/*
	\brief Returns Gamemode name on the server
	*/
	[Obsolete()]
	proto external string GameMode();
	/*
	\brief Issue Join request to the server. On Successful result can initiate transition to the server.
	*/
	proto external void Join(BackendCallback callback, JsonApiStruct params);
	/*
	\brief Returns maximum number of players allowed on the server.
	*/
	proto external int PlayerLimit();
	/*
	\brief Returns current count of players on the server.
	*/
	proto external int PlayerCount();
	/*
	\returns maximum size of the join queue.
	*/
	proto external int GetQueueMaxSize();
	/*
	\returns current size (how many slots are used) of the join queue.
	*/
	proto external int GetQueueSize();
	/*
	\brief Get current position of user in join queue
	\returns int value of current position or -1 if user is not in queue of this room
	*/
	proto external int GetQueueUserPosition();
	/*
	\brief	Get average time (in seconds) to wait before user is moved by one position in this queue.
	\note	Value calculation is based on how long in average it took previous people in queue to join the server.
			It is just estimation and might not represent real waiting time.
	\returns int value of seconds or -1 if there wasn't enough data to compute this value
	*/
	proto external int GetQueueAvgWaitTime();
	/*
	\brief Get time (in seconds) of how long you are in queue since joining it.
	\returns int value of seconds or -1 if you are not in the queue
	*/
	proto external int GetQueueJoinTime();
	/*
	\brief Get time (in seconds) of how long ago you last moved in queue.
	\returns int value of seconds or -1 if you did not moved yet
	*/
	proto external int GetQueueLastMoveTime();
	/*
	\brief Set BackendCallback which will be used in periodical polls in queue
	*/
	proto external void SetQueueBackendCallback(BackendCallback callback);
	/*
	\brief Leave join queue
	*/
	proto external void LeaveJoinQueue();
	/*
	\brief Get the list of addons that are present on the server.
	\note Available just if IsAuthorized() returns true
	*/
	proto external void GetItems(out notnull array<Dependency> items);
	/*
	\brief Returns true of server has mods
	*/
	proto external bool IsModded();
	/*
	\brief Sets favorite state for the server
	*/
	proto external void SetFavorite(bool favorite, BackendCallback callback);
	/*
	\brief Returns true if server is flagged as favorite
	*/
	proto external bool IsFavorite();
	/*
	\brief Are client's mods the same as in this room?
	*/
	proto external bool IsClientReady();
	/*
	\brief Returns true if server is flagged as crossplatform
	*/
	proto external bool IsCrossPlatform();
	/*
	\brief Returns MissionWorkshopItem of currently hosted scenario on the server
	*/
	proto external MissionWorkshopItem HostScenario();
	/*
	\brief Get the last measured ping value.
	\note The ping is measured between client and a ping site that is assigned to the server.
	*/
	proto external float GetPing();
	/*
	\brief Returns true if server is protected by BattlEye
	*/
	proto external bool HasBattlEye();
	/*!
	\brief Was the password verified and may the client join the server or view its content.
	*/
	proto external bool IsAuthorized();
	/*
	\brief Verify if input password is valid and if it is then authorize client
	*/
	proto external void VerifyPassword(string password, BackendCallback callback);
	/*
	\brief Check if client is authorized with previous server - used in case of reconnect without need of reentering the password
	*/
	proto external void CheckAuthorization(BackendCallback callback);
}

/*!
\}
*/
