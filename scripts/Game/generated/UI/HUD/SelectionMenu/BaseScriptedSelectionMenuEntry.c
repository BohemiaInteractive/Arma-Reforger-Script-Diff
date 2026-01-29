/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD_SelectionMenu
\{
*/

class BaseScriptedSelectionMenuEntry: BaseSelectionMenuEntry
{
	//! Can this entry be shown?
	event bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu);
	//! Can this entry be performed?
	event bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu);
	//! Callback for when this entry is supposed to be performed
	event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu);
	//! If overridden and true is returned, outName is returned when BaseSelectionMenuEntry.GetEntryName is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	event bool GetEntryNameScript(out string outName);
	//! If overridden and true is returned, outDescription is returned when BaseSelectionMenuEntry.GetEntryDescription is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	event bool GetEntryDescriptionScript(out string outDescription);
	//! If overridden and true is returned, outIconPath is returned when BaseSelectionMenuEntry.GetEntryIconPath is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	event bool GetEntryIconPathScript(out string outIconPath);
	//! Can be overriden to return desired UIInfo in GetUIInfo method
	event UIInfo GetUIInfoScript();
}

/*!
\}
*/
