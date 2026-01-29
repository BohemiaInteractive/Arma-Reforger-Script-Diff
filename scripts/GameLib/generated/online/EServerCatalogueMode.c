/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Different modes of ServerCatalogueApi which determines how will be catalogue filled with servers.
enum EServerCatalogueMode
{
	//! Server list provided by Backend
	SCM_INTERNET,
	//! Server list discovered via broadcast on LAN
	SCM_DISCOVERY_BROADCAST,
	//! Server discovered via direct IP and appended into list
	SCM_DISCOVERY_DIRECT,
	//! Combined list of servers from Broadcast and Direct Discovery
	SCM_DISCOVERY_COMBINED,
}

/*!
\}
*/
