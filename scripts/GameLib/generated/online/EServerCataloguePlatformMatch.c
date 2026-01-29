/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Server Catalogue value for platform support matching.
enum EServerCataloguePlatformMatch
{
	//! Will list only servers which support only platform of the client.
	SCPM_ONLY_MY_PLATFORM,
	//! Will list only servers which support multiple platforms including one of the client.
	SCPM_CROSSPLATFORM,
	//! Will list all servers that at least support platform of the client.
	SCPM_ANY_COMPATIBLE,
	//! Will list all servers that are not compatible with client platform.
	SCPM_ONLY_INCOMPATIBLE,
	//! Will list all servers regardless platform compatibility.
	SCPM_ANY,
}

/*!
\}
*/
