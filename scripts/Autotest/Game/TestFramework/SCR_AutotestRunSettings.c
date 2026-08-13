//! Settings container consumed by the SCR_TestRunner to bootstrap the TestHarness and initialize test execution
class SCR_AutotestRunSettings
{	
	string m_sConfig;

	int m_iTimeCoefficient;
	bool m_bOpenLogAfterRun;
	bool m_bOpenDialogAfterRun;
	SCR_EAutotestOnFinishedAction m_eActionAfterRun;
	bool m_bVerboseLog;
	ref set<typename> m_aSkippedCases;
	
	[Friend(SCR_AutotestRunSettingsBuilder)]
	protected void SCR_AutotestRunSettings(string config)
	{
		this.m_sConfig = config;
	}
};

class SCR_AutotestRunSettingsBuilder
{
	private ref SCR_AutotestRunSettings m_Instance;
		
	//! Initializes a container with the given config and all other parameters = false;
	static SCR_AutotestRunSettingsBuilder CreateContainer(string config)
	{
		SCR_AutotestRunSettingsBuilder builder = new SCR_AutotestRunSettingsBuilder();
		builder.m_Instance = new SCR_AutotestRunSettings(config);			

		builder
			.WithTimeCoefficient(1)
			.WithLogAfterRun(false)
			.WithDialogAfterRun(false)
			.WithActionAfterRun(SCR_EAutotestOnFinishedAction.EXIT)
			.WithVerboseLog(false);
				
		return builder;
	}
	
	//! Builds AutotestRunSettings container from CLI parameters if -autotest is specified
	static SCR_AutotestRunSettings CreateFromCLI()
	{
		string autotestConfigCLI;
		System.GetCLIParam("autotest", autotestConfigCLI);

		if (!autotestConfigCLI)
		{
			Debug.Error("Empty -autotest parameter value");
			return null;
		}
		
		SCR_AutotestRunSettingsBuilder builder = CreateContainer(autotestConfigCLI);
		ProcessCLISettings(builder);
		return builder.Build();
	}

	private static SCR_AutotestRunSettings CreateFromString(string cmd)
	{
		// TODO: implement with the support of dev console test runs
		// TODO: remove private when done
	}
	
	// TODO: add other harness settings as CLI params
	private static void ProcessCLISettings(SCR_AutotestRunSettingsBuilder builder)
	{
		if (System.IsCLIParam("autotestTimeCoef"))
		{
			string autotestTimeCoefCLI;
			System.GetCLIParam("autotestTimeCoef", autotestTimeCoefCLI);
			
			int autotestTimeCoef = autotestTimeCoefCLI.ToInt();
			builder.WithTimeCoefficient(autotestTimeCoef);
		}
		
		if (System.IsCLIParam("autotestBrokenCases"))
		{
			string autotestSkippedCasesCLI;
			System.GetCLIParam("autotestBrokenCases", autotestSkippedCasesCLI);
			
			array<string> autotestSkippedCases = {};
			autotestSkippedCasesCLI.Split(",", autotestSkippedCases, true);
			
			foreach (string testCase : autotestSkippedCases)
			{
				typename testCaseType = testCase.ToType();
				if (testCaseType == typename.Empty || !testCaseType.IsInherited(SCR_AutotestCaseBase))
					PrintFormat("Invalid 'autotestBrokenCases' value: %1", testCase, level: LogLevel.WARNING);
				
				builder.WithSkippedTestCase(testCaseType);
			}
		}
	}
	
	//! Set the time coefficient ( >= 0) to be used in-game for the test run
	//! This could be used to speed up test execution
	SCR_AutotestRunSettingsBuilder WithTimeCoefficient(int value)
	{
		if (value <= 0)
			PrintFormat("Invalid 'autotestTimeCoef' value: %1", value, level: LogLevel.WARNING);
		else
			m_Instance.m_iTimeCoefficient = value;
		
		return this;
	}
	
	//! Set if the log should be opened after the test run
	SCR_AutotestRunSettingsBuilder WithLogAfterRun(bool value)
	{
		m_Instance.m_bOpenLogAfterRun = value;
		return this;
	}
	
	//! Set if the test results dialogue should be opened after the test run
	SCR_AutotestRunSettingsBuilder WithDialogAfterRun(bool value)
	{
		m_Instance.m_bOpenDialogAfterRun = value;
		return this;
	}
	
	//! Set the action to be executed after the test run [Exit, Exit if no error, or None]
	SCR_AutotestRunSettingsBuilder WithActionAfterRun(SCR_EAutotestOnFinishedAction value)
	{
		m_Instance.m_eActionAfterRun = value;
		return this;
	}
	
	//! Set if the log should be verbose
	SCR_AutotestRunSettingsBuilder WithVerboseLog(bool value)
	{
		m_Instance.m_bVerboseLog = value;
		return this;
	}
	
	//! Set test cases to be skipped
	SCR_AutotestRunSettingsBuilder WithSkippedTestCase(typename testCaseTypename)
	{
		if (!m_Instance.m_aSkippedCases)
			m_Instance.m_aSkippedCases = new set<typename>();

		m_Instance.m_aSkippedCases.Insert(testCaseTypename);

		return this;
	}
	
	ref SCR_AutotestRunSettings Build()
	{
		SCR_AutotestRunSettings instance = m_Instance;
		m_Instance = null;
		return instance;
	}
};