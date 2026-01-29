/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Player
\{
*/

class SocialComponentClass: GameComponentClass
{
}

/*!
Component responsible for handling social interactions b/w 2 players.
Gathers restrictions from different sources like platform user permissions/privileges.
This component is meant to be on the Player Controller.
*/
class SocialComponent: GameComponent
{
	ref ScriptInvoker<int> m_OnBlockedPlayerJoinedInvoker = new ScriptInvoker<int>();
	ref ScriptInvoker<int, bool> m_OnReportPlayerFinishInvoker = new ScriptInvoker<int, bool>();

	/*!
	Is local user privileged to participate in given interaction?
	*/
	static proto bool IsPrivilegedTo(EUserInteraction interaction);
	/*!
	Requests the local user privilege associated with given interaction
	\param interaction Requested interaction.
	\param callback Callback object used to obtain result.
	\return Returns true on successful request placement.
	*/
	static proto bool RequestSocialPrivilege(EUserInteraction interaction, PrivilegeCallback cb);
	/*!
	Is local user allowed to participate in multiplayer?
	*/
	static proto bool IsMultiplayerAllowed();
	/*!
	Requests the local user privilege to participate in multiplayer games
	\param callback Callback object used to obtain result.
	\return Returns true on successful request placement.
	*/
	static proto bool RequestMultiplayerPrivilege(PrivilegeCallback cb);
	/*!
	Is the given interaction b/w this and otherPlayer allowed? Returns false
	in case of invalid arguments.
	*/
	proto external bool IsRestricted(int otherPlayerID, EUserInteraction interaction);
	/*!
	Determines if otherPlayer is blocked by this player. All player interactions
	are restricted if one player is blocked. Returns false in case of invalid argument.
	*/
	proto external bool IsBlocked(int otherPlayerID);
	/*!
	Determines if the player can be unblocked from within game - it is on game block list.
	\return true if player can be unblocked, or false when player is not blocked throught a game block list.
	*/
	proto external bool CanUnblock(int otherPlayerID);
	/*!
	Determines if otherPlayer is muted by this player for the duration of
	the game session. Only VoiceChat UserInteraction is affected. Returns
	false in case of invalid argument.
	*/
	proto external bool IsMuted(int otherPlayerID);
	/*!
	Mutes/unmutes otherPlayer for the duration of the game session. Only
	VoiceChat interaction is affected.
	*/
	proto external void SetMuted(int otherPlayerID, bool mute);
	/*!
	Reports inappropriate player behaviour into BackendApi
	*/
	proto external void ReportPlayer(int reportedPlayerID, SCR_EReportReason reason);

	// callbacks

	//! Event invoked when player present on a platform or game blocklist joins the server
	event protected void OnBlockedPlayerJoined(int joinedPlayerID) { m_OnBlockedPlayerJoinedInvoker.Invoke(joinedPlayerID); };
	//! Event invoked as a result of ReportPlayer
	event protected void OnReportPlayerFinish(int reportedPlayerID, bool success) { m_OnReportPlayerFinishInvoker.Invoke(reportedPlayerID, success); };
}

/*!
\}
*/
