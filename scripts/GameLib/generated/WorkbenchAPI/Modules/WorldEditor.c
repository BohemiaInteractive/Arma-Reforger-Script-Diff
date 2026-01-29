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

sealed class WorldEditor: WBModuleDef
{
	proto external WorldEditorAPI GetApi();
	proto external bool GetTerrainBounds(out vector min, out vector max);
	proto external void SwitchToGameMode(bool debugMode = false, bool fullScreen = false);
	proto external void SwitchToEditMode();
	//! Return `true` if editor is in prefab edit mode (dedicated mode for editing prefabs).
	proto external bool IsPrefabEditMode();
	//! Return exact paths of selected items from ResourceBrowser panel in WorldEditor.
	proto external void GetResourceBrowserSelection(WorkbenchSearchResourcesCallback callback, bool recursive = false);
}

/*!
\}
*/

#endif // WORKBENCH
