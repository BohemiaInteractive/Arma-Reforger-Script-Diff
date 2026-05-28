/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

//! Data needed to register a new Group
sealed class GroupManifest: Managed
{
	proto external void SetCreatedAtPublic(bool bCreatedAtPublic);
	proto external void SetName(string sName);
	proto external void SetClanTag(string sClanTag);
	proto external void SetDescription(string sDescription);
	proto external void SetGroupEnlistmentStatus(EGroupEnlistmentStatus eEnlistmentStatus);
	proto external void SetPrimaryLanguage(string sPrimaryLanguage);
	proto external void SetContactInfo(string sContactInfo);
	proto external void SetRules(string sRules);
	proto external void SetGameplayApproaches(EGroupGameplayApproach eApproaches);
	proto external void SetGameplayFocuses(EGroupGameplayFocus eFocuses);
	proto external void SetGameRegions(EGroupGameRegion eRegions);
}

/*!
\}
*/
