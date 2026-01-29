/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD_SelectionMenu
\{
*/

class BaseSelectionMenuEntry: ScriptAndConfig
{
	//! Can this entry be shown?
	proto external bool CanBeShown(IEntity user, BaseSelectionMenu sourceMenu);
	//! Can this entry be performed?
	proto external bool CanBePerformed(IEntity user, BaseSelectionMenu sourceMenu);
	//! Request to perform this entry
	proto external void Perform(IEntity user, BaseSelectionMenu sourceMenu);
	//! Return the name of this entry.
	//! By default returns the value in UIInfo or empty string if UIInfo doesn't exist.
	proto external string GetEntryName();
	//! Return the description of this entry.
	//! By default returns the value in UIInfo or empty string if UIInfo doesn't exist
	proto external string GetEntryDescription();
	//! Return the icon path of this entry.
	//! By default returns the value in UIInfo or empty string if UIInfo doesn't exist.
	proto external string GetEntryIconPath();
	//! Get related UI info or null if none.
	proto external UIInfo GetUIInfo();
}

/*!
\}
*/
