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
This is server side API for reading current states of the server in relation to its registration
into backend services for listing servers on clients.

\warning it is only usable on server side and calling methods on clients who are connected to the
         server will not provide valid data or no data at all.
         If game wants to show some of these data on clients then it must replicate them on its own.
*/
sealed class ServerLobbyApi
{
	private void ServerLobbyApi();
	private void ~ServerLobbyApi();

	/*!
	Returns true if server lobby is enabled and server should register into backend server list.
	\note this returns valid value only on server side.
	*/
	static proto bool IsEnabled();
	/*!
	Returns current instance of the ServerInfo of the server.
	It represents partial data which are provided to clients in ServerCatalogueApi
	when browsing the list.
	The ServerInfo is constructed from data filled in the ServerConfig.
	\note this returns valid value only on server side and on client will always return null.
	      To access data on client side the game must implement its own way of replicating them.
	\warning some values are filled in by the backend side systems and server does not know about them
	         so when compared with instance received from ServerCatalogueApi it might be missing some data.
	*/
	static proto ServerInfo GetServerInfo();
	/*!
	Returns current instance of the ServerConfig of the server.
	To access game specific data it must be cast into appropriate game specific class for config.
	\note this returns valid value only on server side.
	      To access data on client side the game must implement its own way of replicating them.
	\warning changing values via setters might not always change the actual configuration of the server
	         because some parameters are read and take effect only once at the start of the server
	         while others might be replicated to clients on connection (replication implemented by game specific code).
	*/
	static proto ServerConfig GetServerConfig();
	/*!
	Returns current instance of the ServerGameDataBase of the server which must be cast into
	game specific class which adds the getters for individual data.
	\note this returns valid value only on server side.
	      To access data on client side the game must implement its own way of replicating them.
	      This returns same value as ServerLobbyApi.GetServerInfo().GetGameData()
	*/
	static proto ServerGameDataBase GetServerGameData();
}

/*!
\}
*/
