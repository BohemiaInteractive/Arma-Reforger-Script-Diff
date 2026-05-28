#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Autotest Tool",
	description: "",
	awesomeFontCode: 0xF188
)]
class SCR_AutotestTool : WorldEditorTool
{
	[Attribute("", UIWidgets.ResourceNamePicker, category: "Test Selection", desc: "", params: "conf class=SCR_AutotestGroup")]
	protected ResourceName m_sTestGroup;

	[Attribute("", UIWidgets.EditBox, category: "Test Selection", desc: "Name of test case or test suite class to execute.")]
	protected string m_sTestClass;
	
	[Attribute("", UIWidgets.CheckBox, category: "New Process", desc: "Should the tests run in new process.")]
	protected bool m_bRunInNewWindow;

	[Attribute(SCR_AutotestHelper.GetDefaultLaunchParams(), UIWidgets.EditBoxMultiline, category: "New Process", desc: "Additional arguments the new process will launch with.")]
	protected string m_bArguments;

	protected ScriptEditor m_ScriptEditor = Workbench.GetModule(ScriptEditor);

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run group")]
	protected void ButtonRunGroup()
	{
		PrintFormat("User requested test run: %1", m_sTestGroup, level: LogLevel.NORMAL);

		SCR_AutotestGroup config = GetConfigByPath(m_sTestGroup);
		if (!config)
		{
			Print("Invalid config", LogLevel.ERROR);
		}

		if (m_bRunInNewWindow)
		{
			RunNewProcess(m_sTestGroup);
			return;
		}

		SCR_AutotestPlugin autotestPlugin = SCR_AutotestPlugin.Cast(m_ScriptEditor.GetPlugin(SCR_AutotestPlugin));
		autotestPlugin.FocusWorldEditor();

		autotestPlugin.RunConfig(config);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run class")]
	protected void ButtonRunClass()
	{
		if (m_bRunInNewWindow)
		{
			RunNewProcess(m_sTestClass);
			return;
		}

		SCR_AutotestPlugin autotestPlugin = SCR_AutotestPlugin.Cast(m_ScriptEditor.GetPlugin(SCR_AutotestPlugin));
		autotestPlugin.FocusWorldEditor();

		autotestPlugin.RunClassName(m_sTestClass.Trim(), true);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AutotestGroup GetConfigByPath(ResourceName configName)
	{
		Resource configHolder = Resource.Load(configName);

		return SCR_AutotestGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(configHolder.GetResource().ToBaseContainer()));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RunNewProcess(string autotestArg)
	{
		string cwd;
		Workbench.GetCwd(cwd);
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		SCR_AutotestToolPlugin settings = SCR_AutotestToolPlugin.Cast(worldEditor.GetPlugin(SCR_AutotestToolPlugin));

		string exePath = settings.GetExecutablePath();
		if (!exePath)
		{
			Workbench.Dialog("Autotest Tool - Error", "Executable path not configured.\nPlugins > Settings > Autotest Tool");
			return;
		}
		
		string exe = exePath;
		string gproj = Workbench.GetCurrentGameProjectFile();

		string cmd = string.Format("\"%1\" -gproj \"%2\"", exe, gproj);

		string addonDirs = GetAddonsDirCLI();
		cmd += string.Format(" -addonsDir %1", addonDirs);

		string addons = GetAddonsCLI();
		cmd += string.Format(" -addons %1", addons);
		
		cmd += string.Format(" -autotest %1", autotestArg);

		// user provided args
		cmd += " " + m_bArguments;

		Print("\nRunning command:\n" + cmd, LogLevel.NORMAL);
		ProcessHandle handle = Workbench.RunProcess(cmd);
		if (!handle)
			Print("Autotest couldn't run. Check if your Executable or other settings are correct", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetAddonsDirCLI()
	{
		string addonsDir;
		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);

		foreach (string GUID : addonsGUIDs)
		{
			if (!GameProject.IsVanillaAddon(GUID))
			{
				string addonPath = "$" + GameProject.GetAddonID(GUID) + ":";
				string absPath;
				if (Workbench.GetAbsolutePath(addonPath, absPath))
				{
					if (!addonsDir.IsEmpty())
						addonsDir += ",";

					addonsDir += absPath;
				}
			}
		}

		return "\"" + addonsDir + "\"";
	}

	//------------------------------------------------------------------------------------------------
	protected string GetAddonsCLI()
	{
		string addonIDs;

		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);

		foreach (string GUID : addonsGUIDs)
		{
			if (!GameProject.IsVanillaAddon(GUID))
			{
				if (!addonIDs.IsEmpty())
					addonIDs += ",";

				addonIDs += GameProject.GetAddonID(GUID);
			}
		}

		return addonIDs;
	}
}
#endif
