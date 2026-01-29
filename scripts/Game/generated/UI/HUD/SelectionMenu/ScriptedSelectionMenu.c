/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD_SelectionMenu
\{
*/

class ScriptedSelectionMenu: BaseSelectionMenu
{
	//! Callback when open is requested
	event protected void OnOpen(IEntity owner);
	//! Callback when close is requested
	event protected void OnClose(IEntity owner);
	//! Callback when menu update is requested
	//! Note that it is up to user to call Update to receive this event
	event protected void OnUpdate(IEntity owner, float timeSlice);
}

/*!
\}
*/
