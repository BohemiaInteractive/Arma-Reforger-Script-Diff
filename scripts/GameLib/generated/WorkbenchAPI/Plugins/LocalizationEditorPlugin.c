/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_Plugins
\{
*/

class LocalizationEditorPlugin: WorkbenchPlugin
{
	event void OnSave(BaseContainer stringTable, string stringTableItemClassName, string stringTableAbsPath);
	event void OnChange(BaseContainer stringTableItem, string propName, string propValue);
	event void OnImport(BaseContainer newItem, BaseContainer oldItem);
	event void OnExport(BaseContainer item);
	event void OnSelectionChanged();
	event bool IsReadOnly(BaseContainer item, bool isImporting);
	//! Called for each item during building runtime table, expected column name to export for given language.
	event string GetExportColumn(BaseContainer item, string languageCode);
}

/*!
\}
*/

#endif // WORKBENCH
