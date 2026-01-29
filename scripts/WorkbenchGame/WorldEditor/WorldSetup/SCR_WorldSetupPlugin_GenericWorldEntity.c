#ifdef WORKBENCH
//! This Plugin is experimental because it directly writes in the layer file, expecting a certain directory structure and file format.
//! An engine solution should be provided for this matter.
[WorkbenchPluginAttribute(
	name: "[Experimental] World Entity Setup (LOAD the world after this, do NOT save!)",
	category: "World Setup",
	description: "Set up GenericWorldEntity's Prefab",
	shortcut: "", // "Ctrl+Shift+N", // for new content?
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF0AC)]
class SCR_WorldSetupPlugin_GenericWorldEntity : SCR_WorldSetupPluginBasePlugin
{
	[Attribute(desc: "The World Entity Prefab to be used for this world", uiwidget: UIWidgets.ResourceNamePicker, params: "et class=GenericWorldEntity", defvalue: "{08A95D735ECC517F}Prefabs/World/DefaultWorld/GenericWorld_Default.et")]
	protected ResourceName m_sWorldEntityPrefab;

	[Attribute(defvalue: "0", desc: "Create Prefab's child from the provided one in the selected addon and use it instead of using the provided Prefab directly")]
	protected bool m_bCopyPrefabToAddon;

	[Attribute(/* defvalue: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 2)[0].m_Value, */uiwidget: UIWidgets.ComboBox, desc: "Addon in which the Prefab will be copied (if copied)", enums: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 2))]
	protected int m_iAddon;

	protected static const string CHILD_PREFAB = "%1_%2"; // %1 = parent Prefab filename, %2 = world name
	protected static const string LAYERS_DIR = "%1_Layers"; // %1 = world name
	protected static const string DEFAULT_LAYER = "default.layer";
	// protected static const string WORLD_ENTITY_CLASS = ((typename)GenericWorldEntity).ToString();
	protected static const string WORLD_ENTITY_CLASS = "GenericWorldEntity";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("World Setup (WorldEntity)", "This plugin replaces the current WorldEntity (generic entity or Prefab) by the provided Prefab.", this))
			return;

		ResourceName worldEntityPrefab = m_sWorldEntityPrefab;

		if (worldEntityPrefab.IsEmpty())
		{
			Print("No World Entity Prefab provided", LogLevel.WARNING);
			return;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.WARNING);
			return;
		}

		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);
		string worldName = FilePath.StripExtension(FilePath.StripPath(worldPath));

		// duplicate the Prefab
		if (m_bCopyPrefabToAddon)
			worldEntityPrefab = CreatePrefabChildInAddon(worldEntityPrefab, m_iAddon, worldName);

		if (worldEntityPrefab.IsEmpty())
		{
			Print("Child Prefab could not be created", LogLevel.WARNING);
			return;
		}

		if (SCR_WorldFilesHelper.ReplaceWorldEntity(worldPath, worldEntityPrefab))
			Print("File written successfully. Please LOAD the world to see the changes applied", LogLevel.NORMAL);
		else
			Print("File could not be written", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the path to the default layer (worldDir/worldName_Layers/default.layer)
	//! \param[in] worldPath world's directory
	//! \param[in] worldName world's name (used to determine layers directory)
	//! \return path to the default layer
	protected string GetDefaultLayerAbsoluteFilePath(string worldPath, string worldName)
	{
		string defaultLayerAbsFilePath;
		if (!Workbench.GetAbsolutePath(worldPath, defaultLayerAbsFilePath))
		{
			Print("Could not get absolute path for " + worldPath, LogLevel.WARNING);
			return string.Empty;
		}

		string layerAbsDir = FilePath.StripFileName(defaultLayerAbsFilePath);
		if (!layerAbsDir.EndsWith("/"))
			layerAbsDir += "/";

		return layerAbsDir + string.Format(LAYERS_DIR, worldName) + "/" + DEFAULT_LAYER;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
#endif // WORKBENCH
