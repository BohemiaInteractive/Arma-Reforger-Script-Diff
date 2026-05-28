/*!
\defgroup ScriptTestingFramework Script Testing Framework
\addtogroup ScriptTestingFramework
\{
For framework description, see \ref Page_System_ScriptTestingFramework.
\}
*/

/*!
\page Page_System_ScriptTestingFramework Script Testing Framework
\tableofcontents


\section Introduction Introduction

- Provides a unified and simple interface that emphasizes the smallest amount
of boiler plate code possible.

- Each test is marked using Test attribute. A test is a type derived from
TestBase. It can run for several ticks, maintaining state using member
variables. Your logic has to be ran through __step methods__.

- Each test specifies to which TestSuite it belongs. Suites provide additional
API for controlling the execution environment (for all tests which are part of
a suite).

- Within the framework a SINGLE class derived from TestHarness can exist.
TestHarness is collection of all available test suites. It runs tests in them
and provides API to access them.

\subsection StepMethods Step methods
- You can name your step methods however you like.
- They have to be annotated with TestStep attribute that specifies TestStage to
which this step belongs.

\subsection Stages Stages
- They divide the steps into groups that express the initialization and
finalization process.
- Under normal circumstances (no failure or error occurs), stages are
executed in following order:
	1. TestStage.Setup
	2. TestStage.Main
	3. TestStage.TearDown
- Methods in stages are executed in order of definition.

\subsection ReturnValues Return values
- `void` -> Will get executed only once.
- `bool` -> Will get executed every tick until true is returned.

\subsection Result Result
- Result is set via TestBase.SetResult(TestResultBase).
- Setting a result which evaluates to failure will terminate the stage.

\subsection FailureUnwind Failure unwind
- If the TestStage.Setup fails the test only terminates and TestStage.TearDown
is not called.
- Failure during TestStage.Main will trigger the TestStage.TearDown.
- Failure during TestStage.TearDown will do nothing.

\subsection Timeout Timeout
- Tests won't timeout by default. The value may be specified via Test or TestStep
attributes.
- Test attribute specifies default timeout for steps of a test.
- Non-zero timeout specified in TestStep overrides timeout from Test.
- TestSuite steps may also have their
- The timeout timer resets for every step method.
- If the step method times out the TimeoutResult is set (it evaluates to
failure and the failure unwind process starts).

\snippet this Stateful test

*/

/*!
\addtogroup ScriptTestingFramework
\{
*/

//-----------------------------------------------------------------------------
//! Attribute used for tests annotation and assignment to Suites.
class Test
{
	typename Suite;

	int TimeoutS;
	int TimeoutMs;
	int MaxAttempts;

	//! Defines a suite the test belongs to and its timeout value.
	void Test(string suiteStr = "", typename suite = void, int timeoutS = 0, int timeoutMs = 0, int maxAttempts = 1)
	{
		if (suite != void)
		{
			Suite = suite;
		}
		else
		{
			PrintFormat(
				"Test referring to TestSuite using string '%1' instead of typename. This is deprecated and should be replaced by explicitly setting passing 'suite' parameter",
				suiteStr, level: LogLevel.WARNING
			);
			Suite = suiteStr.ToType();
		}
		TimeoutS = timeoutS;
		TimeoutMs = timeoutMs;
		MaxAttempts = maxAttempts;
	}
}

//-----------------------------------------------------------------------------
//! \deprecated Use TestStage instead.
typedef TestStage EStage;

//-----------------------------------------------------------------------------
//! Attribute which marks a method as part of the testing process.
class TestStep
{
	TestStage Stage;
	int TimeoutS;
	int TimeoutMs;

	void TestStep(TestStage stage = TestStage.Main, int timeoutS = 0, int timeoutMs = 0)
	{
		Stage = stage;
		TimeoutS = timeoutS;
		TimeoutMs = timeoutMs;
	}
}

//! \deprecated Use TestStep instead.
typedef TestStep Step;

/*!
\}
*/

#ifdef DOXYGEN

//-----------------------------------------------------------------------------
//! Basic test result.
class TestBoolResult : TestResultBase
{
	bool Value;

	void TestBoolResult(bool val) { Value = val; }

	override bool Failure() { return !Value; }

	override string FailureText()
	{
		// junit kind of error report. (simple)
		return "<failure type=\"TestBoolResult\" message=\"Failed.\" />";
	}
}

//-----------------------------------------------------------------------------
class MyTestSuite : TestSuite
{
	private int m_Count;

	[TestStep(TestStage.Setup)]
	void Prep()
	{
		Print("MyTestSuite.Prep");
		m_Count = 3;
	}

	[TestStep(TestStage.Setup)]
	bool Count()
	{
		--m_Count;
		Print("MyTestSuite.Count: " + m_Count);
		return m_Count == 0;
	}

	[TestStep(TestStage.TearDown)]
	bool CountUp()
	{
		++m_Count;
		Print("MyTestSuite.CountUp: " + m_Count);
		return m_Count == 10;
	}
}

//-----------------------------------------------------------------------------
//! [Stateful test]
[Test(suite: MyTestSuite, timeoutS: 2, timeoutMs: 250)]
class MyAsyncTest : TestBase
{
	private int m_Count;

	// Simple blocking initialization.
	[TestStep(TestStage.Setup)]
	void Init()
	{
		Print("MyAsyncTest.Init");
		m_Count = 10;
	}

	// Async test which is waiting for result for several frames.
	[TestStep(TestStage.Main)]
	bool Poll()
	{
		Print("MyAsyncTest.Poll: " + m_Count);

		if (m_Count == 0)
		{
			Print("MyAsyncTest.Poll Result");
			SetResult(new TestBoolResult(false));
			m_Count = 3;
			return true;
		}

		Print("MyAsyncTest.Poll Progress");

		m_Count--;
		return false;
	}

	// Finalization process waiting for result for several frames.
	[TestStep(TestStage.TearDown)]
	bool Finalizing()
	{
		Print("MyAsyncTest.Finalizing: " + m_Count);
		if (m_Count == 0)
		{
			Print("MyAsyncTest.Finalizing Done");
			return true;
		}

		Print("MyAsyncTest.Finalizing Progress");
		m_Count--;
		return false;
	}

	// Simple blocking finalization call.
	[TestStep(TestStage.TearDown)]
	void Finalized()
	{
		Print("MyAsyncTest.Finalized");
	}
}
//! [Stateful test]

#endif
