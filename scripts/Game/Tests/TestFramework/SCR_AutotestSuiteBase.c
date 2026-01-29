/*!
Collection of game tests.
Ensures the world is loaded before tests will run.
Provides integration with test specific logger for improved output.
*/
[BaseContainerProps(category: "Autotest")]
class SCR_AutotestSuiteBase : TestSuite
{
	// This is not related to suite IsEnabled state.
	[Attribute("1", desc: "Enable/Disable test suite in config. Used to temporarily prevent it from running even if present in a test group.")]
	protected bool m_bEnabled;
	
	//------------------------------------------------------------------------------------------------
	//! Override in your user test suites to specify the world the test will run in.
	ResourceName GetWorldFile()
	{
		return SCR_AutotestHelper.WORLD_AUTOTEST_PLAIN;
	}

	//------------------------------------------------------------------------------------------------
	//! Prints content of variable to console/log and autotest/log.
	//! Shadows global Print to force the logs to go through SCR_AutotestPrinter.
	//!
	void Print(string msg, LogLevel level = LogLevel.NORMAL)
	{
		SCR_AutotestHarness.GetLogger().Log(msg, level);
	}

	//------------------------------------------------------------------------------------------------
	//! Prints formated text to console/log and autotest/log.
	//! Shadows global PrintFormat to force the logs to go through SCR_AutotestPrinter.
	//!
	void PrintFormat(string fmt, string param1 = "", string param2 = "", string param3 = "", LogLevel level = LogLevel.NORMAL)
	{
		SCR_AutotestHarness.GetLogger().Log(string.Format(fmt, param1, param2, param3), level);
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShouldSkipInGroup()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Log "opening" part of the test suite output.
	[Step(EStage.Setup)]
	private void Setup_PrintPrelude()
	{
		SCR_AutotestHarness.GetLogger().PrintTestSuitePrelude(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Open world requested by this test suite.
	[Step(EStage.Setup)]
	private void Setup_OpenWorld()
	{
		ResourceName world = GetWorldFile();
		if (world && !SCR_AutotestHelper.WorldOpenFile(world))
		{
			SetResult(SCR_AutotestResult.AsFailure("Failed to load world: %1", world));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Waits for the world to load.
	[Step(EStage.Setup)]
	private bool Setup_AwaitWorld()
	{
		return !GameStateTransitions.IsTransitionRequestedOrInProgress();
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus that could interfere with the test suite.
	[Step(EStage.Setup)]
	private void Setup_CloseMenus()
	{
		GetGame().GetMenuManager().CloseAllMenus();
	}

	//------------------------------------------------------------------------------------------------
	//! Log "closing" part of the test suite output.
	[Step(EStage.TearDown)]
	private void TearDown_PrintEpilogue()
	{
		SCR_AutotestHarness.GetLogger().PrintTestSuiteEpilogue(this);
	}
}
