/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup ScriptTestingFramework
\{
*/

/*!
Base class for test failures. It is used to signal failure of a test and to
provide textual representation of failure information in JUnit format. For
details on JUnit format, you can refer to:
- https://llg.cubic.org/docs/junit/
- https://github.com/testmoapp/junitxml
- https://lotterfriends.github.io/online-junit-parser/
*/
class TestFailureBase: Managed
{
	/*!
	Returns text used as body of `testcase` element in JUnit XML.
	Usually this function generates `failure` element.
	*/
	string FailureText() { return NativeFailureText(); }

	proto external string NativeFailureText();
}

/*!
\}
*/
