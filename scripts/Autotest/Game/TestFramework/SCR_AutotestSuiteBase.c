/*!
Collection of game tests.
Ensures the world is loaded before tests will run.
Provides integration with test specific logger for improved output.
*/
[BaseContainerProps(category: "Autotest")]
class SCR_AutotestSuiteBase : TestSuite
{
	ref array<ref SCR_AutotestCaseBase> m_aTests = {};

	//! Returns an array of ALL test cases that belong to this test suite
	//! Internally calls GetTestCases(null)
	[Friend(SCR_AutotestHarness)]
	protected array<ref SCR_AutotestCaseBase> GetAllTestCases()
	{
		return GetTestCases(null);
	}
	
	//! Returns an array of test cases such that 
	//! they belong to this test suite AND are present in the [specifiedTests] array
	//! Returns ALL test cases if the [specifiedTests] is NULL. 
	[Friend(SCR_AutotestHarness)]
	protected array<ref SCR_AutotestCaseBase> GetTestCases(array<typename> specificTests)
	{
		array<typename> testTypes = {};
		TestHarness.GetTestTypes(testTypes);
		
		array<ref SCR_AutotestCaseBase> tests = {};

		array<ref SCR_AutotestParamData> suiteParams = CreateParams();
	
		foreach(typename testType : testTypes)
		{
			Test testAttr = SCR_AutotestCaseBase.GetTestAttribute(testType);
			if (!testAttr)
				continue;
	
			typename suiteType = testAttr.Suite;
			if (!suiteType)
				continue;
			
			if (suiteType != Type())
				continue;
			
			if (specificTests && !specificTests.Contains(testType))
				continue;
			
			bool paramsValid = suiteParams && !suiteParams.IsEmpty();
			if (SCR_AutotestCaseBase.IsParameterized(testType) && paramsValid)
			{
				foreach (int idx, SCR_AutotestParamData suiteParam : suiteParams)
				{
					SCR_AutotestCaseBase testWithParams = SCR_AutotestCaseBase.Cast(testType.Spawn());
					testWithParams.SetParamsInternal(suiteParam.WithIdx(idx));
					tests.Insert(testWithParams);
				}
			}
			else
			{
				SCR_AutotestCaseBase test = SCR_AutotestCaseBase.Cast(testType.Spawn());
				tests.Insert(test);
			}
		}
		
		return tests;
	}
	
	//! This method must return an array of container classes that inherit from SCR_AutotestParamData. 
	//! Each container is one set of parameters to run your tests with.
	array<ref SCR_AutotestParamData> CreateParams();
	
	// TODO(maciejewskifil) refactor
	[Friend(SCR_AutotestHarness)]
	protected void AddTest(notnull SCR_AutotestCaseBase test)
	{
		test.SetSuite(this);
		m_aTests.Insert(test);
		super.AddTest(test);
	}
	
	// TODO(maciejewskifil) refactor
	array<ref SCR_AutotestCaseBase> GetTestCaseInstances()
	{
		return m_aTests;
	}

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
	[TestStep(TestStage.Setup)]
	private void Setup_PrintPrelude()
	{
		SCR_AutotestHarness.GetLogger().PrintTestSuitePrelude(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Open world requested by this test suite.
	[TestStep(TestStage.Setup)]
	private void Setup_OpenWorld()
	{
		ResourceName world = GetWorldFile();
		ResourceName systemsConfig = GetWorldSystemsConfigFile();
		if (world && !SCR_AutotestHelper.WorldOpenFile(world, systemsConfig))
		{
			string failure = string.Format("Failed to load world: %1, %2", world, systemsConfig);
			SCR_AutotestHarness.GetLogger().Log(failure, level: LogLevel.ERROR);

			SetFailure(SCR_AutotestFailure.Create(failure));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Waits for the world to load.
	[TestStep(TestStage.Setup)]
	private bool Setup_AwaitWorld()
	{
		return !GameStateTransitions.IsTransitionRequestedOrInProgress();
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus that could interfere with the test suite.
	[TestStep(TestStage.Setup)]
	private void Setup_CloseMenus()
	{
		GetGame().GetMenuManager().CloseAllMenus();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set time multiplier.
	[TestStep(TestStage.Setup)]
	private void Setup_SetTimeMultiplier()
	{
		SCR_AutotestHelper.WorldSetTimeCoef(SCR_AutotestHarness.GetWorldTimeCoef());
	}

	//------------------------------------------------------------------------------------------------
	//! Reset time multiplier.
	[TestStep(TestStage.TearDown)]
	private void TearDown_ResetTimeMultiplier()
	{
		SCR_AutotestHelper.WorldSetTimeCoef(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Log "closing" part of the test suite output.
	[TestStep(TestStage.TearDown)]
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
