#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Prefab Structure Creation",
	description: "Create Prefab structure according to config",
	category: "Prefab Structure",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF6BE)]
class SCR_PrefabStructureCreationPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "0", desc: "In which addon this structure is to be created", uiwidget: UIWidgets.ComboBox, enums: SCR_ParamEnumArray.FromAddons())]
	protected int m_iAddon;

//	[Attribute()]
	protected ref SCR_PrefabHelper_Structure m_ConfigType;

	[Attribute(defvalue: "{009E0970F762CC4E}Configs/Workbench/PrefabStructureCreationPlugin/CarPrefabStructure.conf", params: "conf class=SCR_PrefabHelper_Structure")]
	protected ResourceName m_sStructureToCreate;

	[Attribute(defvalue: "", desc: "Project Name (e.g S105) - allowed characters: " + ALLOWED_CHARACTERS)]
	string m_sProjectName;

	protected static const string ALLOWED_CHARACTERS = SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + SCR_StringHelper.UNDERSCORE;

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		m_sProjectName.TrimInPlace();

		if (!DialogConfirmation())
			return;

		if (!SCR_FileIOHelper.IsValidFileName(m_sProjectName))
			return;

		if (!m_ConfigType.m_Directory)
			Print("No directory setup", LogLevel.WARNING);

		string addonFileSystem = SCR_AddonTool.GetAddonFileSystem(m_iAddon);
		string absoluteAddonDirectory;
		if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddon, string.Empty, absoluteAddonDirectory, true))
		{
			Print("Cannot find path for addon " + addonFileSystem, LogLevel.ERROR);
			return;
		}

		m_ConfigType.m_sName = m_sProjectName;
		if (!SCR_PrefabHelper.CreatePrefabStructure(m_ConfigType, addonFileSystem))
		{
			Print("Cannot create Prefab structure", LogLevel.WARNING);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true on success OR if it is impossible to fill m_ConfigType from a filled m_sStructureToCreate, false otherwise
	protected bool DialogConfirmation()
	{
		if (!Workbench.ScriptDialog("Prefab Structure Creation", "", this))
			return false;

		array<string> errors = {};

		if (m_sStructureToCreate)
		{
			m_ConfigType = SCR_ConfigHelperT<SCR_PrefabHelper_Structure>.GetConfigObject(m_sStructureToCreate);
//			m_sStructureToCreate = ResourceName.Empty;
			if (!m_ConfigType)
			{
				Print("Cannot get m_ConfigType from " + m_sStructureToCreate, LogLevel.ERROR);
				Workbench.Dialog("Error", "Cannot get m_ConfigType from " + m_sStructureToCreate);
				return true;
			}
		}
		else // no config provided
		{
			errors.Insert("Structure To Create is null");
		}

		m_sProjectName = SCR_StringHelper.Filter(m_sProjectName, ALLOWED_CHARACTERS, false);
		if (!m_sProjectName)
			errors.Insert("Project Name is invalid");

		if (!errors.IsEmpty())
		{
			Workbench.Dialog("Error", SCR_StringHelper.Join(SCR_StringHelper.LINE_RETURN, errors));
			errors.Clear();
		}

		return errors.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected int ButtonOK()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonClose()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_PrefabStructureCreationPlugin();
}
#endif // WORKBENCH
