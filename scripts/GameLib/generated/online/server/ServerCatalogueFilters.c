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
Object used to specify filters when listing internet servers in the ServerCatalogueApi.

Supported value filter paths (EServerCatalogueValueFilterPath):

| Path                   | Type   | Allowed operators | Description                                               |
|------------------------|--------|-------------------|-----------------------------------------------------------|
| SEARCH_TEXT            | STRING | EQUAL             | Text search in server name.                               |
| JOINABLE               | BOOL   | EQUAL             | Filter joinable/unjoinable servers.                       |
| LOCKED                 | BOOL   | EQUAL             | Filter locked (password-protected) / unlocked servers.    |
| OFFICIAL               | BOOL   | EQUAL             | Filter official/community servers.                        |
| VERSION_MATCH          | ENUM   | EQUAL             | Filter by game version compatibility.                     |
| PLATFORM_MATCH         | ENUM   | EQUAL             | Filter by platform compatibility.                         |
| PING_SITE_AVG_RTT      | INT    | all comparisons   | Average round-trip time in milliseconds. Min 0.           |
| PLAYER_PERCENT         | INT    | all comparisons   | Player fill percentage (0-100).                           |
| PLAYER_COUNT           | INT    | all comparisons   | Absolute player count. Min 0.                             |
| WORKSHOP_ASSET_COUNT   | INT    | all comparisons   | Number of mods. 0 = vanilla, >0 = modded. Min 0.          |
| INCLUDE_OFFLINE        | BOOL   | EQUAL             | Include offline servers in results.                       |
| FAVORITES              | BOOL   | EQUAL             | Filter favorite/non-favorite servers.                     |
| OLDEST_JOIN_IN_SECONDS | INT    | EQUAL             | Servers user joined within the last N seconds.            |

\note BOOL paths always use EQUAL. SetBoolValueFilter takes no operator argument.
\note INCLUDE_OFFLINE is accepted by the backend only when combined with FAVORITES
or OLDEST_JOIN_IN_SECONDS. Using it alone returns empty results.
\note OLDEST_JOIN_IN_SECONDS: the backend interprets the value as an upper bound
(servers joined within the last N seconds) and enforces the EQUAL operator.

\remark The convenience methods below (SetFavorite, SetLocked, SetFull, SetOfficial,
SetModded, SetVersionMatch, SetPlatformMatch, SetSearchText, SetPlayerPercentage,
SetOldestJoinInSeconds, SetIncludeOffline) exist for backward compatibility with
pre-existing script code. New code should prefer the value filter API with
EServerCatalogueValueFilterPath constants -- it expresses intent more clearly,
covers paths that have no wrapper (PING_SITE_AVG_RTT, PLAYER_COUNT), and allows
combining multiple operators on the same path (numeric ranges).

## Usage examples

### Public browser query: unlocked, joinable, community, version-matched, not full

\code
ServerCatalogueFilters filters = new ServerCatalogueFilters();
filters.SetBoolValueFilter(EServerCatalogueValueFilterPath.JOINABLE, true);
filters.SetBoolValueFilter(EServerCatalogueValueFilterPath.LOCKED,   false);
filters.SetBoolValueFilter(EServerCatalogueValueFilterPath.OFFICIAL, false);
filters.SetEnumStaticValueFilter(EServerCatalogueValueFilterPath.VERSION_MATCH,
	EBackendValueFilterOperator.EQUAL, EServerCatalogueVersionMatch.ONLY_MATCHING);
filters.SetIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.LESS_THAN, 100);
\endcode

### Numeric range on a single path: player fill between 25% and 75%

Using AddIntValueFilter twice keeps both bounds; SetIntValueFilter would replace the first with the second.

\code
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, 25);
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.LESS_THAN_OR_EQUAL, 75);
\endcode

### Low-latency cutoff: only servers under 80 ms to their closest ping site

\code
filters.SetIntValueFilter(EServerCatalogueValueFilterPath.PING_SITE_AVG_RTT,
	EBackendValueFilterOperator.LESS_THAN, 80);
\endcode

### Recently-joined favorites, including offline ones

INCLUDE_OFFLINE is accepted by the backend only when combined with FAVORITES or OLDEST_JOIN_IN_SECONDS.

\code
filters.SetBoolValueFilter(EServerCatalogueValueFilterPath.FAVORITES, true);
filters.SetIntValueFilter(EServerCatalogueValueFilterPath.OLDEST_JOIN_IN_SECONDS,
	EBackendValueFilterOperator.EQUAL, 7 * 24 * 60 * 60); // last 7 days
filters.SetBoolValueFilter(EServerCatalogueValueFilterPath.INCLUDE_OFFLINE, true);
\endcode

### Lightly modded servers: between 1 and 5 workshop assets

\code
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT,
	EBackendValueFilterOperator.GREATER_THAN, 0);
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT,
	EBackendValueFilterOperator.LESS_THAN_OR_EQUAL, 5);
\endcode

### Updating an existing filter: Set and Add both overwrite matching entries

Set and Add never duplicate -- they replace. Calling either again for the same path
(Set) or same path+operator (Add) simply updates the stored value. A typical case is
a UI slider whose minimum-player-percent value changes as the user drags it:

\code
// User picks 25% as the minimum.
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, 25);

// User drags the slider to 50%. No need to remove the previous filter first;
// same path + same operator, so this overwrites the 25 with 50.
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, 50);
\endcode

Set clears *all* operators on the path before inserting, so mixing Set with a
previously added bound wipes the other bound. Use Add when multiple operators on
the same path must coexist; use Set when the path holds exactly one filter:

\code
// Range: 25% <= playerPercent <= 75%
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, 25);
filters.AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.LESS_THAN_OR_EQUAL, 75);

// Later: this single Set REPLACES every PLAYER_PERCENT filter -- the upper bound
// from above is gone, the lower bound is now 50.
filters.SetIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT,
	EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, 50);
\endcode

### Clearing a single path (e.g. user cleared the search box)

\code
filters.RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.SEARCH_TEXT);
\endcode
*/
class ServerCatalogueFilters: BackendCatalogueFilter
{
	void ServerCatalogueFilters();

	//! Sets text search filter for server names. Empty string removes the filter.
	void SetSearchText(string searchText)
	{
		if (searchText.IsEmpty())
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.SEARCH_TEXT);
		else
			SetStringValueFilter(EServerCatalogueValueFilterPath.SEARCH_TEXT, EBackendValueFilterOperator.EQUAL, searchText);
	}

	//! Returns current search text filter. Returns empty string if not set.
	string GetSearchText()
	{
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.SEARCH_TEXT);
		if (filters.IsEmpty())
			return string.Empty;
		return filters[0].GetString();
	}

	//! Filter servers that user recently joined in last X seconds. Value of 0 removes the filter.
	void SetOldestJoinInSeconds(int seconds)
	{
		if (seconds <= 0)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.OLDEST_JOIN_IN_SECONDS);
		else
			SetIntValueFilter(EServerCatalogueValueFilterPath.OLDEST_JOIN_IN_SECONDS, EBackendValueFilterOperator.EQUAL, seconds);
	}

	/*!
	Returns current oldest join filter in seconds. Returns 0 if not set.
	\note The setter rejects non-positive values, so 0 unambiguously means
	"not set" -- no valid filter value collides with the sentinel.
	*/
	int GetOldestJoinInSeconds()
	{
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.OLDEST_JOIN_IN_SECONDS);
		if (filters.IsEmpty())
			return 0;
		return filters[0].GetInt();
	}

	/*!
	Include offline servers in results. Setting false removes the filter.
	\warning The backend accepts this filter only when combined with FAVORITES
	or OLDEST_JOIN_IN_SECONDS. Using it alone returns empty results.
	*/
	void SetIncludeOffline(bool val)
	{
		if (!val)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.INCLUDE_OFFLINE);
		else
			SetBoolValueFilter(EServerCatalogueValueFilterPath.INCLUDE_OFFLINE, true);
	}

	//! Returns true if offline servers are included. Returns false if not set.
	bool GetIncludeOffline()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.INCLUDE_OFFLINE))
			return false;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.INCLUDE_OFFLINE);
		return filters[0].GetBool();
	}

	//! Filter any/favorite/not favorite servers.
	void SetFavorite(EBackendCatalogueBoolFlag val)
	{
		if (val == EBackendCatalogueBoolFlag.BCB_ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.FAVORITES);
		else
			SetBoolValueFilter(EServerCatalogueValueFilterPath.FAVORITES, val == EBackendCatalogueBoolFlag.BCB_TRUE);
	}

	//! Returns current favorites filter - any/favorite/not favorite.
	EBackendCatalogueBoolFlag GetFavorite()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.FAVORITES))
			return EBackendCatalogueBoolFlag.BCB_ANY;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.FAVORITES);
		if (filters[0].GetBool())
			return EBackendCatalogueBoolFlag.BCB_TRUE;
		return EBackendCatalogueBoolFlag.BCB_FALSE;
	}
	//! Filter any/locked/unlocked servers.
	void SetLocked(EBackendCatalogueBoolFlag val)
	{
		if (val == EBackendCatalogueBoolFlag.BCB_ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.LOCKED);
		else
			SetBoolValueFilter(EServerCatalogueValueFilterPath.LOCKED, val == EBackendCatalogueBoolFlag.BCB_TRUE);
	}

	//! Returns current lock filter - any/locked/unlocked.
	EBackendCatalogueBoolFlag GetLocked()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.LOCKED))
			return EBackendCatalogueBoolFlag.BCB_ANY;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.LOCKED);
		if (filters[0].GetBool())
			return EBackendCatalogueBoolFlag.BCB_TRUE;
		return EBackendCatalogueBoolFlag.BCB_FALSE;
	}

	/*!
	Filter any/full/not full servers.
	\note "Full" is implemented as PLAYER_PERCENT >= 100; because the path
	is capped at 100 this is effectively PLAYER_PERCENT == 100.
	*/
	void SetFull(EBackendCatalogueBoolFlag val)
	{
		if (val == EBackendCatalogueBoolFlag.BCB_ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.PLAYER_PERCENT);
		else if (val == EBackendCatalogueBoolFlag.BCB_TRUE)
			SetIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, 100);
		else
			SetIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.LESS_THAN, 100);
	}

	//! Returns current full filter - any/full/not full.
	EBackendCatalogueBoolFlag GetFull()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT))
			return EBackendCatalogueBoolFlag.BCB_ANY;
		BackendValueFilter filter = GetValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL);
		if (filter && filter.GetInt() >= 100)
			return EBackendCatalogueBoolFlag.BCB_TRUE;
		return EBackendCatalogueBoolFlag.BCB_FALSE;
	}

	/*!
	Filter by player fill percentage range (0-100).
	\note Passing the full range (0, 100) removes the filter.
	*/
	void SetPlayerPercentage(int minPercent = 0, int maxPercent = 100)
	{
		RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.PLAYER_PERCENT);
		if (minPercent > 0)
			AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL, minPercent);
		if (maxPercent < 100)
			AddIntValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.LESS_THAN_OR_EQUAL, maxPercent);
	}

	//! Returns currently set minimum player percentage. Returns 0 if not set.
	int GetPlayerMinPercentage()
	{
		BackendValueFilter filter = GetValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.GREATER_THAN_OR_EQUAL);
		if (!filter)
			return 0;
		return filter.GetInt();
	}

	//! Returns currently set maximum player percentage. Returns 100 if not set.
	int GetMaxPlayerPercentage()
	{
		BackendValueFilter filter = GetValueFilter(EServerCatalogueValueFilterPath.PLAYER_PERCENT, EBackendValueFilterOperator.LESS_THAN_OR_EQUAL);
		if (!filter)
			return 100;
		return filter.GetInt();
	}

	//! Filter any/official/community servers.
	void SetOfficial(EBackendCatalogueBoolFlag val)
	{
		if (val == EBackendCatalogueBoolFlag.BCB_ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.OFFICIAL);
		else
			SetBoolValueFilter(EServerCatalogueValueFilterPath.OFFICIAL, val == EBackendCatalogueBoolFlag.BCB_TRUE);
	}

	//! Returns current official filter - any/official/community.
	EBackendCatalogueBoolFlag GetOfficial()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.OFFICIAL))
			return EBackendCatalogueBoolFlag.BCB_ANY;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.OFFICIAL);
		if (filters[0].GetBool())
			return EBackendCatalogueBoolFlag.BCB_TRUE;
		return EBackendCatalogueBoolFlag.BCB_FALSE;
	}

	//! Filter any/modded/vanilla servers.
	void SetModded(EBackendCatalogueBoolFlag val)
	{
		if (val == EBackendCatalogueBoolFlag.BCB_ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT);
		else if (val == EBackendCatalogueBoolFlag.BCB_TRUE)
			SetIntValueFilter(EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT, EBackendValueFilterOperator.GREATER_THAN, 0);
		else
			SetIntValueFilter(EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT, EBackendValueFilterOperator.EQUAL, 0);
	}

	//! Returns current modded filter - any/modded/vanilla.
	EBackendCatalogueBoolFlag GetModded()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT))
			return EBackendCatalogueBoolFlag.BCB_ANY;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.WORKSHOP_ASSET_COUNT);
		if (filters[0].GetOperator() == EBackendValueFilterOperator.GREATER_THAN)
			return EBackendCatalogueBoolFlag.BCB_TRUE;
		return EBackendCatalogueBoolFlag.BCB_FALSE;
	}

	/*!
	Returns current version matching filter. Returns ANY if not set.
	\note The int-to-enum cast at the end is safe: the matching setter only
	writes valid EServerCatalogueVersionMatch values into this path.
	*/
	EServerCatalogueVersionMatch GetVersionMatch()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.VERSION_MATCH))
			return EServerCatalogueVersionMatch.ANY;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.VERSION_MATCH);
		return filters[0].GetInt();
	}

	/*!
	Returns current platform compatibility matching filter. Returns ANY if not set.
	\note The int-to-enum cast at the end is safe: the matching setter only
	writes valid EServerCataloguePlatformMatch values into this path.
	*/
	EServerCataloguePlatformMatch GetPlatformMatch()
	{
		if (!HasValueFilter(EServerCatalogueValueFilterPath.PLATFORM_MATCH))
			return EServerCataloguePlatformMatch.ANY;
		array<BackendValueFilter> filters = {};
		GetValueFiltersByPath(filters, EServerCatalogueValueFilterPath.PLATFORM_MATCH);
		return filters[0].GetInt();
	}

	//! Filter servers by version compatibility with client. ANY removes the filter (backend decides default).
	void SetVersionMatch(EServerCatalogueVersionMatch val)
	{
		if (val == EServerCatalogueVersionMatch.ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.VERSION_MATCH);
		else
			SetEnumStaticValueFilter(EServerCatalogueValueFilterPath.VERSION_MATCH, EBackendValueFilterOperator.EQUAL, val);
	}

	//! Filter servers by platform compatibility with client. ANY removes the filter (backend decides default).
	void SetPlatformMatch(EServerCataloguePlatformMatch val)
	{
		if (val == EServerCataloguePlatformMatch.ANY)
			RemoveValueFiltersByPath(EServerCatalogueValueFilterPath.PLATFORM_MATCH);
		else
			SetEnumStaticValueFilter(EServerCatalogueValueFilterPath.PLATFORM_MATCH, EBackendValueFilterOperator.EQUAL, val);
	}


}

/*!
\}
*/
