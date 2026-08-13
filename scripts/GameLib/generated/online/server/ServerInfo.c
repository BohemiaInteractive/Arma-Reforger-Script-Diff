/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

/*!
Object containing info and interaction methods for game servers.
They are initialized, managed and cleared by native systems and script
should only interact with those provided by ServerCatalogueApi.


*/
sealed class ServerInfo
{
	private void ServerInfo();
	private void ~ServerInfo();

	//! Returns unique ID of this server.
	proto external string GetId();
	//! Returns name of this server.
	proto external string GetName();
	/*!
	Returns description of this server.
	\note Details needs to be fetched at least once via RequestDetails() otherwise this will not provide any data.
	*/
	proto external string GetDescription();
	//! Returns type of the server which specifies how it was hosted.
	proto external EServerType GetServerType();
	/*!
	Returns count of max players that can be connected to the server.
	\note This should not be used to block user from attempting join the server
	      because he might have reserved slot or space in some join queue.
	*/
	proto external int GetMaxPlayers();
	/*!
	Returns count of currently connected players on the server.
	\note This should not be used to block user from attempting join the server
	      because he might have reserved slot or space in some join queue.
	*/
	proto external int GetPlayerCount();
	//! Returns true if server is indicated as currently online (running)
	proto external bool IsOnline();
	/*!
	Will request change for favorite mark state for this server.
	\throw VME - if set to state which is already set is attempted IsFavorite() == isFavorite
	*/
	proto external void SetFavorite(notnull BackendCallback callback, bool isFavorite);
	//! Returns true if client has this server marked as favorite.
	proto external bool IsFavorite();
	//! Returns how long ago user last joined this server in seconds.
	proto external int GetSecondsSinceLastJoin();
	/*!
	Returns true if server is configured to be visible in ServerCatalogue.
	\note All servers provided by ServerCatalogueApi should theoretically return as true.
	      Can be useful on server-side ServerLobbyApi when changing live configuration of the server
	      to show to admins current visibility in ServerCatalogueApi.
	*/
	proto external bool IsVisible();
	/*!
	Returns true if server is in state in which clients can join it.
	\note If server is joinable is determined by the backend.
	      Server might not be joinable because it stopped
	      communicating with the backend due to crash or other issues,
	*/
	proto external bool IsJoinable();
	//! Returns true if platform of the client is supported by the server.
	proto external bool IsMyPlatformSupported();
	//! Returns true if server supports my and at least one more platform.
	proto external bool IsCrossPlatform();
	/*!
	Returns true if server requires password for further requests to
	more detailed info about the server or to request join to the server.
	*/
	proto external bool IsPasswordProtected();
	/*!
	Sets password for this server.
	When used for server obtained by ServerCatalogue it will serve for client authentication.
	*/
	proto external void SetPassword(string password);
	/*!
	Returns currently set password for joining the server.
	\note Servers from ServerCatalogueApi will always have this return "" by default.
	      Can be used to get currently configured password on server side via ServerLobbyApi.
	*/
	proto external string GetPassword();
	[Obsolete("Use GetWorkshopData().GetAssetCount() instead. This will always returns 0.")]
	proto external int GetModCount();
	/*!
	If server is running with some workshop assets then this will provide related data.
	\note If this returns null then server is Vanilla.
	*/
	proto external ServerWorkshopData GetWorkshopData();
	/*!
	Returns true if this server has any ping site assigned.
	If it returns false then that means it will never have any ping site available and might not
	even be joinable from the internet. It might be joinable only from the local network.
	\note This information is available regardless of whether the client has fetched the list
	      of available ping sites. Use this to check if it makes sense to call GetPingSite().
	\warning It should not block the ability to connect to the server but can be used for warning purposes.
	*/
	proto external bool HasPingSite();
	/*!
	Returns the ping site assigned to this server, or nullptr if:
	- The available ping sites have not been fetched yet.
	- The server does not have an assigned ping site.
	- The server reports a ping site that is not in the client's fetched list.
	\note Use HasPingSite() to determine if the server has an assigned ping site
	      before attempting to retrieve it.
	*/
	proto external ServerPingSite GetPingSite();
	//! Returns count of how many queues are known for this server.
	proto external int GetQueueInfoCount();
	//! Returns specific queue by type for this server.
	proto external ServerQueueInfo GetQueueInfoByType(EServerJoinQueueType type);
	//! Returns all available queues for this server.
	proto external void GetAllQueueInfo(out notnull array<ServerQueueInfo> queuesOut);
	/*!
	Requests to join this server. The server will either approve the join immediately
	or place the user in a queue if the server is full.

	Before calling, ensure the server IsJoinable(), supports the client platform
	(IsMyPlatformSupported()), and has the password set via SetPassword() if the server
	IsPasswordProtected().

	\param callback Receives the result of the join request.
	\param onDetails Optional function invoked when full server details are received,
	                 before the join completes. Use this to perform final validations
	                 (e.g. checking connected players). To cancel the join from inside
	                 this function, set the callback to null.

	\throw VME If the server is not joinable, does not support the client platform,
	           requires a password that was not set, or another request for this server
	           is already in progress.

	API codes delivered to the callback:
	 - OnSuccess:
	   - EApiCode::EACODE_SERVER_JOIN — join approved, request a game state transition
	     to connect to the server.
	   - EApiCode::EACODE_SERVER_QUEUE_WAIT — server is full, the user has been placed
	     in a queue. Use ServerJoinManagerApi.GetQueueData() to access queue info
	     and set up a polling callback for status updates.
	 - OnError:
	   - EApiCode::EACODE_ERROR_SERVER_NOT_FOUND — server no longer exists.
	   - EApiCode::EACODE_ERROR_SERVER_PASSWORD_MISMATCH — incorrect password.
	   - EApiCode::EACODE_ERROR_SERVER_IS_FULL — server is full and has no queue.

	\see ServerJoinManagerApi
	\see ServerQueueData

	\code
		ref BackendCallback joinCallback;
		ServerInfo serverToJoin;

		// Simple join
		void Join()
		{
			serverToJoin.RequestJoin(joinCallback);
		}

		// Join with pre-join validation
		void JoinWithValidation()
		{
			serverToJoin.RequestJoin(joinCallback, OnDetailsReceived);
		}

		// Called when server details are received, before join completes
		void OnDetailsReceived()
		{
			// perform any checks, e.g. blocked players

			if (shouldCancel)
			{
				joinCallback = null; // cancels the join
			}

			// returning without nulling the callback continues the join
		}

		// Handle callback result
		void OnJoinResult(BackendCallback callback)
		{
			EApiCode result = callback.GetApiCode();

			if (result == EApiCode.EACODE_SERVER_JOIN)
			{
				// approved — request game state transition to connect
			}
			else if (result == EApiCode.EACODE_SERVER_QUEUE_WAIT)
			{
				// placed in queue — set up polling callback for updates
				ServerQueueData queueData = ServerJoinManagerApi.GetQueueData();
				queueData.SetPollingCallback(joinCallback);
			}
		}

	\endcode
	*/
	proto external void RequestJoin(notnull BackendCallback callback, func onDetails = null);
	/*!
	Will request full details about the server which are not automatically provided when browsing in the catalogue.
	It is not necessary to perform before joining the server since it will always be performed
	automatically to ensure that client can actually join or to show possible warnings before join.

	\warning This request should be sent only on specific user actions like opening some details dialog.
	         Passworded servers also require to set valid password before providing details.

	\note API Codes to handle:
				- OnError:
						- EApiCode::EACODE_ERROR_SERVER_NOT_FOUND
						- EApiCode::EACODE_ERROR_SERVER_PASSWORD_MISMATCH
	*/
	proto external void RequestDetails(notnull BackendCallback callback);
	/*!
	Returns instance of game specific data for this server.
	It must be casted into game specific class for game data to access the values.

	\note If this returns null then at the moment the server did not provide any data
	      or the game did not setup any game data in which case this would be always
	      null.
	*/
	proto external ServerGameDataBase GetGameData();
}

/*!
\}
*/
