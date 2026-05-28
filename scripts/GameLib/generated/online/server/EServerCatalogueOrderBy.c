/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

//! Server Catalogue ordering values.
sealed enum EServerCatalogueOrderBy
{
	//! ordered randomly.
	NONE,
	//! ordered by player counts.
	PLAYER_COUNT,
	//! ordered by their names.
	SERVER_NAME,
	SERVER_NAME_ASCII,
	//! ordering based on how long ago user last joined - works only with favorites and oldest join filters
	LAST_JOINED,
	PING_SITE_AVG_RTT,
}

/*!
\}
*/
