/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup ScriptTestingFramework
\{
*/

//! Collection of tests. Provides API for environment preparation.
class TestSuite: Managed
{
	//! Suite name getter that can be used to customize name in test reports and UI.
	string GetName() { return this.ClassName(); }
	//! \deprecated Use SetFailure() instead.
	[Obsolete("Use SetFailure() instead.")]
	void SetResult(TestResultBase res)
	{
		if (res.Failure())
			SetFailure(res);
	}

	proto external void SetFailure(TestFailureBase failure);
	//! Enables/Disables the suites. Disabled suites won't run at all.
	proto external void SetEnabled(bool val);
	//! Enabled flag getter.
	proto external bool IsEnabled();
	//! Returns the number for tests within this suite.
	proto external int GetNTests();
	//! Returns a test.
	proto external TestBase GetTest(int handle);
	proto external void AddTest(notnull TestBase test);
}

/*!
\}
*/
