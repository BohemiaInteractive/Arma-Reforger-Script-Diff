#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Create New Script", shortcut: "CTRL+N", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xF1C9)]
class SCR_NewScriptPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "File system where the new script file will be created", enums: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 2), category: "General")]
	protected int m_iAddon;

	[Attribute(defvalue: "scripts/Game", desc: "File destination - the path can be selected in an addon (typically the Arma project) and the directory tree will then be created in said targeted addon", params: "unregFolders", category: "General")]
	protected ResourceName m_sLastAbsoluteDestinationDirectory;

	protected static const string SCRIPT_EXTENSION = ".c";
	protected static const string ALLOWED_CLASSNAME_CHARS = SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + SCR_StringHelper.UNDERSCORE;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
			return;

		string currentFilePath;
		bool createRelative = scriptEditor.GetCurrentFile(currentFilePath);

		string relativeDirectory;
		string fileName;
		if (createRelative)
		{
			SCR_NewScriptPlugin_RelativeUI ui = new SCR_NewScriptPlugin_RelativeUI();
			ui.m_iAddon = m_iAddon; // restore save
			ui.m_sDestinationDirectory = FilePath.StripFileName(currentFilePath);
			if (!Workbench.ScriptDialog("Script Creation", "Define the new file's settings.", ui))
				return;

			m_iAddon = ui.m_iAddon; // saves AND stores for later in script
			fileName = ui.m_sFileName;
			if (fileName.EndsWith(SCRIPT_EXTENSION))
				fileName.Substring(0, fileName.Length() - SCRIPT_EXTENSION.Length());

			relativeDirectory = ui.m_sDestinationDirectory;
		}
		else
		{
			SCR_NewScriptPlugin_AbsoluteUI ui = new SCR_NewScriptPlugin_AbsoluteUI();
			ui.m_iAddon = m_iAddon; // restore save
			ui.m_sDestinationDirectory = m_sLastAbsoluteDestinationDirectory;
			if (!Workbench.ScriptDialog("Script Creation", "Define the new file's settings.", ui))
				return;

			m_iAddon = ui.m_iAddon; // saves AND stores for later in script
			fileName = ui.m_sFileName;
			if (fileName.EndsWith(SCRIPT_EXTENSION))
				fileName.Substring(0, fileName.Length() - SCRIPT_EXTENSION.Length());

			relativeDirectory = ui.m_sDestinationDirectory.GetPath();
		}

		string absoluteDirectory;
		if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddon, relativeDirectory, absoluteDirectory, false))
		{
			Print("Cannot Get absolute directory for " + relativeDirectory, LogLevel.WARNING);
			return;
		}

		if (!FileIO.FileExists(absoluteDirectory))
		{
			if (!FileIO.MakeDirectory(absoluteDirectory))
			{
				Print("Cannot create " + absoluteDirectory);
				return;
			}
		}

		if (!absoluteDirectory.EndsWith(SCR_StringHelper.SLASH))
			absoluteDirectory += SCR_StringHelper.SLASH;

		fileName = SCR_StringHelper.Filter(fileName, ALLOWED_CLASSNAME_CHARS, false);

		int i;
		while (!fileName || FileIO.FileExists(absoluteDirectory + fileName))
		{
			fileName = "NewScriptFile_" + ++i;
			if (!fileName.EndsWith(SCRIPT_EXTENSION))
				fileName += SCRIPT_EXTENSION;
		}

		if (!fileName.EndsWith(SCRIPT_EXTENSION))
			fileName += SCRIPT_EXTENSION;

		string absoluteFilePath = absoluteDirectory + fileName;

		FileHandle file = FileIO.OpenFile(absoluteFilePath, FileMode.WRITE);
		if (!file)
		{
			Workbench.Dialog("Script Creation error", "\"" + absoluteFilePath + "\" could not be created.");
			return;
		}

		file.Close();

		Workbench.GetModule(ScriptEditor).SetOpenedResource(absoluteFilePath);
	}
}

class SCR_NewScriptPlugin_RelativeUI
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "File system where the new script file will be created", enums: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 0))]
	int m_iAddon;

	[Attribute(defvalue: "", desc: "File destination - the path can be selected in an addon (typically the Arma project) and Addon be defined to something else, the directory tree will then be created in said targeted addon")]
	string m_sDestinationDirectory;

	[Attribute(desc: "Allowed characters: a-z, A-Z, 0-9, _")]
	string m_sFileName;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected int OkButton()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int CancelButton()
	{
		return 0;
	}
}

class SCR_NewScriptPlugin_AbsoluteUI
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "File system where the new script file will be created", enums: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 2))]
	int m_iAddon;

	[Attribute(defvalue: "", desc: "File destination - the path can be selected in an addon (typically the Arma project) and Addon be defined to something else, the directory tree will then be created in said targeted addon", params: "unregFolders")]
	ResourceName m_sDestinationDirectory;

	[Attribute(desc: "Allowed characters: a-z, A-Z, 0-9, _")]
	string m_sFileName;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected int OkButton()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int CancelButton()
	{
		return 0;
	}
}
#endif // WORKBENCH
