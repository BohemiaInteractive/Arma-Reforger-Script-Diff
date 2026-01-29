/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Object used to specify filters when listing internet servers in the ServerCatalogueApi
class ServerCatalogueFilters: Managed
{
	//! Sets flags for text search filter
	proto external void SetSearchFlags(EServerCatalogueSearchFlags flags);
	//! Returns currently set search flags
	proto external EServerCatalogueSearchFlags GetSearchFlags();
	/*!
	Text search filter in fields specified by search flags.
	\note Defaults to "" (any).
	\throw VME if no search flags are set.
	*/
	proto external void SetSearchText(string searchText);
	//! Returns current search filter in server names.
	proto external string GetSearchText();
	/*!
	Filter any/favorite/not favorite servers.
	\note Defaults to any.
	*/
	proto external void SetFavorite(EBackendCatalogueBoolFlag val);
	//! Returns current lock filter - any/favorite/not favorite.
	proto external EBackendCatalogueBoolFlag GetFavorite();
	/*!
	Filter servers that user recently joined in last X seconds.
	\note Defaults to any = 0.
	*/
	proto external void SetOldestJoinInSeconds(int seconds);
	//! Returns current oldest join filter.
	proto external int GetOldestJoinInSeconds();
	/*!
	Will include servers that are currently offline if set to true.
	This option works only when filtering only favorite or recently joined servers.
	\note Defaults to false.
	*/
	proto external void SetIncludeOffline(bool val);
	// Returns currently set value for including offline servers.
	proto external bool GetIncludeOffline();
	/*!
	Filter any/locked/unlocked servers.
	\note Defaults to any.
	*/
	proto external void SetLocked(EBackendCatalogueBoolFlag val);
	//! Returns current lock filter - any/locked/unlocked.
	proto external EBackendCatalogueBoolFlag GetLocked();
	/*!
	Filter any/full/not full servers.
	\note Defaults to any.
	*/
	proto external void SetFull(EBackendCatalogueBoolFlag val);
	//! Returns current full filter - any/full/not full.
	proto external EBackendCatalogueBoolFlag GetFull();
	/*!
	Filter server based on percentage range 0% - 100% of how many players are on the server.
	\note Defaults to 0% - 100%.
	\throw VME if values are not in valid percentage range or if minPercent is less than maxPercent.
	*/
	proto external void SetPlayerPercentage(int minPercent = 0, int maxPercent = 100);
	//! Returns currently set min player percentage
	proto external int GetPlayerMinPercentage();
	//! Returns currently set max player percentage
	proto external int GetMaxPlayerPercentage();
	/*!
	Filter any/official/community servers.
	\note Defaults to any.
	*/
	proto external void SetOfficial(EBackendCatalogueBoolFlag val);
	//! Returns current server type filter - any/official/community.
	proto external EBackendCatalogueBoolFlag GetOfficial();
	/*!
	Filter any/modded/vanilla servers.
	\note Defaults to any.
	*/
	proto external void SetModded(EBackendCatalogueBoolFlag val);
	//! Returns current modded filter - any/modded/vanilla.
	proto external EBackendCatalogueBoolFlag GetModded();
	/*!
	Filter server based on their version matching with the client.
	\note Defaults to any.
	*/
	proto external void SetVersionMatch(EServerCatalogueVersionMatch val);
	//! Returns current version matching filter.
	proto external EServerCatalogueVersionMatch GetVersionMatch();
	/*!
	Filter server based on their platform compatibility matching with the client.
	\note Defaults to any compatible platform.
	*/
	proto external void SetPlatformMatch(EServerCataloguePlatformMatch val);
	//! Returns current platform compatibility matching filter.
	proto external EServerCataloguePlatformMatch GetPlatformMatch();
}

/*!
\}
*/
