#ifdef WORKBENCH

//! Mock feature class for the purpose of the Example1TestSuite
sealed class SCR_Example1Subject
{
	// method with broken implementation
	int GetFive()
	{
		return 4;
	}
}

//[BaseContainerProps(category: "Autotest")]
class SCR_TEST_Example1SubjectSuite : SCR_AutotestSuiteBase
{
	override ResourceName GetWorldFile()
	{
		return SCR_AutotestHelper.WORLD_MPTEST;
	}
}

[Test(suite: "SCR_TEST_Example1SubjectSuite")]
class SCR_TEST_Example1Subject_Counter_CountsToFive : SCR_AutotestCaseBase
{
	int m_iCounter;
	// Class instance will not be deleted between test runs.
	// Any class properties must be initialized in Setup stages.
	// TODO should we change that behaviour? I feel like this is a bit unexpected.
	int m_iExecutedTimes = 0;

	[Step(EStage.Setup)]
	void Setup()
	{
		m_iCounter = 0;
	}

	[Step(EStage.Main)]
	bool Execute()
	{
		PrintFormat("Execute: %1, %2", GetName(), m_iCounter.ToString());
		m_iCounter += 1;

		if (m_iCounter <= 5)
		{
			return false; // keep running
		}

		SetResult(SCR_AutotestResult.AsSuccess());
		return true; // finish the test
	}

	[Step(EStage.TearDown)]
	void TearDown()
	{
		m_iExecutedTimes++;
		PrintFormat("TearDown: %1, was executed %2 times since game start", GetName(), m_iExecutedTimes.ToString());
	}
}

[Test(suite: "SCR_TEST_Example1SubjectSuite")]
class SCR_TEST_Example1Subject_GetFive_ReturnsFive : SCR_AutotestCaseBase
{
	[Step(EStage.Main)]
	bool Execute()
	{
		SCR_Example1Subject subject = new SCR_Example1Subject();
		int result = subject.GetFive();

		Print("Lets see if GetFive() returns five...");

		if (result == 5)
		{
			SetResult(SCR_AutotestResult.AsSuccess());
		}
		else
		{
			SetResult(SCR_AutotestResult.AsFailure("Expected 5, got %1", result.ToString()));
		}

		// AssertTrue can be used instead of above construct for more concise code.
		// if the test is already failed the result will not be overwritten.
		AssertTrue(result == 5, "Result is five");

		return true;
	}
}

[Test(suite: "SCR_TEST_Example1SubjectSuite", timeoutMs: 10)]
class SCR_TEST_Example1Subject_TimeoutExample : SCR_AutotestCaseBase
{
	[Step(EStage.Main)]
	bool Execute()
	{
		PrintOnce("Waiting for timeout");
		return false;
	}
}

[Test(suite: "SCR_TEST_Example1SubjectSuite")]
class SCR_TEST_Example1Subject_NoResultExample : SCR_AutotestCaseBase
{
	[Step(EStage.Main)]
	bool Execute()
	{
		return true;
	}
}

// This is a functional test, these are NOT supported in the framework, the "Run test" plugin does not work with them.
// Also they will need manual usage of the SCR_AutotestPrinter for logging.
// DO NOT USE FUNCTIONAL TESTS
[Test(suite: "SCR_TEST_Example1SubjectSuite")]
TestResultBase SCR_TEST_Example1Subject_GetFive_ValidReturn()
{
	SCR_Example1Subject subject = new SCR_Example1Subject();
	int result = subject.GetFive();

	if (result == 5)
	{
		return SCR_AutotestResult.AsSuccess();
	}
	else
	{
		return SCR_AutotestResult.AsFailure("Expected 5, got %1", result.ToString());
	}
}

#endif
