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
	proto external bool GetCreatedAtPublic();
	proto external string GetName();
	proto external string GetClanTag();
	proto external string GetDescription();
	proto external EGroupEnlistmentStatus GetGroupEnlistmentStatus();
	proto external string GetPrimaryLanguage();
	proto external string GetContactInfo();
	proto external string GetRules();
	proto external EGroupGameplayApproach GetGameplayApproaches();
	proto external EGroupGameplayFocus GetGameplayFocuses();
	proto external EGroupGameRegion GetGameRegions();
}

/*!
\}
*/
