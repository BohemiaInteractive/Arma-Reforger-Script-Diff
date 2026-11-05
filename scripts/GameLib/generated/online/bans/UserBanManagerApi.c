/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

/*!
Provides access to ban information for the currently authenticated player.
Call RequestUserBans() once at session start to populate the local cache, then
use IsLocalUserBanned() for synchronous checks without further network calls.
*/
sealed class UserBanManagerApi
{
	private void UserBanManagerApi();
	private void ~UserBanManagerApi();

	/*!
	Load the list of bans that apply to the current player and store them locally.
	Call this once at the start of a session. Once complete, use IsLocalUserBanned()
	and GetUserBans() to check ban status without additional network calls.
	*/
	static proto void RequestUserBans(notnull BackendCallback pCallback);
	/*!
	Check whether the current player has an active ban for the given system and action.
	Returns true if a matching ban was found. Requires a prior call to RequestUserBans().
	*/
	static proto bool IsLocalUserBanned(EBanScopeSystem eSystem, EBanScopeAction eAction);
	//! Fills the output array with all bans loaded by the last RequestUserBans() call.
	static proto void GetUserBans(out array<BanEntry> bansOut);
}

/*!
\}
*/
