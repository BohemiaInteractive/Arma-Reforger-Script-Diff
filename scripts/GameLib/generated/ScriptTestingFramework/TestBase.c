/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup ScriptTestingFramework
\{
*/

//! Test base class.
class TestBase: Managed
{
	//! Test name getter. Strictly for UI porposes!
	proto external string GetName();
	//! Sets the test result. Failure can result in specialized behavior described
	//! in TestResultBase.
	proto external void SetResult(TestResultBase res);
	//! Result getter.
	proto external TestResultBase GetResult();
	//! Enables/Disables the test. Disabled tests won't run at all.
	proto external void SetEnabled(bool val);
	//! Enabled flag getter.
	proto external bool IsEnabled();
}

/*!
\}
*/
