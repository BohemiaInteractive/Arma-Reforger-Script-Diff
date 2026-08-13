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
	//! Test name getter that can be used to customize name in test reports and UI.
	string GetName() { return this.ClassName(); }
	//! \deprecated Use SetFailure() instead.
	[Obsolete("Use SetFailure() instead.")]
	void SetResult(TestResultBase res)
	{
		if (res.Failure())
			SetFailure(res);
	}
	//! \deprecated Use GetFailure() instead.
	[Obsolete("Use GetFailure() instead.")]
	TestResultBase GetResult() { return null; }

	proto external void SetFailure(TestFailureBase failure);
	proto external TestFailureBase GetFailure();
	//! Enables/Disables the test. Disabled tests won't run at all.
	proto external void SetEnabled(bool val);
	//! Enabled flag getter.
	proto external bool IsEnabled();
}

/*!
\}
*/
