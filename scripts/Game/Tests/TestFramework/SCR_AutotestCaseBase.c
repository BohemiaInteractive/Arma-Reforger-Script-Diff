/*!
Base game test class.
Provides integration with test specific logger for improved output.
*/
class SCR_AutotestCaseBase : TestBase
{
	//------------------------------------------------------------------------------------------------
	//! Asserts that a boolean expression is true.
	//! Will set the test as failed with provided message otherwise.
	//! If the test is already failed the current result will be used.
	TestResultBase AssertTrue(bool expression, string msg)
	{
		TestResultBase result = GetResult();
		if (result && result.Failure())
			return result;

		if (expression)
			result = SCR_AutotestResult.AsSuccess();
		else
			result = SCR_AutotestResult.AsFailure(msg);

		SetResult(result);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get suite that owns the test.
	TestSuite GetSuite()
	{
		for (int i, countSuites = TestHarness.GetNSuites(); i < countSuites; i++)
		{
			TestSuite suite = TestHarness.GetSuite(i);
			for (int j, countTests = suite.GetNTests(); j < countTests; j++)
			{
				TestBase test = suite.GetTest(j);
				if (test.Type() == this.Type())
					return suite;
			}
		}

		return null;
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
	[Step(EStage.Setup)]
	private void Setup_Logger()
	{
		SCR_AutotestHarness._SetActiveTestCase(this);
	}

	//------------------------------------------------------------------------------------------------
	[Step(EStage.TearDown)]
	private void TearDown_PrintTestResult()
	{
		SCR_AutotestHarness.GetLogger().LogTestCaseResult(this);
	}

	//------------------------------------------------------------------------------------------------
	[Step(EStage.TearDown)]
	private void TearDown_Logger()
	{
		SCR_AutotestHarness._SetActiveTestCase(null);
	}
}
