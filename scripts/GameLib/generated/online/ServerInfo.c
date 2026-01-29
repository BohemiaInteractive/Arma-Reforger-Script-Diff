/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
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
	/*!
	Returns count of how many mods is server using.
	\note Can be used in ServerBrowser to determine if server is modded or not
	      before requesting more detailed data about the server.
	*/
	proto external int GetModCount();
	/*!
	Will request join to the server which will assign slot for player
	and provide authentication tokens for encrypted communication with the server.
	This method will also set both JoiningServer and LastServer to this in ServerCatalogueApi.

	\param onDetails is optional function which will be invoked when full server details
	                 are received and can be used to make final validations before join.
	                 Delete of BackendCallback provided to the request will stop join process.

	\throw VME - If server is not joinable - IsJoinable() returns false.
	           - If server does not support client platform - IsMyPlatformSupported() returns false.
	           - If server IsPasswordProtected() but password is missing - SetPassword() not used.
	           - If another request for this server is currently processed (Join or Details requests).

	\note API Codes to handle:
	      - OnError:
	          - EApiCode::EACODE_ERROR_SERVER_NOT_FOUND
	          - EApiCode::EACODE_ERROR_SERVER_PASSWORD_MISMATCH
	          - EApiCode::EACODE_ERROR_SERVER_IS_FULL

	\code
		ref BackendCallback myCallback;
		ServerInfo serverToJoin;

		// simple join with no checks
		void Join()
		{
			serverToJoin.RequestJoin(myCallback); // if callback invokes OnSuccess begin GameTransition to the server.
		}

		// join with additional validations
		void JoinWithValidation()
		{
			serverToJoin.RequestJoin(myCallback, ValidationMethod); // if callback invokes OnSuccess begin GameTransition to the server.
		}

		// do some desired validation like there is blocked player on the server
		void ValidationMethod()
		{

			//...

			if (blockedPlayerConnected)
			{
				myCallback = null; // stop joining by deleting the callback

				// schedule some UI dialog or any means to inform that join was interrupted
			}

			// finishing without delete of callback will continue join
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
}

/*!
\}
*/
