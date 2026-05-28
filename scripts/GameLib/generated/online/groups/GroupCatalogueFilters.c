/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

class GroupCatalogueFilters: Managed
{
	//! Sets minimum number of members filter
	proto external void SetMembersMin(int iMembersMin);
	//! Returns current minimum number of members filter
	proto external int GetMembersMin();
	//! Sets maximum number of members filter
	proto external void SetMembersMax(int iMembersMax);
	//! Returns current maximum number of members filter
	proto external int GetMembersMax();
	//! Sets primary language filter
	proto external void SetPrimaryLanguage(string sLanguage);
	//! Returns current primary language filter
	proto external string GetPrimaryLanguage();
	proto external void SetClanTag(string sTag);
	//! Sets IsMember filter
	proto external void SetIsMember(EBackendCatalogueBoolFlag eIsMember);
	/*!
	Returns current IsMember filter
	\note Only Groups which the user is member of are listed
	*/
	proto external EBackendCatalogueBoolFlag GetIsMember();
	/*!
	Returns current Clan Tag filter
	\note Only Groups which clan tag starts with set string are listed
	*/
	proto external string GetClanTag();
	//! Sets name filter
	proto external void SetName(string sName);
	/*!
	Returns current name filter
	\note Only Groups which name starts with set string are listed
	*/
	proto external string GetName();
	/*!
	Sets gameplay approaches filter
	\param iApproaches is bit mask
	*/
	proto external void SetGameplayApproaches(int iApproaches);
	/*!
	Returns current gameplay approaches filter
	\note Only groups with given gameplay approaches are listed
	*/
	proto external int GetGameplayApproaches();
	/*!
	Sets gameplay focuses filter
	\param iFocuses is a bitmask
	*/
	proto external void SetGameplayFocuses(int iFocuses);
	/*!
	Returns current gameplay focuses filter
	\note Only groups with given gameplay focuses are listed
	*/
	proto external int GetGameplayFocuses();
	/*!
	Sets game regions filter
	\param iRegions is bitmask
	*/
	proto external void SetGameRegions(int iRegions);
	/*!
	Returns current game regions filter
	\note Only groups with given game regions are listed
	*/
	proto external int GetGameRegions();
}

/*!
\}
*/
