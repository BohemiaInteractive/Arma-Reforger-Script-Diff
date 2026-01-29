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

class WorldEditorPlugin: WorkbenchPlugin
{
	event void OnGameModeStarted(string worldName, string gameMode, bool playFromCameraPos, vector cameraPosition, vector cameraAngles);
	event void OnGameModeEnded();
	/*!
	Called when user dropped some kind of data into a world edit window. Plugins can completely reimplement default editor funtionality using WorldEditorAPI
	\param windowType Type of a window where data were dropped. Values represent Perpective, Top, Right, Back views
	\param posX Horizontal window position
	\param posY Vertical window position
	\param dataType Type of dropped data. At the moment we support "WorldEditor/EntityType" and "Workbench/ResourceFiles" values
	\param data Depends on dataType. If dataType is "WorldEditor/EntityType" then data[0] contains a className. If dataType is "Workbench/ResourceFiles" then it contains one or more registered resource names
	*/
	event bool OnWorldEditWindowDataDropped(int windowType, int posX, int posY, string dataType, array<string> data);
}

/*!
\}
*/

#endif // WORKBENCH
