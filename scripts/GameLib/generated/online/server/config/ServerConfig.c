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
	Default enabled state of BattlEye (Anti-Cheat) protection.
	ServerConfig::DEFAULT_BATTLEYE = `true`
	*/
	static const bool DEFAULT_BATTLEYE;
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
	ServerConfig::MAX_MAX_PLAYERS = `128`
	*/
	static const int MAX_MAX_PLAYERS;
	/*!
	Default port for connecting to the server replication.
	ServerConfig::DEFAULT_MAX_PLAYERS = `64`
	*/
	static const int DEFAULT_PORT;
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
	//! Returns current value for enabled sate of the BattlEye.
	proto external bool GetBattlEye();
	//! Changes the enabled sate of the BattlEye.
	proto external void SetBattlEye(bool value);
}

/*!
\}
*/
