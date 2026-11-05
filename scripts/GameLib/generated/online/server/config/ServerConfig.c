/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server_config
\{
*/

sealed class ServerConfig
{
	/** \name Constants
	 * Populated by the engine.
	 */
	///@{

	/*!
	Default visibility state of the server in server browser.
	ServerConfig::DEFAULT_VISIBLE = `true`
	*/
	static const bool DEFAULT_VISIBLE;
	/*!
	Default platform compatibility configuration.
	ServerConfig::DEFAULT_PLATFORMS = platform of the application
	*/
	static const EServerPlatformFlags DEFAULT_PLATFORMS;
	/*!
	Default max player connection capacity.
	ServerConfig::DEFAULT_MAX_PLAYERS = `64`
	*/
	static const int DEFAULT_MAX_PLAYERS;
	/*!
	Minimum limit of value for max player connection capacity.
	ServerConfig::MIN_MAX_PLAYERS = `1`
	*/
	static const int MIN_MAX_PLAYERS;
	/*!
	Maximum limit of value for max player connection capacity.
	ServerConfig::MAX_MAX_PLAYERS = `256`
	*/
	static const int MAX_MAX_PLAYERS;
	/*!
	Default port for connecting to the server replication.
	ServerConfig::DEFAULT_MAX_PLAYERS = `64`
	*/
	static const int DEFAULT_PORT;
	/*!
	Minimum length limit for server name.
	ServerConfig::MIN_NAME_LENGTH = `1`
	*/
	static const int MIN_NAME_LENGTH;
	/*!
	Maximum  length limit for server name.
	ServerConfig::MAX_NAME_LENGTH = `100`
	*/
	static const int MAX_NAME_LENGTH;
	///@}

	private void ServerConfig();
	void ~ServerConfig();

	//! Returns currently set name in the config.
	proto external string GetName();
	//! Changes name set in the config.
	proto external void SetName(string value);
	//! Returns currently set description in the config.
	proto external string GetDescription();
	//! Changes description set in the config.
	proto external void SetDescription(string value);
	//! Returns currently set password in the config.
	proto external string GetPassword();
	//! Changes password set in the config.
	proto external void SetPassword(string value);
	//! Returns currently set limit for max players in the config.
	proto external int GetMaxPlayers();
	//! Changes the limit for max players set in the config.
	proto external void SetMaxPlayers(int value);
	//! Returns currently set limit for max players in the config.
	proto external int GetPort();
	//! Changes the limit for max players set in the config.
	proto external void SetPort(int value);
	//! Returns currently allowed platforms in the server config.
	proto external EServerPlatformFlags GetPlatforms();
	//! Changes allowed platforms in the server config.
	proto external void SetPlatforms(EServerPlatformFlags value);
	//! Returns current value for the visibility in server browser.
	proto external bool GetVisible();
	//! Changes the visibility in server browser.
	proto external void SetVisible(bool value);
	/*!
	Sets the socket address the server will bind to.
	Pass an empty address to bind to all interfaces (0.0.0.0 - INADDR_ANY).
	Pass -1 as port to use the server's main port.
	\return `true` on success. Triggers a VM error on invalid address or out-of-range port.
	*/
	proto external bool SetSocket(string address, int port);
	//! Writes the currently configured socket address and port into the out parameters.
	proto void GetSocket(out string address, out int port);
	/*!
	Appends a connection endpoint to the list.
	Pass -1 as port to use the server's main port.
	\return Index of the appended endpoint on success, or -1 on invalid input.
	Triggers a VM error on invalid input.
	*/
	proto external int AddConnectionEndpoint(string address, int port);
	//! Returns the number of connection endpoints currently configured.
	proto external int GetConnectionEndpointCount();
	/*!
	Writes the address and port of the endpoint at index into the out parameters.
	Triggers a VM error if index is out of range.
	*/
	proto void GetConnectionEndpoint(int index, out string address, out int port);
	/*!
	Removes the endpoint at index, preserving the order of remaining endpoints.
	Triggers a VM error if index is out of range.
	*/
	proto external void RemoveConnectionEndpoint(int index);
	//! Removes all connection endpoints from the list.
	proto external void ClearConnectionEndpoints();
}

/*!
\}
*/
