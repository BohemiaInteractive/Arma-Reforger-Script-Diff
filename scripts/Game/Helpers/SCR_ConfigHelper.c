class SCR_ConfigHelper
{
	//------------------------------------------------------------------------------------------------
	//! Get BaseContainer from resource, ensuring the whole thing is valid
	//! \param[in] resource if invalid, returns null
	//! \param[in] subChildPath path is case-insensitive, separated by /, spaces can be involved \
	//! examples: "level1/level2/level3", "level1 / level2   /   level3" \
	//! if empty, returns the provided container
	//! \param[in] removeEntryPart if true, removes the last path element \
	//! examples: path/to/entryName → path/to (to target the parent container itself)
	//! \return found child BaseContainer, or null if base container or path is invalid
	static BaseContainer GetBaseContainerByPath(notnull Resource resource, string subChildPath = "", bool removeEntryPart = false)
	{
		if (!resource.IsValid())
			return null;

		if (subChildPath.IsEmpty())
			return resource.GetResource().ToBaseContainer();

		return GetChildBaseContainerByPath(resource.GetResource().ToBaseContainer(), subChildPath, removeEntryPart);
	}

	//------------------------------------------------------------------------------------------------
	//! returns the found sub-container or null if not found
	//! \param[in] container if null, returns null
	//! \param[in] subChildPath path is case-insensitive, separated by /, spaces can be involved \
	//! examples: "level1/level2/level3", "level1 / level2   /   level3" \
	//! if empty, returns the provided container
	//! \param[in] removeEntryPart if true, removes the last path element \
	//! examples: path/to/entryName → path/to (to target the parent container itself)
	//! \return found child BaseContainer, or null if path is incorrect
	protected static BaseContainer GetChildBaseContainerByPath(BaseContainer container, string subChildPath, bool removeEntryPart = false)
	{
		if (!container)
			return null;

		array<string> paths = {};
		SplitConfigPath(subChildPath, paths, removeEntryPart);
		if (paths.IsEmpty())
			return container;

		return GetChildBaseContainerByPath(container, paths);
	}

	//------------------------------------------------------------------------------------------------
	//! returns the found sub-container or null if not found
	//! \param[in] container if null, returns null
	//! \param[in] paths case-insensitive, items will be trimmed \
	//! if empty, returns the provided container
	//! \return found child BaseContainer, or null if path is incorrect
	// to do: split in smaller methods?
	protected static BaseContainer GetChildBaseContainerByPath(notnull BaseContainer container, array<string> paths)
	{
		if (!container)
			return null;

		if (!paths || paths.IsEmpty())
			return container;

		string path;
		string nextPath;
		BaseContainer prevChild = container;
		BaseContainer child = container;
		BaseContainerList containerList;
		for (int i, cntMinus1 = paths.Count() - 1; i <= cntMinus1; i++)
		{
			path = paths[i];
			if (i < cntMinus1)
				nextPath = paths[i + 1];
			else
				nextPath = string.Empty;

			prevChild = child;
			child = prevChild.GetObject(path);
			if (child)
				continue; // child is found, we need to go deeper

			if (nextPath.IsEmpty())
				return null;

			containerList = prevChild.GetObjectArray(path);
			if (!containerList)
				return null; // no child, no array of children, get out

			child = GetChildFromList(containerList, nextPath);
			if (!child)
				return null;

			i++; // moved one step ahead with list → child
		}

		return child;
	}

	//------------------------------------------------------------------------------------------------
	//! Get BaseContainer from BaseContainerList by its (case-insensitive) name
	//! \param[in] containerList list of containers from which to look
	//! \param[in] childName case-insensitive child name
	//! \return BaseContainer found child or null
	protected static BaseContainer GetChildFromList(notnull BaseContainerList containerList, string childName)
	{
		if (childName.IsEmpty())
			return null;

		string arrayItemName;
		BaseContainer containerListElement;
		for (int i, cnt = containerList.Count(); i < cnt; i++)
		{
			containerListElement = containerList[i];
			arrayItemName = containerListElement.GetName();

			if (arrayItemName.IsEmpty())
				arrayItemName = containerListElement.GetClassName(); // if .et

			if (arrayItemName.IsEmpty())
				continue;

			arrayItemName.ToLower();
			childName.ToLower();
			if (arrayItemName == childName)
				return containerListElement;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! split "config path" (separator: / ), trims and toLower parts and returns last part if any
	//! \param[in] input the input string
	//! \param[out] output the target output array
	//! \param[in] removeLastPart removes last part if entry path is provided
	//! \return last part (trimmed but not ToLower'ed), whether it has been removed or not
	static string SplitConfigPath(string input, out array<string> output, bool removeLastPart = false)
	{
		input.Split("/", output, true);

		string lastPart;
		if (output.IsEmpty())
			return lastPart;

		int lastIndex = output.Count() -1;
		lastPart = output[lastIndex].Trim();
		if (removeLastPart)
			output.Remove(lastIndex);

		for (int i, cnt = output.Count(); i < cnt; i++)
		{
			output[i] = output[i].Trim();
		}

		return lastPart;
	}

	//------------------------------------------------------------------------------------------------
	//! Extract GUID from full resource name
	//! \param[in] resourceName full resource path
	//! \param[in] removeBrackets if true, remove brackets
	//! \return resourceName's GUID
	static string GetGUID(ResourceName resourceName, bool removeBrackets = false)
	{
		int guidIndex = resourceName.LastIndexOf("}");
		if (guidIndex < 0)
			return string.Empty;

		if (removeBrackets)
			return resourceName.Substring(1, guidIndex - 1);
		else
			return resourceName.Substring(0, guidIndex + 1);
	}
}

class SCR_ConfigHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! Get typed container instance (or null)
	//! See also ConfType.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(configPath));
	//! \param[in] configPath
	//! \return T-casted BaseContainer instance or null if not found/wrong type
	static T GetConfigObject(ResourceName configPath)
	{
		if (configPath.IsEmpty())
			return null;

		Resource resource = BaseContainerTools.LoadContainer(configPath);
		if (!resource || !resource.IsValid())
			return null;

		BaseContainer container = resource.GetResource().ToBaseContainer();
		if (!container)
			return null;

		return T.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
	}
}
