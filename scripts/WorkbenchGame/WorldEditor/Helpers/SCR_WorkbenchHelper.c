class SCR_WorkbenchHelper
{
	protected static ref array<string> s_aTempStrings;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] message
	//! \param[in] caption
	static void PrintDialog(string message, string caption = "", LogLevel level = LogLevel.WARNING)
	{
		Print("" + message, level);
		Workbench.Dialog(caption, message);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] message
	//! \param[in] param1
	//! \param[in] param2
	//! \param[in] param3
	//! \param[in] caption
	static void PrintFormatDialog(string message, string param1, string param2 = "", string param3 = "", string caption = "", LogLevel level = LogLevel.WARNING)
	{
		message = string.Format(message, param1, param2, param3);
		Print("" + message, level: level);
		Workbench.Dialog(caption, message);
	}

	//------------------------------------------------------------------------------------------------
	//! Search Workbench-available files by extension and filters inside a provided directory
	//! Does NOT support unregistered files such as script files - use SearchWorkbenchFiles if needed
	//! \param[in] fileExtensions
	//! \param[in] searchStrArray
	//! \param[in] rootPath format $addon:Workbench/Directory
	//! \param[in] recursive
	//! \return found resources
	static array<ResourceName> SearchWorkbenchResources(array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true)
	{
		if (fileExtensions && fileExtensions.Contains("c"))
		{
			Print("SCR_WorkbenchHelper.SearchWorkbenchResources() cannot search for script files as they are not registered, use SearchWorkbenchFiles instead", LogLevel.WARNING);
			fileExtensions.RemoveItemOrdered("c");
		}

		if (fileExtensions && fileExtensions.Contains("layer"))
		{
			Print("SCR_WorkbenchHelper.SearchWorkbenchResources() cannot search for layer files as they are not registered, use SearchWorkbenchFiles instead", LogLevel.WARNING);
			fileExtensions.RemoveItemOrdered("layer");
		}

		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = fileExtensions;
		filter.recursive = recursive;
		filter.rootPath = rootPath;
		filter.searchStr = searchStrArray;

		set<ResourceName> tempResult = new set<ResourceName>();
		ResourceDatabase.SearchResources(filter, tempResult.Insert);

		// remove unregistered entries
		tempResult.RemoveItem(ResourceName.Empty);

		// convert to array
		array<ResourceName> result = {};
		result.Reserve(tempResult.Count());
		foreach (ResourceName resourceName : tempResult)
		{
			result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Search Workbench-available files by extension and filters inside a provided directory
	//! Support unregistered files such as script files - use SearchWorkbenchResources if ResourceNames are needed
	//! \param[in] fileExtensions
	//! \param[in] searchStrArray
	//! \param[in] rootPath format $addon:Workbench/Directory
	//! \param[in] recursive
	//! \return found file's relative paths (e.g $Arma4:Scripts/Game/MyFile.c)
	static array<string> SearchWorkbenchFiles(array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true)
	{
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = fileExtensions;
		filter.recursive = recursive;
		filter.rootPath = rootPath;
		filter.searchStr = searchStrArray;

		s_aTempStrings = {};
		ResourceDatabase.SearchResources(filter, SearchResourcesCallbackMethod);

		array<string> result = {};
		result.Copy(s_aTempStrings);
		s_aTempStrings = null;

		return result;
	}

	//
	// Callbacks
	//

	//------------------------------------------------------------------------------------------------
	//! SearchResourcesCallback method used by SearchWorkbenchFiles to get file (relative) PATHS (not ResourceNames), e.g $Arma4:Scripts/Game/MyFile.c
	protected static bool SearchResourcesCallbackMethod(ResourceName resourceName, string exactPath = "")
	{
		s_aTempStrings.Insert(exactPath);
		return true;
	}
}
