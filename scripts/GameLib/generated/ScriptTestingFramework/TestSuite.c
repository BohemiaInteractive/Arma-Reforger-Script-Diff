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
	//! Sets the suite result. Failure can result in specialized behavior described
	//! in TestResultBase.
	proto external void SetResult(TestResultBase res);
	//! Enables/Disables the suites. Disabled suites won't run at all.
	proto external void SetEnabled(bool val);
	//! Enabled flag getter.
	proto external bool IsEnabled();
	//! Returns the number for tests within this suite.
	proto external int GetNTests();
	//! Returns a test.
	proto external TestBase GetTest(int handle);
	//! Suite class name getter. Strictly for UI porposes!
	proto external string GetName();
	proto external void AddTest(notnull TestBase test);
}

/*!
\}
*/
