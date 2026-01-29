/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Network
\{
*/

//! Statistical Api - Analytics
class GameStatsApi
{
	//! Sends a player event to treasure data (evt_player) - script invokable
	proto external void CreatePlayerEvent(int iPlayerID, Managed params, string sEventName, bool isSP);
	//! Sends a session event to treasure data (evt_session) - script invokable
	proto external void CreateSessionEvent(Managed params, string sEventName);
	//! Sends an editor event to treasure data (evt_gm) - script invokable
	proto external void CreateEditorEvent(Managed params, string sEventName);
	//! Sends a mod event to treasure data (evt_mod) - script invokable
	proto external void CreateModEvent(Managed params, string sEventName);
	//! Sends an application event to treasure data (evt_app) - script invokable
	proto external void CreateApplicationEvent(Managed params, string sEventName);
	//! Generic Player Event - script invokable
	proto external void PlayerEvent(int iPlayerID, Managed params);
	//! Player Score - script invokable
	proto external void PlayerScore(int iPlayerID, Managed params);
	//! Player Rank Gained - script invokable
	proto external void PlayerRankGained(int iPlayerID, Managed params);
	//! Session Base Captured - script invokable
	proto external void SessionBaseCaptured(Managed params);
	//! Session Base Attacked - script invokable
	proto external void SessionBaseAttacked(Managed params);
	//! Session Measures - script invokable
	proto external void SessionMeasures(Managed params);
	//! Editor Start - script invokable
	proto external void EditorStart();
	//! Editor End - script invokable
	proto external void EditorClosed();
	//! Generic Mod related Event - script invokable
	proto external void ModEvent(Managed params);
}

/*!
\}
*/
