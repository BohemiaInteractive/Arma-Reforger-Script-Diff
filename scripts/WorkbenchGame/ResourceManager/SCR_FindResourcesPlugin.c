#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Find Linked Resources", description: "Find resources referenced by the resources selected in Resource Browser", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF0C1)]
class SCR_FindResourcesPlugin : ResourceManagerPlugin
{
	[Attribute(defvalue: "", desc: "File extensions separated by a comma (e.g.: \"edds,imageset\")\nNo value means all extensions")]
	protected string m_sExtensions;

	[Attribute(defvalue: "0", desc: "Show only values set directly in the resource (and not in its parents or default value)")]
	protected bool m_bOnlyShowDirectlySetVariables;

	protected static const ref array<string> ALLOWED_EXTENSIONS = { "conf", "et" };
//	protected static const ref array<string> FORBIDDEN_DOTTED_EXTENSIONS = { ".c", ".edds", ".fbx", ".png", ".txt", ".wav", ".xob" };

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Find Linked Resources", "Scan all selected files and print out resources of given extension(s) linked to from attributes.", this))
			return;

		m_sExtensions.ToLower();
		array<string> extensions = {};
		m_sExtensions.Split(",", extensions, true);

		for (int i = extensions.Count() - 1; i >= 0; --i)
		{
			string extension = extensions[i];
			extension.TrimInPlace();
			if (extension)
				extensions[i] = extension;
			else
				extensions.Remove(i);
		}

		m_sExtensions = SCR_StringHelper.Join(",", extensions, false);

		bool noExtensionsDefined = extensions.IsEmpty();

		array<ResourceName> selection = {};
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.GetResourceBrowserSelection(selection.Insert, true);

		int count = selection.Count();
		if (count < 1)
		{
			Print("No resources selected in Resource Browser", LogLevel.NORMAL);
			return;
		}

		array<ref Resource> resourceObjects = {}; // reference MUST be kept
		Resource resource;
		array<BaseContainer> containers = {};
		float prevProgress, currProgress;
		WBProgressDialog progress = new WBProgressDialog("Loading " + count + " resources...", resourceManager);
		foreach (int i, ResourceName resourceName : selection)
		{
			string extension;
			FilePath.StripExtension(resourceName.GetPath(), extension);
			extension.ToLower();
			if (!ALLOWED_EXTENSIONS.Contains(extension))
				continue;

			resource = Resource.Load(resourceName);
			if (!resource.IsValid())
			{
				Print("Cannot load resource " + resourceName.GetPath(), LogLevel.WARNING);
				continue;
			}

			resourceObjects.Insert(resource);
			containers.Insert(resource.GetResource().ToBaseContainer());

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}

		array<string> resourcePaths = {};
		BaseContainer container, object;
		BaseContainerList list;

		prevProgress = 0;
		currProgress = 0;
		count = containers.Count();
		if (count < 1)
		{
			Print("No containers found.", LogLevel.NORMAL);
			return;
		}

		array<ResourceName> resourceNames;
		progress = new WBProgressDialog("Processing " + count + " main containers...", resourceManager);
		while (!containers.IsEmpty())
		{
			container = containers[0];
			containers.RemoveOrdered(0);

			if (!container)
			{
				Print("null container?!", LogLevel.WARNING);
				continue;
			}

			// add children already
			for (int i = container.GetNumChildren() - 1; i >= 0; --i)
			{
				containers.Insert(container.GetChild(i));
			}

			for (int i, varCount = container.GetNumVars(); i < varCount; ++i)
			{
				string varName = container.GetVarName(i);
				if (m_bOnlyShowDirectlySetVariables && !container.IsVariableSetDirectly(varName))
					continue;

				switch (container.GetDataVarType(i))
				{
					case DataVarType.RESOURCE_NAME:
//					case DataVarType.STRING:
						ResourceName resourceName;
						if (!container.Get(varName, resourceName))
						{
							Print("Cannot read " + varName + " (resourceName)", LogLevel.WARNING);
							break;
						}

						if (!resourceName) // .IsEmpty()
						{
							// Print(varName + " is empty", LogLevel.NORMAL);
							break;
						}

						if (!noExtensionsDefined)
						{
							string varExtension;
							FilePath.StripExtension(resourceName, varExtension);
							varExtension.ToLower();
						
							if (!extensions.Contains(varExtension))
								break;
						}

						string path = resourceName.GetPath();
						if (!path) // .IsEmpty()
						{
							PrintFormat("%1 has no path (%2)", varName, resourceName, level: LogLevel.WARNING);
							break;
						}

						if (!resourcePaths.Contains(path))
							resourcePaths.Insert(path);

						break;

					case DataVarType.RESOURCE_NAME_ARRAY:
						if (!container.Get(varName, resourceNames))
						{
							Print("Cannot read " + varName + " (resourceName array)", LogLevel.WARNING);
							break;
						}

						foreach (ResourceName resourceName : resourceNames)
						{
							if (!noExtensionsDefined)
							{
								string varExtension;
								FilePath.StripExtension(resourceName, varExtension);
								varExtension.ToLower();
							
								if (!extensions.Contains(varExtension))
									continue;
							}

							string path = resourceName.GetPath();
							if (!path) // .IsEmpty()
							{
								PrintFormat("%1 has no path (%2)", varName, resourceName, level: LogLevel.WARNING);
								break;
							}

							if (!resourcePaths.Contains(resourceName))
								resourcePaths.Insert(resourceName);
						}

						break;

					case DataVarType.OBJECT:
						object = container.GetObject(varName);
						if (!object)
						{
							Print("Cannot read " + varName + " (object)", LogLevel.WARNING);
							break;
						}

						containers.Insert(object);
						break;

					case DataVarType.OBJECT_ARRAY:
						list = container.GetObjectArray(varName);
						if (!list)
						{
							Print("Cannot read " + varName + " (object array)", LogLevel.WARNING);
							break;
						}

						for (int l = 0, listCount = list.Count(); l < listCount; l++)
						{
							containers.Insert(list.Get(l));
						}

						break;
				}
			}

			currProgress = (count - containers.Count()) / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}

		int resourcesCount = resourcePaths.Count();
		if (resourcesCount < 1)
		{
			Print("No resources found.", LogLevel.NORMAL);
			return;
		}

		if (noExtensionsDefined)
			PrintFormat("Linked resources of all types (%1):", resourcesCount, level: LogLevel.NORMAL);
		else
			PrintFormat("Linked resources of type(s) %1 (%2):", SCR_StringHelper.Join(", ", extensions, false), resourcesCount, level: LogLevel.NORMAL);

		resourcePaths.Sort();
		foreach (string resourcePath : resourcePaths)
		{
			PrintFormat("@\"%1\"", resourcePath, level: LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run")]
	protected bool OK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool Cancel()
	{
		return false;
	}
}
#endif // WORKBENCH
