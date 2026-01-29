/*!
Game core responsible for running the gameplay test suites.

Tests can be started by passing the -autotest parameter, the runner supports SCR_AutotestGroup configs, test suite class names and test case class names.
CLI examples:
	-autotest "{6AB9C8EEE9A651B5}"
	-autotest Example1SubjectTestSuite
	-autotest Example1Subject_GetFive_ReturnsFive
*/
[BaseContainerProps(configRoot: true)]
sealed class SCR_AutotestRunnerCore : SCR_GameCoreBase
{
	override bool CanCreate()
	{
		HandleCommandLineArguments();

		bool shouldCreate = SCR_AutotestHarness.s_bIsRunning;
		if (!shouldCreate)
		{
			return false;
		}
		
		// TODO this is copied over from original AutotestCore, why we can't run as localhost?
		if (RplSession.Mode() == RplMode.Listen)
		{
			Print("Cannot run autotest as listen server! Make sure to untick 'Server localhost' option before running autotest.", LogLevel.ERROR);
			SCR_AutotestHarness.s_bIsRunning = false;
			GetGame().RequestClose();
			return false;
		}

		Print("AutotestExecution was requested", LogLevel.NORMAL);
		PrintFormat("Creating: %1", Type().ToString(), level: LogLevel.NORMAL);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		if (!SCR_AutotestHarness.s_bIsRunning)
		{
			return;
		}

		if (GameStateTransitions.IsTransitionRequestedOrInProgress())
		{
			return;
		}

		// execute the tests
		if (!SCR_AutotestHarness.Run())
		{
			return;
		}

		Print("SCR_TestRunner has finished running", LogLevel.NORMAL);

		SCR_AutotestReport report = SCR_AutotestHarness.Finish();
		report.WriteJUnitXML();
		report.WriteFailedList();

		if (ShouldCloseGameAfterRun())
		{
			// GameStateTransitions.RequestGameTerminateTransition();
			GetGame().RequestClose();
		}
	}

	//------------------------------------------------------------------------------------------------
	private static void HandleCommandLineArguments()
	{
		string autotestConfigCLI;
		System.GetCLIParam("autotest", autotestConfigCLI);
		if (!autotestConfigCLI)
			return;

		if (autotestConfigCLI.StartsWith("{"))
		{
			Resource configHolder = Resource.Load(autotestConfigCLI);
			SCR_AutotestGroup config = SCR_AutotestGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(configHolder.GetResource().ToBaseContainer()));
			if (config)
			{
				PrintFormat("CLI autotest config: %1", config, level: LogLevel.NORMAL);
				SCR_AutotestHarness.Begin(config, true);
				return;
			}

			PrintFormat("Specified resource is not of type SCR_AutotestGroup: %1", config, level: LogLevel.ERROR);
			return;
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
	private bool ShouldCloseGameAfterRun()
	{
		return SCR_AutotestHarness.s_bCloseGameAfterRun;
	}
}
