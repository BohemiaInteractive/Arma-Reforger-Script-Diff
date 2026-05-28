/*!
Collection of game tests.
Ensures the world is loaded before tests will run.
Provides integration with test specific logger for improved output.
*/
[BaseContainerProps(category: "Autotest")]
class SCR_AutotestSuiteBase : TestSuite
{
	//------------------------------------------------------------------------------------------------
	//! Override in your user test suites to specify the world the test will run in.
	ResourceName GetWorldFile()
	{
		return SCR_AutotestHelper.GetDefaultWorld();
	}

	//------------------------------------------------------------------------------------------------
	//! Override in your user test suites to specify the world systems config the test will run with.
	ResourceName GetWorldSystemsConfigFile()
	{
		return SCR_AutotestHelper.GetDefaultSystemsConfig();
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
		ResourceName systemsConfig = GetWorldSystemsConfigFile();
		if (world && !SCR_AutotestHelper.WorldOpenFile(world, systemsConfig))
		{
			string failure = string.Format("Failed to load world: %1, %2", world, systemsConfig);
			SCR_AutotestHarness.GetLogger().Log(failure, level: LogLevel.ERROR);

			SetResult(SCR_AutotestResult.AsFailure(failure));
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

#ifdef MODULE_AUTOTEST
// HACK: prevents script compilation from sealing methods by overriding them in any class
// fixes the issue with not being able to override these methods in different script modules (Autotest/)
sealed class SCR_Hack_AutotestSuiteBase : SCR_AutotestSuiteBase
{
	override ResourceName GetWorldFile();
	override ResourceName GetWorldSystemsConfigFile();
}
#endif