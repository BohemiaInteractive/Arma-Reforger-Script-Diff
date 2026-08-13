#ifdef WORKBENCH

[WorkbenchPluginAttribute(
	name: "Autotest",
	description: "Runs the test suite or test case the cursor currently is in.\nSupports only SCR_Autotest* suites.",
	shortcut: "F4",
	wbModules: { "ScriptEditor", "WorldEditor" },
//	category: SCR_PluginCategory.SCRIPTEDITOR_ASSISTANCE,
	awesomeFontCode: 0xF188
)]
class SCR_AutotestPlugin : WorldEditorPlugin
{
	[Attribute(defvalue: "1", desc: "Focus world editor when starting the run")]
	bool m_bFocusWorldEditor;

	[Attribute(defvalue: "1", desc: "Open result dialog after the test run")]
	bool m_bOpenDialogAfterRun;

	[Attribute(defvalue: "0", desc: "Open the autotest.log after the test run")]
	bool m_bOpenLogAfterRun;

	[Attribute(defvalue: "0", desc: "Output all logs for successful tests in autotest.log")]
	bool m_bVerboseLog;

	[Attribute(SCR_EAutotestOnFinishedAction.EXIT.ToString(), desc: "Close the game after test runner has finished?", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EAutotestOnFinishedAction))]
	SCR_EAutotestOnFinishedAction m_eActionAfterRun;

#ifndef AUTOTEST_DISABLE_RUN_SCRIPT_EDITOR
	//------------------------------------------------------------------------------------------------
	//! \param[out] className
	//! \param[out] methodName
	//! \return
	static bool GetCursorClassAndMethodNames(out string className, out string methodName)
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		return SCR_CopyClassAndMethodPlugin.GetCursorClassAndMethodNames(scriptEditor, className, methodName);
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		string className, methodName;
		if (!GetCursorClassAndMethodNames(className, methodName))
		{
			Print("The current line is not inside of a class.", LogLevel.WARNING);
			return;
		}
		
		if (m_bFocusWorldEditor)
			FocusWorldEditor();
		
		SCR_AutotestRunSettings container = CreateParamContainer(className.Trim());
		SCR_TestRunner.InitRunner(CreateParamContainer(className.Trim()));
		SCR_AutotestHelper.SwitchToGameMode();
	}
#endif
	
	[Friend(SCR_AutotestTool)]
	protected SCR_AutotestRunSettings CreateParamContainer(string config)
	{
		SCR_AutotestPlugin plugin = GetSourcePlugin();
		
		SCR_AutotestRunSettings container = SCR_AutotestRunSettingsBuilder
			.CreateContainer(config)
			.WithLogAfterRun(plugin.m_bOpenLogAfterRun)
			.WithDialogAfterRun(plugin.m_bOpenDialogAfterRun)
			.WithActionAfterRun(plugin.m_eActionAfterRun)
			.WithVerboseLog(plugin.m_bVerboseLog)
			.Build();
		
		return container;
	}
	
	override void OnGameModeEnded()
	{
		SCR_TestRunner.AbortRunner();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows plugin configuration dialog.
	override void Configure()
	{
		SCR_AutotestPlugin plugin = GetSourcePlugin();
		
		Workbench.ScriptDialog("Autotest Plugin Configuration", "", plugin);
	}

	[Friend(SCR_AutotestTool)]
	protected void FocusWorldEditor()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		// getting plugin focuses the module window
		worldEditor.GetPlugin(SCR_AutotestTool);
	}
	
	//! We are using this plugin across different workbench windows, but it is instantiated in each one separately.
	//! In order to synchronize the parameters, we only use one "source" instance to set/get the parameters from.
	//! This method finds that "source" plugin.
	private SCR_AutotestPlugin GetSourcePlugin()
	{
		ScriptEditor we = Workbench.GetModule(WorldEditor);
		SCR_AutotestPlugin plugin = SCR_AutotestPlugin.Cast(we.GetPlugin(SCR_AutotestPlugin));
		return plugin;
	}
}
#endif
