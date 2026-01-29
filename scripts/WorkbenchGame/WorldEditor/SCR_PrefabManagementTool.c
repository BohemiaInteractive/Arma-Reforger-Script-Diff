#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Prefab Management Tool",
	description: "This Tool allows to:\n" +
		"- clone existing Prefabs\n" +
		"- create existing Prefabs' children\n" +
		"- import XOBs as Prefabs\n" +
		"Resulting Prefabs are created in the source's directory if a save path is not defined",
	awesomeFontCode: 0xF49E)]
class SCR_PrefabManagementTool : WorldEditorTool
{
	/*
		Category: General
	*/

	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "File system where new prefabs will be created", enums: SCR_ParamEnumArray.FromAddons(), category: "General")]
	protected int m_iAddon;

	[Attribute(defvalue: "1", desc: "If a destination file already exists, it is overwritten", category: "General")]
	protected bool m_bOverwriteFiles;

	/*
		Category: Suffixes
	*/

	[Attribute(desc: "suffixes to name clones, e.g _red, _blue, _US, etc; the underscore is added automatically if needed. Accepted chars: [a-zA-Z0 - 9_-]", category: "Suffixes")]
	protected ref array<string> m_aSuffixes1;

	[Attribute(desc: "suffixes to name clones, e.g _red, _blue, _US, etc; the underscore is added automatically if needed. Accepted chars: [a-zA-Z0 - 9_-]", category: "Suffixes")]
	protected ref array<string> m_aSuffixes2;

	[Attribute(desc: "suffixes to name clones, e.g _red, _blue, _US, etc; the underscore is added automatically if needed. Accepted chars: [a-zA-Z0 - 9_-]", category: "Suffixes")]
	protected ref array<string> m_aSuffixes3;

	// ^ more can be added if needed

	/*
		Category: Clone
	*/

	[Attribute(defvalue: "0", desc: "Which Suffixes array will be used for Clones - 0 = no suffixes", /* uiwidget: UIWidgets.Slider, */params: "0 3 1", category: "Clone")]
	protected int m_iCloneSuffixes;

	[Attribute(defvalue: "{C502C0D612FB13CE}Prefabs", desc: "Path to save the clones", params: "unregFolders", category: "Clone")]
	protected ResourceName m_sCloneSavePath;

	/*
		Category: Children
	*/

	[Attribute(defvalue: "0", desc: "Which Suffixes array will be used for creating Children - 0 = no suffixes", /* uiwidget: UIWidgets.Slider, */params: "0 3 1", category: "Children")]
	protected int m_iChildrenSuffixes;

	[Attribute(defvalue: "{C502C0D612FB13CE}Prefabs", desc: "Path to save the prefabs' children", params: "unregFolders", category: "Children")]
	protected ResourceName m_sChildrenSavePath;

	/*
		Category: XOB Import
	*/

	[Attribute(defvalue: "{C502C0D612FB13CE}Prefabs", desc: "Path to save the XOB prefabs", params: "unregFolders", category: "XOB Import")]
	protected ResourceName m_sXOBSavePath;

	[Attribute(defvalue: "0", desc: "if ONE directory is selected and XOB Save Path is defined, " +
		"relative paths will be used to mimic that directory's structure\n" +
		"e.g Assets/Rocks/Granite/Granite_01.xob saves to XOBSavePath/Rocks/Granite/Granite_01.et", category: "XOB Import")]
	protected bool m_bKeepSingleDirectoryStructure;

	[Attribute(desc: "Parent prefab to created prefabs", uiwidget: UIWidgets.ResourcePickerThumbnail, params: PREFAB_EXTENSION, category: "XOB Import")]
	protected ResourceName m_sParentPrefab;

	[Attribute(desc: "Create a \"_base\" prefab for each imported xob", params: PREFAB_EXTENSION, category: "XOB Import")]
	protected bool m_bCreateBasePrefab;

	protected static const string PREFAB_EXTENSION = "et";
	protected static const string DEFAULT_CLONE_SUFFIX = "_clone";
	protected static const string DEFAULT_CHILD_SUFFIX = "_child";

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Clone Prefabs")]
	protected void BtnClonePrefabs()
	{
		array<string> suffixes = GetSelectedSuffixes(m_iCloneSuffixes);
		if (!suffixes)
		{
			Print("No such suffixes defined - check the index", LogLevel.ERROR);
			return;
		}

		CreateClonesOrChildren(suffixes, true);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create Children")]
	protected void BtnCreateChildren()
	{
		array<string> suffixes = GetSelectedSuffixes(m_iChildrenSuffixes);
		if (!suffixes)
		{
			Print("No such suffixes defined - check the index", LogLevel.ERROR);
			return;
		}

		CreateClonesOrChildren(suffixes, false);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Import XOBs")]
	protected void BtnImportXOBs()
	{
		array<ResourceName> foundXOBs;

		string sourceDir;
		string targetDir;

		if (m_bKeepSingleDirectoryStructure)
		{
			array<ResourceName> dirs = SCR_WorldEditorToolHelper.GetSelectedDirectories();
			if (dirs && dirs.Count() == 1)	// get currently selected directory's XOB content
			{
				sourceDir = dirs[0].GetPath();
				targetDir = m_sXOBSavePath.GetPath();
			}
		}

		// get -selected- XOBs
		foundXOBs = SCR_WorldEditorToolHelper.GetSelectedOrOpenedResources("xob");

		if (foundXOBs.IsEmpty())
		{
			Print("No models are opened or selected in the Resource Browser", LogLevel.WARNING);
			return;
		}

		int importedXOBs;

		Debug.BeginTimeMeasure();

		foreach (ResourceName foundXOB : foundXOBs)
		{
			string fileNameWithoutExtension = FilePath.StripExtension(FilePath.StripPath(foundXOB.GetPath()));

			string relativeSaveDirectory;
			if (m_sXOBSavePath.IsEmpty())	// save in xob's directory
				relativeSaveDirectory = FilePath.StripFileName(foundXOB.GetPath());
			else if (sourceDir.IsEmpty())	// save in provided directory
				relativeSaveDirectory = m_sXOBSavePath.GetPath();
			else							// save in provided directory using tree structure
				relativeSaveDirectory = SCR_PrefabHelper.GetRelativeParentDirectory(FilePath.StripFileName(foundXOB.GetPath()), targetDir);

			string absoluteSaveDirectory;
			if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddon, relativeSaveDirectory, absoluteSaveDirectory, false))
			{
				Print("Wrong path: " + SCR_AddonTool.ToFileSystem(SCR_AddonTool.GetAddonID(m_iAddon)) + relativeSaveDirectory, LogLevel.WARNING);
				continue;
			}

			if (!absoluteSaveDirectory.EndsWith("/"))
				absoluteSaveDirectory += "/";

			string absoluteFilePath = absoluteSaveDirectory + fileNameWithoutExtension + "." + PREFAB_EXTENSION;
			if (!m_bOverwriteFiles && FileIO.FileExists(absoluteFilePath))
			{
				Print("File already exists, skipping " + absoluteFilePath, LogLevel.WARNING);
				continue;
			}

			SCR_PrefabHelper.CreatePrefabFromXOB(foundXOB, absoluteFilePath, m_sParentPrefab, m_bCreateBasePrefab);
			importedXOBs++;
		}

		Debug.EndTimeMeasure("Importing " + importedXOBs + "/" + foundXOBs.Count() + " XOBs as Prefabs");
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateClonesOrChildren(array<string> rawSuffixes, bool isCloning)
	{
		array<ResourceName> foundPrefabs = SCR_WorldEditorToolHelper.GetSelectedOrOpenedResources(PREFAB_EXTENSION);
		if (foundPrefabs.IsEmpty())
		{
			Print("No Prefabs are opened or selected in the Resource Browser", LogLevel.WARNING);
			return;
		}

		set<string> suffixes = new set<string>();
		foreach (string suffix : rawSuffixes)
		{
			suffix = SanitiseAndFormatSuffix(suffix);
			if (suffix.IsEmpty())
				continue;

			suffixes.Insert(suffix);
		}

		if (suffixes.IsEmpty())
		{
			if (isCloning)
				suffixes.Insert(DEFAULT_CLONE_SUFFIX);
			else
				suffixes.Insert(DEFAULT_CHILD_SUFFIX);
		}

		int created;

		Debug.BeginTimeMeasure();

		foreach (ResourceName foundPrefab : foundPrefabs)
		{
			string fileNameWithoutExtension = FilePath.StripExtension(FilePath.StripPath(foundPrefab.GetPath()));
			if (fileNameWithoutExtension.EndsWith(SCR_PrefabHelper.PREFAB_BASE_SUFFIX))
				fileNameWithoutExtension = fileNameWithoutExtension.Substring(0, fileNameWithoutExtension.Length() - SCR_PrefabHelper.PREFAB_BASE_SUFFIX.Length());

			string relativeSaveDirectory;
			if (isCloning && !m_sCloneSavePath.IsEmpty())
				relativeSaveDirectory = m_sCloneSavePath.GetPath();
			else if (!isCloning && !m_sChildrenSavePath.IsEmpty())
				relativeSaveDirectory = m_sChildrenSavePath.GetPath();
			else
				relativeSaveDirectory = FilePath.StripFileName(foundPrefab.GetPath()); // prefab's directory

			string absoluteSaveDirectory;
			if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddon, relativeSaveDirectory, absoluteSaveDirectory))
			{
				Print("Wrong path: " + SCR_AddonTool.GetAddonID(m_iAddon) + relativeSaveDirectory, LogLevel.WARNING);
				continue;
			}

			if (!absoluteSaveDirectory.EndsWith("/"))
				absoluteSaveDirectory += "/";

			foreach (string suffix : suffixes)
			{
				string absoluteFilePath = absoluteSaveDirectory + fileNameWithoutExtension + suffix + "." + PREFAB_EXTENSION;
				if (!m_bOverwriteFiles && FileIO.FileExists(absoluteFilePath))
				{
					Print("File already exists, skipping " + absoluteFilePath, LogLevel.WARNING);
					continue;
				}

				ResourceName resourceName;
				if (isCloning)
					resourceName = SCR_PrefabHelper.ClonePrefab(foundPrefab, absoluteFilePath);
				else
					resourceName = SCR_PrefabHelper.CreateChildPrefab(foundPrefab, absoluteFilePath);

				if (resourceName.IsEmpty())
					Print("Could not create prefab " + FilePath.StripPath(absoluteFilePath), LogLevel.ERROR);

				created++;
			}
		}

		if (isCloning)
			Debug.EndTimeMeasure("Cloning " + created + "/" + (foundPrefabs.Count() * suffixes.Count()) + " (" + foundPrefabs.Count() + " prefabs x " + suffixes.Count() + " suffixes)");
		else
			Debug.EndTimeMeasure("Creating " + created + "/" + (foundPrefabs.Count() * suffixes.Count()) + " children (" + foundPrefabs.Count() + " prefabs x " + suffixes.Count() + " suffixes)");
	}

	//------------------------------------------------------------------------------------------------
	//! Strip a suffix of weird or unauthorised characters
	//! Accepted characters: [a-zA-Z0-9_-]
	//! \param[in] suffix
	//! \return underscore-prefixed suffix or empty if wrong
	protected string SanitiseAndFormatSuffix(string suffix)
	{
		suffix = SCR_StringHelper.Filter(suffix, SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + "_-");
		if (suffix.IsEmpty())
			return suffix;

		if (!suffix.StartsWith("_"))
			suffix = "_" + suffix;

		return suffix;
	}

	//------------------------------------------------------------------------------------------------
	protected array<string> GetSelectedSuffixes(int selectedIndex)
	{
		selectedIndex--; // let's get civilised shalt we

		switch (selectedIndex)
		{
			case -1: return {}; // index 0 = no suffixes
			case 0: return m_aSuffixes1;
			case 1: return m_aSuffixes2;
			case 2: return m_aSuffixes3;

			// ^ more can be added
		}

		return null;
	}
}
#endif // WORKBENCH
