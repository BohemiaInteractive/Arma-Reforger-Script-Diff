/*!
Class responsible for running the game test suites.

Tests can be started by passing the -autotest parameter, the runner supports SCR_AutotestGroup configs, test suite class names and test case class names.
CLI examples:
	-autotest "{6AB9C8EEE9A651B5}"
	-autotest Example1SubjectTestSuite
	-autotest Example1Subject_GetFive_ReturnsFive

GUI usage:
	SCR_AutotestTool
	SCR_AutotestPlugin
*/
sealed class SCR_TestRunner
{	
	private ref SCR_AutotestRunSettings m_RunSettings;
	
	private static ref SCR_TestRunner s_Instance;
	
	static ref SCR_TestRunner GetInstance()
	{
		return s_Instance;
	}
	
	static bool HasInstance()
	{
		return s_Instance;
	}
	
	private static bool IsConfigured()
	{
		return s_Instance && s_Instance.m_RunSettings;
	}
	
	private static bool IsRunning()
	{
		return HasInstance() && SCR_AutotestHarness.IsRunning();
	}
	
	private void SCR_TestRunner();
	
	//-----------------------------------------------------------------------------
	//! The main entry point of the Autotest Framework
	//! This method configures the runner and bootstraps the harness, which ensures
	//! that tests will run the next time the game/world is loaded.
	//! [param] force aborts the currently running tests and inits a new runner
	static void InitRunner(notnull SCR_AutotestRunSettings settings, bool force = false)
	{	
		if (force)
		{
			Print("Forcing a new test run. Clearing the instance...");
			AbortRunner();
		}
			
		if (IsRunning())
		{
			Print("Tests are already running");
			return;
		}

		Configure(settings);
		SCR_AutotestHarness.Bootstrap(settings);
	}
	
	//! Clears runner instance and aborts currently running tests.
	//! Will NOT produce report files.
	static void AbortRunner()
	{
		if (SCR_AutotestHarness.IsRunning())
			SCR_AutotestHarness.Finish(abort: true);
		
		delete s_Instance;
	}
	
	//! Creates an instance if it doesn't exist, and assigns settings
	//! This DOES NOT bootstrap the harness
	private static void Configure(notnull SCR_AutotestRunSettings settings)
	{
		if (!s_Instance)
			s_Instance = new SCR_TestRunner();
		
		s_Instance.m_RunSettings = settings;
	}
	
	//------------------------------------------------------------------------------------------------
	static void OnGameStart()
	{
		if (SCR_TestRunner.IsConfigured() || !System.IsCLIParam("autotest"))
		{
			return;
		}
		
		SCR_AutotestRunSettings RunSettings = SCR_AutotestRunSettingsBuilder.CreateFromCLI();
		if (!RunSettings)
		{
			GetGame().RequestClose();
			return;
		}
		
		InitRunner(RunSettings);
	}

	void OnUpdate(notnull Game game)
	{
		if (!SCR_AutotestHarness.IsRunning())
			return;

		if (GameStateTransitions.IsTransitionRequestedOrInProgress())
			return;

		if (!game.IsPreloadFinished())
			return;

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AUTOTEST_PROGRESS, true))
			SCR_AutotestDebugMenu.GetInstance().Update();
#endif

		// execute the tests
		if (!SCR_AutotestHarness.Run())
			return;

		Print("SCR_TestRunner has finished running", LogLevel.NORMAL);

		SCR_AutotestReport report = SCR_AutotestHarness.Finish();
		report.WriteJUnitXML();
		report.WriteLogFiles();

		if (ShouldCloseGameAfterRun(report))
		{
			// GameStateTransitions.RequestGameTerminateTransition();
			GetGame().RequestClose();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void OnGameEnd(notnull Game game)
	{
	}
	
	static void OnConsoleCommand(string command)
	{
		// TODO: add console interface
	}

	//------------------------------------------------------------------------------------------------
	private bool ShouldCloseGameAfterRun(notnull SCR_AutotestReport report)
	{
		if (SCR_AutotestHarness.GetActionAfterRun() == SCR_EAutotestOnFinishedAction.EXIT)
			return true;

		if (SCR_AutotestHarness.GetActionAfterRun() == SCR_EAutotestOnFinishedAction.NONE)
			return false;

		if (SCR_AutotestHarness.GetActionAfterRun() == SCR_EAutotestOnFinishedAction.EXIT_IF_NO_ERROR)
			return !report.IsFailure();

		return true;
	}
}
