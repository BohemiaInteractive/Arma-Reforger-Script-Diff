#ifdef WORKBENCH

[WorkbenchPluginAttribute(
	name: "Run test",
	description: "Runs the test suite or test case the cursor currently is in.\nSupports only SCR_Autotest* suites.",
	shortcut: "F4",
	wbModules: { "ScriptEditor" },
//	category: SCR_PluginCategory.SCRIPTEDITOR_ASSISTANCE,
	awesomeFontCode: 0xF188
)]
class SCR_AutotestPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "1", desc: "Focus world editor when starting the run")]
	bool m_bFocusWorldEditor;

	[Attribute(defvalue: "1", desc: "Open result dialog after the test run")]
	bool m_bOpenDialogAfterRun;

	[Attribute(defvalue: "0", desc: "Open the autotest.log after the test run")]
	bool m_bOpenLogAfterRun;

	[Attribute(defvalue: "0", desc: "Output all logs for successful tests in autotest.log")]
	bool m_bVerboseLog;

	[Attribute(defvalue: "1", desc: "Close the game after test runner has finished")]
	bool m_bCloseGameAfterRun;

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
		{
			FocusWorldEditor();
		}

		RunClassName(className.Trim());
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Shows plugin configuration dialog.
	override void Configure()
	{
		Workbench.ScriptDialog("Autotest Plugin Configuration", "", this);
	}

	//------------------------------------------------------------------------------------------------
	//! Run tests by providing test group config.
	void RunConfig(notnull SCR_AutotestGroup testGroupConfig)
	{
		PrintFormat("Running test group: %1", testGroupConfig, level: LogLevel.NORMAL);

		SCR_AutotestHarness.s_bOpenLogAfterRun = m_bOpenLogAfterRun;
		SCR_AutotestHarness.s_bOpenDialogAfterRun = m_bOpenDialogAfterRun;
		SCR_AutotestHarness.s_bCloseGameAfterRun = m_bCloseGameAfterRun;

		SCR_AutotestHarness.Begin(testGroupConfig, true, m_bVerboseLog);
	}

	//------------------------------------------------------------------------------------------------
	//! Run tests by providing test suite or test case class name.
	void RunClassName(string className, bool gui = false)
	{
		if (className.ToType().IsInherited(SCR_AutotestSuiteBase))
		{
			RunTestSuite(className);
			return;
		}

		if (className.ToType().IsInherited(SCR_AutotestCaseBase))
		{
			RunTestCase(className, gui);
			return;
		}

		PrintFormat("Current line is not inside of a test suite or test case: \"%1\"", className, level: LogLevel.WARNING);
		if (gui)
		{
			string msg = string.Format("\"%1\" is not a test suite or test case class.", className);
			Workbench.Dialog("Invalid class", msg);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RunTestSuite(string testSuiteClass)
	{
		PrintFormat("Running test suite: %1", testSuiteClass, level: LogLevel.NORMAL);

		SCR_AutotestHarness.s_bOpenLogAfterRun = m_bOpenLogAfterRun;
		SCR_AutotestHarness.s_bOpenDialogAfterRun = m_bOpenDialogAfterRun;
		SCR_AutotestHarness.s_bCloseGameAfterRun = m_bCloseGameAfterRun;

		SCR_AutotestSuiteBase testSuite = SCR_AutotestSuiteBase.Cast(testSuiteClass.ToType().Spawn());
		SCR_AutotestHarness.Begin(testSuite, true, m_bVerboseLog);
	}

	//------------------------------------------------------------------------------------------------
	protected void RunTestCase(string testCaseClass, bool gui)
	{
		PrintFormat("Running test case: %1", testCaseClass, level: LogLevel.NORMAL);
		SCR_AutotestCaseBase testCase = SCR_AutotestCaseBase.Cast(testCaseClass.ToType().Spawn());

		// TODO(maciejewskifil) refactor
		/*if (!testCase.GetSuite())
		{
			Print("Specified test does not have parent suite.", LogLevel.WARNING);
			if (gui)
				Workbench.Dialog("Error", "Specified test does not have parent suite.");

			return;
		}*/

		SCR_AutotestHarness.s_bOpenLogAfterRun = m_bOpenLogAfterRun;
		SCR_AutotestHarness.s_bOpenDialogAfterRun = m_bOpenDialogAfterRun;
		SCR_AutotestHarness.s_bCloseGameAfterRun = m_bCloseGameAfterRun;

		SCR_AutotestHarness.Begin(testCase, true, m_bVerboseLog);
	}

	[Friend(SCR_AutotestTool)]
	protected void FocusWorldEditor()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		// getting plugin focuses the module window
		worldEditor.GetPlugin(SCR_AutotestTool);
	}
}
#endif
