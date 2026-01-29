/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_Modules
\{
*/

sealed class LocalizationEditor: WBModuleDef
{
	//! Begins group of undo actions (for user will whole group behave like one action).
	proto external void BeginModify(string text);
	//! Modifies single StringTableItem property. Must be placed between BeginModify/EndModify calls.
	proto external void ModifyProperty(BaseContainer container, int variable, string value);
	/*!
	Insert single StringTableItem property. Must be placed between BeginModify/EndModify calls.
	\param id	ID of new item
	\param bNotify Create WB notification about creation or failed creation
	\param bSelect Select newly created item after creation
	\return created StringTableItem container or null when unsuccessful
	*/
	proto external BaseContainer InsertItem(string id, bool bNotify, bool bSelect);
	//! Delete single StringTableItem item. Must be placed between BeginModify/EndModify calls.
	proto external void DeleteItem(string id);
	//! Ends group of undo actions.
	proto external void EndModify();
	//! Refreshes UI.
	proto external void RefreshUI();
	//! Returns string table container.
	proto external BaseContainer GetTable();
	//! Returns indexes of rows which are filtered at the moment.
	proto external void GetFilteredRows(notnull out array<int> rowsIdx);
	//! Returns indexes of rows which are selected at the moment.
	proto external void GetSelectedRows(notnull out array<int> rowsIdx);
	//! Filters just rows given in `rowsIdx` array.
	proto external void AddUserFilter(notnull array<int> rowsIdx, string caption);
}

/*!
\}
*/

#endif // WORKBENCH
