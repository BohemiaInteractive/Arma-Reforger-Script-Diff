/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

//! Server Catalogue value for platform support matching.
sealed enum EServerCataloguePlatformMatch
{
	//! Will list only servers that support just the client's platform.
	ONLY_MY_PLATFORM,
	//! Will list only servers that support multiple platforms including the client's.
	CROSSPLATFORM,
	//! Will list all servers that at least support platform of the client.
	ANY_COMPATIBLE,
	//! Will list all servers that are not compatible with client platform.
	ONLY_INCOMPATIBLE,
	//! Will list all servers regardless platform compatibility.
	ANY,
}

/*!
\}
*/
