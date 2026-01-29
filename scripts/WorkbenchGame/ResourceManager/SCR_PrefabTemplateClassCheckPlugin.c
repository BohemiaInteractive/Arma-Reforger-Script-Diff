#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Prefab Template Class Check",
	description: "PrefabsLibrary templates must not have a different class than their Prefabs counterpart; this tool verifies this.",
	wbModules: { "ResourceManager" },
	category: "Prefabs",
	awesomeFontCode: 0xF560)]
class SCR_PrefabTemplateClassCheckPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "0", desc: "Tries and replaces wrong classes (difference between Template declared class and parent Prefab class)")]
	protected bool m_bFixTemplateClasses;

	protected static const int MIN_DESCRIPTION_WIDTH = 110; // chars for Prefab path alignment

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		FindAndPrintDiscrepancies();
	}

	//------------------------------------------------------------------------------------------------
	//! Finds and prints discrepancies
	protected void FindAndPrintDiscrepancies()
	{
		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs);

		array<ResourceName> resourceNames = {};

		foreach (string addonGUID : addonGUIDs)
		{
			string addonID = GameProject.GetAddonID(addonGUID);
			if (addonID == "core")
				continue;

			Print("Looking into " + addonID + "/PrefabLibrary", LogLevel.NORMAL);
			resourceNames.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, null, SCR_AddonTool.ToFileSystem(addonID) + "PrefabLibrary"));
		}

		int count = resourceNames.Count();
		Print("Processing " + count + " Templates class check", LogLevel.NORMAL);

		Resource resource;
		BaseContainer baseContainer;
		BaseContainer baseContainerAncestor;

		map<ResourceName, string> classnameCache = new map<ResourceName, string>();
		array<ResourceName> invalidTemplates = {};								// templates that cannot be loaded with Resource.Load (broken, etc)
		array<ResourceName> orphanTemplates = {};								// templates that are not from a Prefab (should NEVER happen?!)
		array<ref Tuple3<ResourceName, string, string>> classMismatches = {};	// class = Template/Prefab .et class names do not match
		array<ref Tuple3<ResourceName, string, string>> selfMismatches = {};	// self = Prefab.GetClassName() returns a different class name from file

		Debug.BeginTimeMeasure();

		foreach (ResourceName resourceName : resourceNames)
		{
			resource = Resource.Load(resourceName);
			if (!resource.IsValid())
			{
				invalidTemplates.Insert(resourceName);
				continue;
			}

			baseContainer = resource.GetResource().ToBaseContainer();
			if (!baseContainer)
			{
				invalidTemplates.Insert(resourceName);
				continue;
			}

			baseContainerAncestor = baseContainer.GetAncestor();
			if (!baseContainerAncestor)
			{
				orphanTemplates.Insert(resourceName);
				continue;
			}

			string childClassName = classnameCache.Get(resourceName);
			if (!childClassName)			// not cached? let's find it
			{
				childClassName = GetClassNameFromPrefabFile(resourceName);
				if (!childClassName)		// could not get it
					continue;				// GetClassNameFromPrefabFile already warns in case of failure

				classnameCache.Insert(resourceName, childClassName);
			}

			ResourceName ancestorResourceName = baseContainerAncestor.GetResourceName();
			string ancestorClassName = classnameCache.Get(ancestorResourceName);
			if (!ancestorClassName)			// not cached? let's find it
			{
				ancestorClassName = GetClassNameFromPrefabFile(ancestorResourceName);
				if (!ancestorResourceName)	// could not get it
					continue;				// GetClassNameFromPrefabFile already warns in case of failure

				classnameCache.Insert(ancestorResourceName, ancestorClassName);
			}

			if (childClassName != ancestorClassName)
			{
				classMismatches.Insert(new Tuple3<ResourceName, string, string>(resourceName, childClassName, ancestorClassName));
				continue;
			}
		}

		Debug.EndTimeMeasure("Processed " + count + " Templates");

		foreach (ResourceName resourceName : invalidTemplates)
		{
			Print(SCR_StringHelper.PadRight("Template could not be loaded: ", MIN_DESCRIPTION_WIDTH) + resourceName, LogLevel.ERROR);
		}

		foreach (ResourceName resourceName : invalidTemplates)
		{
			Print(SCR_StringHelper.PadRight("Template is not a child: ", MIN_DESCRIPTION_WIDTH) + resourceName, LogLevel.ERROR);
		}

		int fixedClassMismatches;
		foreach (Tuple3<ResourceName, string, string> tuple : classMismatches)
		{
			Print(SCR_StringHelper.PadRight(string.Format("Template class (%1) differs from ancestor Prefab class (%2): ", tuple.param2, tuple.param3), MIN_DESCRIPTION_WIDTH) + tuple.param1, LogLevel.WARNING);
			if (m_bFixTemplateClasses)
			{
				if (ChangeTemplateClass(tuple.param1, tuple.param3))
				{
					Print("Successfully changed class " + tuple.param2 + " to " + tuple.param3, LogLevel.NORMAL);
					fixedClassMismatches++;
				}
				else
				{
					Print("Could not change class " + tuple.param2 + " to " + tuple.param3, LogLevel.WARNING);
				}
			}
		}

		Print("Found " + invalidTemplates.Count() + " invalid, " +
			orphanTemplates.Count() + " orphans, " +
			classMismatches.Count() + " class mismatches (" + fixedClassMismatches + " fixed), " +
			"over " + count + " Templates", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Get a class name reading it from the .et file itself
	//! Only accepts .et (could theoretically accept .ct)
	//! \param[in] resourceName the Prefab file from which to get the class name
	//! \return found class name or empty string if not found (file does not exist, could not be opened or read, etc)
	protected string GetClassNameFromPrefabFile(ResourceName resourceName)
	{
		if (!resourceName || !resourceName.EndsWith(".et"))
		{
			Print("Cannot read class name from a file other than Prefab: " + resourceName, LogLevel.WARNING);
			return string.Empty;
		}

		string absoluteFilePath;
		if (!Workbench.GetAbsolutePath(resourceName.GetPath(), absoluteFilePath, true))
		{
			Print("Cannot find Prefab file to read class name: " + resourceName, LogLevel.ERROR);
			return string.Empty;
		}

		FileHandle fileHandle = FileIO.OpenFile(absoluteFilePath, FileMode.READ);
		if (!fileHandle)
		{
			Print("Cannot open Prefab file to read class name: " + resourceName, LogLevel.ERROR);
			return string.Empty;
		}

		string firstLine;
		fileHandle.ReadLine(firstLine);
		fileHandle.Close();

		int spaceIndex = firstLine.IndexOf(" "); // Thomas Pesquet's finger
		if (spaceIndex < 1) // at least... one character?
		{
			Print("Cannot read class from Prefab file: " + resourceName, LogLevel.ERROR);
			return string.Empty;
		}

		return firstLine.Substring(0, spaceIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Replace the provided Prefab's class name in the file
	//! Only accepts .et (could theoretically accept .ct)
	//! \param[in] resourceName the Prefab file
	//! \param[in] newClassName the new class name
	//! \return found class name or empty string if not found (file does not exist, could not be opened or read, etc)
	protected bool ChangeTemplateClass(ResourceName resourceName, string newClassName)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(newClassName))
		{
			Print("No class provided to write in Prefab: " + resourceName, LogLevel.WARNING);
			return string.Empty;
		}

		newClassName.TrimInPlace(); // juuust in case.

		if (!resourceName || !resourceName.EndsWith(".et"))
		{
			Print("Cannot write class name in a file other than Prefab: " + resourceName, LogLevel.WARNING);
			return string.Empty;
		}

		string absoluteFilePath;
		if (!Workbench.GetAbsolutePath(resourceName.GetPath(), absoluteFilePath, true))
		{
			Print("Cannot find Prefab file to write class name: " + resourceName, LogLevel.ERROR);
			return string.Empty;
		}

		FileHandle fileHandle = FileIO.OpenFile(absoluteFilePath, FileMode.READ);
		if (!fileHandle)
		{
			Print("Cannot open Prefab file to write class name: " + resourceName, LogLevel.ERROR);
			return string.Empty;
		}

		string lineContent;
		array<string> lines = {};
		while (!fileHandle.IsEOF())
		{
			fileHandle.ReadLine(lineContent);
			lines.Insert(lineContent);
		}

		fileHandle.Close();

		int spaceIndex = lines[0].IndexOf(" "); // Thomas Pesquet's finger
		if (spaceIndex < 1) // at least... one character?
		{
			Print("Cannot find class from Prefab file: " + resourceName, LogLevel.ERROR);
			return string.Empty;
		}

		lines[0] = newClassName + lines[0].Substring(spaceIndex, lines[0].Length() - spaceIndex);

		fileHandle = FileIO.OpenFile(absoluteFilePath, FileMode.WRITE);
		if (!fileHandle)
		{
			Print("Cannot write new class in Prefab file: " + resourceName, LogLevel.WARNING);
			return false;
		}

		foreach (int lineNumber, string line : lines)
		{
			fileHandle.WriteLine(line);
		}

		fileHandle.Close();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("bweh", "Settings", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}
}
#endif // WORKBENCH
