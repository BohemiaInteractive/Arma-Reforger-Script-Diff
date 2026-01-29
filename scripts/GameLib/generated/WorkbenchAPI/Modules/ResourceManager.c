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

sealed class ResourceManager: WBModuleDef
{
	proto external MetaFile GetMetaFile(string absFilePath);
	/*!
	Register register resource (create meta file) and optionally also build runtime resource.
	Usage:
	\code
		// register resource
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		rm.RegisterResourceFile("c:\\DATA\\UI\\Imagesets\\Test\\Test.tga", true);

	\endcode
	*/
	proto external bool RegisterResourceFile(string absFilePath, bool bBuildRuntimeResource);
	/*!
	Rebuild already registered resource.
	Usage:
	\code
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto external void RebuildResourceFile(string filePath, string configuration, bool selectFiles);
	/*!
	Rebuild already registered resources.
	Usage:
	\code
		rm.RebuildResourceFiles({"UI\\Imagesets\\Test\\Test.tga"}, "PC");
	\endcode
	*/
	proto external void RebuildResourceFiles(notnull array<string> filePaths, string configuration);
	/*!
	Waits for file to be acknowledged by Resource Database.
	Returns true if the file is ready, otherwise returns false due to time out.
	*/
	proto external bool WaitForFile(string filePath, int maxTimeMs = 1000);
	//! Return exact paths of selected items from ResourceBrowser panel in ResourceManager.
	proto external void GetResourceBrowserSelection(WorkbenchSearchResourcesCallback callback, bool recursive = false);
}

/*!
\}
*/

#endif // WORKBENCH
