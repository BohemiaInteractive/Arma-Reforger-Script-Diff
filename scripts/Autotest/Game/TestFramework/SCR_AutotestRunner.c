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
	//------------------------------------------------------------------------------------------------
	static bool ShouldCreate()
	{
		HandleCommandLineArguments();

		bool shouldCreate = SCR_AutotestHarness.s_bIsRunning;
		if (!shouldCreate)
		{
			return false;
		}

		Print("Creating: SCR_TestRunner", LogLevel.NORMAL);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	private static void HandleCommandLineArguments()
	{
		bool autotestParamPresent = System.IsCLIParam("autotest");
		if (!autotestParamPresent)
			return;

		string autotestConfigCLI;
		System.GetCLIParam("autotest", autotestConfigCLI);

		if (!autotestConfigCLI)
		{
			Debug.Error("Empty -autotest parameter value");
			GetGame().RequestClose();
		}

		if (autotestConfigCLI.StartsWith("{"))
		{
			Resource configHolder = Resource.Load(autotestConfigCLI);
			if (configHolder.IsValid())
			{
				SCR_AutotestGroup config = SCR_AutotestGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(configHolder.GetResource().ToBaseContainer()));
				if (config)
				{
					PrintFormat("CLI autotest config: %1", config, level: LogLevel.NORMAL);
					SCR_AutotestHarness.Begin(config, true);
					return;
				}

				PrintFormat("Specified resource is not of type SCR_AutotestGroup: %1", config, level: LogLevel.ERROR);
			}
			else
			{
				PrintFormat("Invalid resource path for autotest config: %1", autotestConfigCLI, level: LogLevel.ERROR);
			}
		}

		if (autotestConfigCLI.ToType().IsInherited(SCR_AutotestSuiteBase))
		{
			PrintFormat("CLI autotest suite: %1", autotestConfigCLI, level: LogLevel.NORMAL);
			SCR_AutotestSuiteBase testSuite = SCR_AutotestSuiteBase.Cast(autotestConfigCLI.ToType().Spawn());
			SCR_AutotestHarness.Begin(testSuite, true);
			return;
		}

		if (autotestConfigCLI.ToType().IsInherited(SCR_AutotestCaseBase))
		{
			PrintFormat("CLI autotest case: %1", autotestConfigCLI, level: LogLevel.NORMAL);
			SCR_AutotestCaseBase testCase = SCR_AutotestCaseBase.Cast(autotestConfigCLI.ToType().Spawn());
			SCR_AutotestHarness.Begin(testCase, true);
			return;
		}

		Debug.Error(string.Format("Invalid -autotest parameter value: %1", autotestConfigCLI));
		GetGame().RequestClose();
	}

	//------------------------------------------------------------------------------------------------
	void OnUpdate(notnull Game game)
	{
		if (!SCR_AutotestHarness.s_bIsRunning)
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
		report.WriteFailedList();

#ifdef ENABLE_DIAG
		SCR_AutotestDebugMenu.Terminate();
#endif

		if (ShouldCloseGameAfterRun())
		{
			// GameStateTransitions.RequestGameTerminateTransition();
			GetGame().RequestClose();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Abort(notnull Game game)
	{
#ifdef WORKBENCH
		// on game end is called two times when going back to EditorMode
		// on the first call InPlayMode is true, false on second
		if (!game.InPlayMode() && SCR_AutotestHarness.s_bIsRunning)
			SCR_AutotestHarness.Finish(abort: true);
#endif
	}

	//------------------------------------------------------------------------------------------------
	private bool ShouldCloseGameAfterRun()
	{
		return SCR_AutotestHarness.s_bCloseGameAfterRun;
	}
}