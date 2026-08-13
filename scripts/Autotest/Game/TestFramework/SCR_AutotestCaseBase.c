/*!
Base game test class.
Provides integration with test specific logger for improved output.
*/
class SCR_AutotestCaseBase : TestBase
{
	private SCR_AutotestSuiteBase m_OwningSuite;
	private ref SCR_AutotestParamData m_ParamDataBase;
	
	private bool m_bIsCrashing;
	
	//------------------------------------------------------------------------------------------------
	//! Override this method in combination with adding a field to your class.
	//! The field must inherit from SCR_AutotestParamData.
	//! This method must cast 'params' to the correct class and assign it to that field.
	void SetParams(SCR_AutotestParamData params);
	
	//------------------------------------------------------------------------------------------------
	static Test GetTestAttribute(typename testType)
	{
		array<Class> attributes = {};
		testType.GetAttributes(attributes);
		Test testAttr;
		foreach (Class attr : attributes)
		{
			testAttr = Test.Cast(attr);
			if (testAttr)
			{
				break;
			}
		}
		
		return testAttr;
	}
	
	//------------------------------------------------------------------------------------------------
	sealed static bool IsParameterized(typename testType)
	{
		return GetTestAttribute(testType).IsInherited(SCR_ParamTest);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		if (IsParameterized(this.Type()))
			return string.Format("%1_###_%2", super.GetName(), GetParamReportIdentifier());

		return super.GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns a string that will be appended to the test name in the report after '#'.
	//! Override this to return distinct names for your parameterized tests.
	string GetParamReportIdentifier()
	{
		if (!m_ParamDataBase)
			return "<missing params>";
		
		return m_ParamDataBase.GetIdentifier();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Asserts that a boolean expression is true.
	//! Will set the test as failed with provided message otherwise.
	//! If the test is already failed the current result will be used.
	TestFailureBase AssertTrue(bool expression, string msg)
	{
		TestFailureBase failure = GetFailure();
		if (failure)
			return failure;

		if (!expression)
		{
			failure = SCR_AutotestFailure.Create("AssertTrue: %1", msg);
			SetFailure(failure);
		}

		return failure;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set failure reason of the test. Won't override already set reason.
	//! \param[in] reason Reason for the failure, supports string interpolation.
	//! \param[in] param1 String param
	//! \param[in] param2 String param
	//! \param[in] param3 String param
	//! \return Failure object
	notnull TestFailureBase SetFailure(string reason, string param1 = "", string param2 = "", string param3 = "")
	{
		TestFailureBase failure = GetFailure();
		if (failure)
			return failure;
		
		failure = SCR_AutotestFailure.Create(reason, param1, param2, param3);
		SetFailure(failure);

		return failure;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_AutotestSuiteBase)]
	protected void SetSuite(SCR_AutotestSuiteBase suite)
	{
		m_OwningSuite = suite;
	}
	
	//------------------------------------------------------------------------------------------------
	[Friend(SCR_AutotestSuiteBase)]
	protected void SetParamsInternal(SCR_AutotestParamData param)
	{
		m_ParamDataBase = param;
		SetParams(param);
	}
	
	//------------------------------------------------------------------------------------------------
	[Friend(SCR_AutotestHarness)]
	protected void MarkCrashing()
	{
		m_bIsCrashing = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get suite that owns the test.
	SCR_AutotestSuiteBase GetSuite()
	{
		return m_OwningSuite;
	}

	//------------------------------------------------------------------------------------------------
	//! Get additional debug lines for diag menu
	array<string> GetDebugText()
	{
		return {};
	}

	//------------------------------------------------------------------------------------------------
	//! Prints content of variable to console/log and autotest/log.
	//! Shadows global Print to force the logs to go through SCR_AutotestPrinter.
	void Print(string msg, LogLevel level = LogLevel.NORMAL)
	{
		SCR_AutotestHarness.GetLogger().Log("\t"+msg, level);
	}

	//------------------------------------------------------------------------------------------------
	//! Prints formated text to console/log and autotest/log.
	//! Shadows global PrintFormat to force the logs to go through SCR_AutotestPrinter.
	void PrintFormat(string fmt, string param1 = "", string param2 = "", string param3 = "", LogLevel level = LogLevel.NORMAL)
	{
		SCR_AutotestHarness.GetLogger().Log(string.Format("\t"+fmt, param1, param2, param3), level);
	}

	//------------------------------------------------------------------------------------------------
	//! Prints content of variable to console/log and autotest/log.
	//! Repeated prints of the same message will be replaced with "collected" variant indicating how many times it was printed.
	void PrintOnce(string msg, LogLevel level = LogLevel.NORMAL)
	{
		SCR_AutotestHarness.GetLogger().LogOnce("\t"+msg, level);
	}

	//------------------------------------------------------------------------------------------------
	[TestStep(TestStage.Setup)]
	private void Setup_Logger()
	{
		SCR_AutotestHarness._SetActiveTestCase(this);
	}
	
	//------------------------------------------------------------------------------------------------
	[TestStep(TestStage.Setup, timeoutMs: 500)]
	private void Setup_Checkpoint()
	{
		SCR_AutotestReport report = SCR_AutotestHarness.Checkpoint();
		report.WriteLogFiles();
	}
	
	[TestStep(TestStage.Setup)]
	private void Setup_ResetLibraries()
	{
		SCR_Timer.ClearAll();
		SCR_TimerInternal.ClearAll();
		SCR_TestLib.DeleteObjects();
	}

	[TestStep(TestStage.Setup)]
	void Setup_VerifyParamsNotEmpty()
	{
		if (!IsParameterized(this.Type()))
			return;
		
		if (m_ParamDataBase == null)
			SetFailure("Parameters not set on a parameterized test case");
	}
	
	[TestStep(TestStage.Setup)]
	void Setup_VerifyNotCrashing()
	{
		// TODO we could use some custom failure type for distinct output in XML for Autotest Web app parsing
		AssertTrue(m_bIsCrashing == false, "Expected test to not be marked as crashing");
	}

	//------------------------------------------------------------------------------------------------
	[TestStep(TestStage.TearDown)]
	private void TearDown_PrintTestResult()
	{
		SCR_AutotestHarness.GetLogger().LogTestCaseResult(this);
	}

	//------------------------------------------------------------------------------------------------
	[TestStep(TestStage.TearDown)]
	private void TearDown_Logger()
	{
		SCR_AutotestHarness._SetActiveTestCase(null);
	}
}
