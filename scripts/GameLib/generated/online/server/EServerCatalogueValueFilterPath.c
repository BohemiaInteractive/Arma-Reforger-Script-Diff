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
Paths for value filters in server catalogue.
\see ServerCatalogueFilters for the full path reference table with types and descriptions.
*/
enum EServerCatalogueValueFilterPath
{
	//! STRING -- text search in server name.
	SEARCH_TEXT,
	//! BOOL -- joinable servers.
	JOINABLE,
	//! BOOL -- locked/unlocked servers.
	LOCKED,
	//! BOOL -- official/community servers.
	OFFICIAL,
	//! ENUM -- version match. Use SetVersionMatch().
	VERSION_MATCH,
	//! ENUM -- platform compatibility. Use SetPlatformMatch().
	PLATFORM_MATCH,
	//! INT -- average round-trip time to ping site (milliseconds).
	PING_SITE_AVG_RTT,
	//! INT -- player fill percentage (0-100).
	PLAYER_PERCENT,
	//! INT -- absolute player count.
	PLAYER_COUNT,
	//! INT -- number of workshop assets (mods). 0 = vanilla.
	WORKSHOP_ASSET_COUNT,
	//! BOOL -- include offline servers in results.
	INCLUDE_OFFLINE,
	//! BOOL -- favorite/non-favorite servers.
	FAVORITES,
	//! INT -- upper bound (in seconds) on how long ago the user last joined the server. Backend enforces EQUAL operator.
	OLDEST_JOIN_IN_SECONDS,
}

/*!
\}
*/
