/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD_SelectionMenu
\{
*/

class BaseSelectionMenu: ScriptAndConfig
{
	//! Request an update to this radial menu, ideally called during EOnFrame
	proto external void Update(IEntity owner, float timeSlice);
	//! Request to open this radial menu
	proto external void Open(IEntity owner);
	//! Request to close this radial menu
	proto external void Close(IEntity owner);
	//! Is this radial menu currently open?
	proto external bool IsOpen();
	//! Clear the list of entries stored in this menu
	proto external void ClearEntries();
	//! Add provided entry to the list of entries (if not contained already)
	proto external void AddEntry(BaseSelectionMenuEntry entry);
	//! Remove provided entry from the list of entries
	proto external void RemoveEntry(BaseSelectionMenuEntry entry);
	//! Returns true if this menu already contains provided entry
	proto external bool ContainsEntry(BaseSelectionMenuEntry entry);
	//! Returns the number of entries stored in this menu
	proto external int GetEntriesCount();
	//! Fills the provided array with weak references to entries from this menu
	//! Returns the number of output elements.
	proto external int GetEntryList(out notnull array<BaseSelectionMenuEntry> outActions);
}

/*!
\}
*/
