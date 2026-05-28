/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

//! Different modes of ServerCatalogueApi which determines how will be catalogue filled with servers.
sealed enum EServerCatalogueMode
{
	//! Server list provided by Backend
	INTERNET,
	//! Server list discovered via broadcast on LAN
	DISCOVERY_BROADCAST,
	//! Server discovered via direct IP and appended into list
	DISCOVERY_DIRECT,
	//! Combined list of servers from Broadcast and Direct Discovery
	DISCOVERY_COMBINED,
}

/*!
\}
*/
