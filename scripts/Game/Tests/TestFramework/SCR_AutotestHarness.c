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

	static bool s_bIsRunning = false;
	static bool s_bOpenLogAfterRun = false;
	static bool s_bOpenDialogAfterRun = false;
	static bool s_bCloseGameAfterRun = true;

	//------------------------------------------------------------------------------------------------
	//! Prints test suites and their test case state.
	static void DebugPrintSuites()
	{
		Print("(SCR_AutotestHarness) Tests to run:", level: LogLevel.DEBUG);
		int suitesCount = SCR_AutotestHarness.GetNSuites();
		for (int i = 0; i < suitesCount; i++)
		{
			TestSuite suite = SCR_AutotestHarness.GetSuite(i);
			PrintFormat("\t%1: %2", suite.GetName(), suite.IsEnabled(), level: LogLevel.DEBUG);

			int testsCount = suite.GetNTests();
			for (int j = 0; j < testsCount; j++)
			{
				TestBase test = suite.GetTest(j);
				PrintFormat("\t\t%1: %2", test.GetName(), test.IsEnabled(), level: LogLevel.DEBUG);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the test runner to execute only the test suites present in the specified test group and initializes required structures.
	static void Begin(SCR_AutotestGroup testGroup, bool autorun = false, bool verboseLog = false)
	{
		if (s_bIsRunning)
			Print("SCR_AutotestHarness::Begin was called while the test is already running", LogLevel.ERROR);

		if (!testGroup)
			testGroup = GetDefaultTestGroupConfig();

		ConfigureTestGroup(testGroup);
		BeginInternal(autorun, verboseLog);
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the test runner to execute single test suite and initializes required structures.
	static void Begin(notnull SCR_AutotestSuiteBase testSuite, bool autorun = false, bool verboseLog = false)
	{
		if (s_bIsRunning)
			Print("SCR_AutotestHarness::Begin was called while the test is already running", LogLevel.ERROR);

		array<ref SCR_AutotestSuiteBase> wantedSuites = {testSuite};
		ConfigureTestSuites(wantedSuites);

		BeginInternal(autorun, verboseLog);
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the test runner to execute single test case and initializes required structures.
	static void Begin(notnull SCR_AutotestCaseBase testCase, bool autorun = false, bool verboseLog = false)
	{
		if (s_bIsRunning)
			Print("SCR_AutotestHarness::Begin was called while the test is already running", LogLevel.ERROR);

		ConfigureTestCases({testCase});

		BeginInternal(autorun, verboseLog);
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize structures needed for the test run and ensure world is loaded.
	private static void BeginInternal(bool autorun, bool verboseLog)
	{
		DebugPrintSuites();

		super.Begin();

		// signal to SCR_AutotestRunnerCore that it should execute the TestHarness.Run() on every tick
		s_bIsRunning = autorun;
		s_Report = new SCR_AutotestReport();
		s_Logger = new SCR_AutotestPrinter(verbose: verboseLog);

		#ifdef WORKBENCH
		if (!GetGame().InPlayMode())
		{
			WorldEditor we = Workbench.GetModule(WorldEditor);
			if (!we.GetApi())
			{
				Print("WorldEditor is not open!", LogLevel.ERROR);
				return;
			}

			we.SwitchToGameMode();
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Signal that the test runner is finished. Does any necessary cleanup.
	static SCR_AutotestReport Finish()
	{
		super.End();

		// signal to SCR_AutotestRunnerCore to stop
		s_bIsRunning = false;
		delete s_Logger;

		s_Report.CollectResults();

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
	//! Configures the test runner to run only test suites in the specified group.
	private static void ConfigureTestGroup(notnull SCR_AutotestGroup testGroup)
	{
		ConfigureTestSuites(testGroup.GetEnabledSuites());
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the test runner to run only specified SCR_AutotestSuite suite classes.
	private static void ConfigureTestSuites(notnull array<ref SCR_AutotestSuiteBase> wantedSuites)
	{
		int allSuiteCount = GetNSuites();
		for (int i = 0; i < allSuiteCount; i++)
		{
			TestSuite suite = GetSuite(i);
			suite.SetEnabled(false);
			foreach (SCR_AutotestSuiteBase wantedSuite : wantedSuites)
			{
				// test suites from the test harness and config are instantiated separately, do the checks via ClassName
				if (suite.ClassName() == wantedSuite.ClassName())
				{
					int suiteTestsCount = suite.GetNTests();
					for (int j = 0; j < suiteTestsCount; j++)
					{
						TestBase test = suite.GetTest(j);
						test.SetEnabled(true);
					}
					suite.SetEnabled(true);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the test runner to run only specified SCR_AutotestCase test classes.
	private static void ConfigureTestCases(notnull array<ref SCR_AutotestCaseBase> wantedTests)
	{
		int allSuiteCount = GetNSuites();
		array<TestSuite> allTestSuites = {};
		for (int i = 0; i < allSuiteCount; i++)
		{
			TestSuite suite = GetSuite(i);
			suite.SetEnabled(false);
			allTestSuites.Insert(suite);
		}

		/*
			disables all test suites, then for each wanted testcase scans:
				test suite:
					all test cases:
						if wanted testcase is found, enable it and its parent test suite
		*/
		foreach (SCR_AutotestCaseBase wantedTest : wantedTests)
		{
			foreach (TestSuite suite : allTestSuites)
			{
				int suiteTestsCount = suite.GetNTests();
				bool testFoundInThisSuite = false;
				for (int i = 0; i < suiteTestsCount; i++)
				{
					TestBase test = suite.GetTest(i);
					// prevent tests from being run in case the test suite would be re-enabled
					test.SetEnabled(false);

					if (test.ClassName() == wantedTest.ClassName())
					{
						testFoundInThisSuite = true;
						test.SetEnabled(true);
						suite.SetEnabled(true);
					}
				}

				// no point in scanning further if we found our TestCase
				if (testFoundInThisSuite)
					break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AutotestPrinter GetLogger()
	{
		return s_Logger;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns default test group for the project.
	//! Should be overridden in project specific addon.
	static SCR_AutotestGroup GetDefaultTestGroupConfig()
	{
		/*
		Resource configHolder = BaseContainerTools.LoadContainer(CORE_CONFIG);
		return SCR_AutotestGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(configHolder.GetResource().ToBaseContainer()));
		*/
		return new SCR_AutotestGroup();
	}
}
