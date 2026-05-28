/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup ScriptTestingFramework
\{
*/

//! Base class for test results. This way you report back to the system.
//! More complex failure types with descriptions can be reported by
//! implementation of FailureText in format of junit (see
//! https://llg.cubic.org/docs/junit/ or https://github.com/testmoapp/junitxml).
class TestResultBase: Managed
{
	//! Return true if the result means failure.
	bool Failure() { return NativeFailure(); }
	//! Text used for xml report output.
	string FailureText() { return NativeFailureText(); }

	proto external bool NativeFailure();
	proto external string NativeFailureText();
}

/*!
\}
*/
