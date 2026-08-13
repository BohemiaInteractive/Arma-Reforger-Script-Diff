/*!
SCR_AutotestHarness is a thin wrapper around TestHarness, responsible for test instantiation and execution.

\code
if (SCR_AutotestHarness.Finished())
{
	SCR_AutotestGroup testGroup = SCR_AutotestHarness.GetDefaultTestGroupConfig();
	SCR_AutotestHarness.Begin(testGroup);
}

SCR_AutotestHarness.DebugPrintSuites();

bool runFinished = SCR_AutotestHarness.Run();
PrintFormat("Run: %1", runFinished, level: LogLevel.DEBUG);

if (runFinished)
{
	SCR_AutotestHarness.End();
	Print("Tests done", LogLevel.DEBUG);
}
\endcode
*/
class SCR_AutotestHarness : TestHarness
{
	protected static ref SCR_AutotestReport s_Report;
	protected static ref SCR_AutotestPrinter s_Logger;

	protected static TestBase s_ActiveTestCase;

	private static bool s_bIsRunning = false;
	
	private static bool s_bOpenLogAfterRun = false;
	private static bool s_bOpenDialogAfterRun = false;
	private static SCR_EAutotestOnFinishedAction m_eActionAfterRun = SCR_EAutotestOnFinishedAction.EXIT;
	private static int s_iWorldTimeCoef = 1;
	
	static ref array<ref SCR_AutotestSuiteBase> s_aCurrentSuites;
	
	static bool IsRunning()
	{
		return s_bIsRunning;
	}
	
	static bool GetOpenLogAfterRun()
	{
		return s_bOpenLogAfterRun;
	}
	
	static bool GetOpenDialogAfterRun()
	{
		return s_bOpenDialogAfterRun;
	}
	
	static SCR_EAutotestOnFinishedAction GetActionAfterRun()
	{
		return m_eActionAfterRun;
	}
	
	static int GetWorldTimeCoef()
	{
		return s_iWorldTimeCoef;
	}
	
	//! Unpacks SCR_AutotestRunSettings container
	private static void SetRunSettings(notnull SCR_AutotestRunSettings settings)
	{
		s_bOpenLogAfterRun = settings.m_bOpenLogAfterRun;
		s_bOpenDialogAfterRun = settings.m_bOpenDialogAfterRun;
		m_eActionAfterRun = settings.m_eActionAfterRun;
		s_iWorldTimeCoef = settings.m_iTimeCoefficient;
	}
	
	//! Consumes SCR_AutotestRunSettings container to prepare the harness for test execution
	//! Processes the config string and instantiates all tests
	[Friend(SCR_TestRunner)]
	protected static void Bootstrap(notnull SCR_AutotestRunSettings params)
	{
		if (s_bIsRunning)
		{
			Print("SCR_AutotestHarness::Bootstrap was called while the test is already running", LogLevel.ERROR);
			return;
		}
		
		array<ref SCR_AutotestSuiteBase> testSuites;
		array<typename> testCases;
		set<typename> skippedCases = params.m_aSkippedCases;
		
		if (IsTestGroup(params.m_sConfig))
			ParseConfig(params.m_sConfig, testSuites);
		
		if (IsTestSuite(params.m_sConfig))
			ParseSuite(params.m_sConfig, testSuites);

		if (IsTestCase(params.m_sConfig))
			ParseCases(params.m_sConfig, testSuites, testCases);
		
		if (!testSuites)
		{
			Debug.Error(string.Format("Invalid -autotest parameter value: %1", params.m_sConfig));
			GetGame().RequestClose();
			return;
		}
		
		SetRunSettings(params);
		ConfigureTestSuites(testSuites, testCases, skippedCases);
		BeginInternal(autorun: true, verboseLog: params.m_bVerboseLog, testSuites: testSuites);
	}
	
	private static bool IsTestGroup(string config)
	{
		return config.StartsWith("{");
	}
	
	private static bool IsTestSuite(string config)
	{
		return config.ToType().IsInherited(SCR_AutotestSuiteBase);
	}
	
	private static bool IsTestCase(string config)
	{
		return config.ToType().IsInherited(SCR_AutotestCaseBase);
	}
	
	private static void ParseConfig(string config, out array<ref SCR_AutotestSuiteBase> testSuites)
	{
		Resource configHolder = Resource.Load(config);
		
		if (!configHolder.IsValid())
		{
			PrintFormat("Invalid resource path for autotest config: %1", config, level: LogLevel.ERROR);
			return;
		}
		
		SCR_AutotestGroup testGroup = SCR_AutotestGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(configHolder.GetResource().ToBaseContainer()));
		if (!testGroup)
		{
			PrintFormat("Specified resource is not of type SCR_AutotestGroup: %1", testGroup, level: LogLevel.ERROR);
			return;
		}
		
		testSuites = testGroup.GetSuites();
	}
	
	private static void ParseSuite(string config, out array<ref SCR_AutotestSuiteBase> testSuites)
	{
		PrintFormat("CLI autotest suite: %1", config, level: LogLevel.NORMAL);
		SCR_AutotestSuiteBase testSuite = SCR_AutotestSuiteBase.Cast(config.ToType().Spawn());
		testSuites = {testSuite};
	}
	
	private static void ParseCases(string config, out array<ref SCR_AutotestSuiteBase> testSuites, out array<typename> testCases)
	{
		PrintFormat("CLI autotest case: %1", config, level: LogLevel.NORMAL);
		testCases = {config.ToType()};
		testSuites = GetCaseParentSuites({config.ToType()});
	}
	
	//! Used by the Bootstrap to instantiate individual test cases
	//! The framework requires test suites to execute tests, so this method helps connect cases to suites
	private static array<ref SCR_AutotestSuiteBase> GetCaseParentSuites(array<typename> testTypes)
	{
		set<typename> suiteTypes = new set<typename>;
		array<ref SCR_AutotestSuiteBase> suites = {};
		
		foreach (typename testType : testTypes)
		{
			Test testAttr = SCR_AutotestCaseBase.GetTestAttribute(testType);
			if (!testAttr)
			{
				Debug.Error(string.Format("Unable to run test case %1, no Test attribute", testType));
				continue;
			}
			
			typename suiteType = testAttr.Suite;
			if (!suiteType.IsInherited(SCR_AutotestSuiteBase))
			{
				Debug.Error(string.Format("Unable to run test case %1, no valid SCR_AutotestCaseBase suite assigned", testType));
				continue;
			}
			
			if (!suiteTypes.Contains(suiteType))
			{
				suiteTypes.Insert(suiteType);
				suites.Insert(SCR_AutotestSuiteBase.Cast(suiteType.Spawn()));
			}
		}
		
		return suites;
	}

	//------------------------------------------------------------------------------------------------
	//! Prints test suites and their test case state.
	static void DebugPrintSuites()
	{
		Print("(SCR_AutotestHarness) Tests to run:", level: LogLevel.DEBUG);
		foreach (SCR_AutotestSuiteBase suite : s_aCurrentSuites)
		{
			PrintFormat("\t%1: %2", suite.GetName(), suite.IsEnabled(), level: LogLevel.DEBUG);

			foreach (SCR_AutotestCaseBase test : suite.GetTestCaseInstances())
			{
				PrintFormat("\t\t%1: %2", test.GetName(), test.IsEnabled(), level: LogLevel.DEBUG);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize structures needed for the test run and ensure world is loaded.
	private static void BeginInternal(bool autorun, bool verboseLog, array<ref SCR_AutotestSuiteBase> testSuites)
	{
		s_aCurrentSuites = testSuites;
		DebugPrintSuites();
		
		array<ref TestSuite> beginTestSuites = {};
		foreach (TestSuite suite : testSuites)
		{
			beginTestSuites.Insert(suite);
		}
		super.Begin(beginTestSuites);
		
		// signal to SCR_AutotestRunnerCore that it should execute the TestHarness.Run() on every tick
		s_bIsRunning = autorun;
		
		s_Report = new SCR_AutotestReport();
		s_Logger = new SCR_AutotestPrinter(verbose: verboseLog);
	}
	
	//------------------------------------------------------------------------------------------------
	// Build report with curernt state of the test run for easy resume in case of a crash.
	static SCR_AutotestReport Checkpoint()
	{
		SCR_AutotestReport report = new SCR_AutotestReport();
		report.CollectResults();
		
		return report;
	}

	//------------------------------------------------------------------------------------------------
	//! Signal that the test runner is finished. Does any necessary cleanup.
	static SCR_AutotestReport Finish(bool abort = false)
	{
		if (abort)
			s_Logger.Log("Test run was aborted by going back to editor mode...", forceFileWrite: true);
		
		super.End();

		// signal to SCR_AutotestRunnerCore to stop
		s_bIsRunning = false;
		delete s_Logger;
		_SetActiveTestCase(null);

		s_Report.CollectResults();
		
#ifdef ENABLE_DIAG
		SCR_AutotestDebugMenu.GetInstance().Terminate();
#endif
		
		// abort happens mid transition back to editor mode
		// doing things like opening the report dialog will deadlock the UI
		if (abort)
			return s_Report;

#ifdef WORKBENCH
		if (SCR_AutotestHarness.s_bOpenLogAfterRun)
		{
			ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
			scriptEditor.SetOpenedResource(SCR_AutotestPrinter.LOG_PATH);
			SCR_AutotestHarness.s_bOpenLogAfterRun = false;
		}

		if (SCR_AutotestHarness.s_bOpenDialogAfterRun)
		{
			s_Report.OpenDialog();
			SCR_AutotestHarness.s_bOpenDialogAfterRun = false;
		}
#endif

		return s_Report;
	}

	//------------------------------------------------------------------------------------------------
	//! Internal function. Sets active test case.
	static void _SetActiveTestCase(TestBase testCase)
	{
		s_ActiveTestCase = testCase;
	}

	//------------------------------------------------------------------------------------------------
	//! Currently executed test case.
	static TestBase ActiveTestCase()
	{
		return s_ActiveTestCase;
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the test runner to run only specified SCR_AutotestSuite suite classes.
	private static void ConfigureTestSuites(notnull array<ref SCR_AutotestSuiteBase> wantedSuites, array<typename> specificCases = null, set<typename> brokenCases = null)
	{
		// if there's are test cases to skip we will skipping cases till we reach last one
		bool checkpointSearch = brokenCases != null;
		
		foreach (SCR_AutotestSuiteBase wantedSuite : wantedSuites)
		{
			foreach (SCR_AutotestCaseBase test : wantedSuite.GetTestCases(specificCases))
			{
				wantedSuite.AddTest(test);
				OnTestAdded(test);
				
				// TODO nested ifs, checkpoint restore method?
				if (checkpointSearch)
				{					
					if (brokenCases.Contains(test.Type()))
					{
						PrintFormat("Marking test as broken: %1", test, level: LogLevel.NORMAL);
						
						brokenCases.RemoveItem(test.Type());
						checkpointSearch = !brokenCases.IsEmpty();
						
						test.MarkCrashing();
					}
					else
					{
						PrintFormat("Skipping test: %1", test, level: LogLevel.NORMAL);

						// TODO disabling is better than not adding the test at all
						// JUnit generation with ommitSkipped = false could report these skipped tests.
						//
						// The only issue right now is that not yet executed tests will show as passing in JUnit due to SetFailure refactor,
						// making mid run generated JUnit not that useful.
						test.SetEnabled(false);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	private static void OnTestAdded(TestBase test)
	{
#ifdef ENABLE_DIAG
		SCR_AutotestDebugMenu.GetInstance().s_aAllTests.Insert(test.Type());
#endif
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AutotestPrinter GetLogger()
	{
		return s_Logger;
	}
}
