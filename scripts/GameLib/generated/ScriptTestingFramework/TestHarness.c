/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup ScriptTestingFramework
\{
*/

//! Collection of suites and main interface of the Testing framework.
class TestHarness: Managed
{
	//! Returns number of default test suites.
	[Obsolete("Switch to instantiating tests and suites manually.")]
	static proto int GetNSuites();
	//! Returns default test suite.
	[Obsolete("Switch to instantiating tests and suites manually.")]
	static proto TestSuite GetSuite(int handle);
	//! Returns currently active TestSuite or null when none is active.
	static proto TestSuite ActiveSuite();
	//! Returns true when all tests and suites finished.
	static proto bool Finished();
	//! Starts up the testing process and initializes the structures.
	static proto void Begin(array<ref TestSuite> suites = null);
	//! Starts the testing process. Returns true when all tests have finished. If
	//! some of them are still in progress false is reported.
	static proto bool Run();
	//! Finalizes the testing process.
	static proto void End();
	//! Generates a xml report.
	static proto string Report(bool omitSkipped = false);
	/*!
	Replaces characters with special meaning in XML so that result can be used in
	XML attribute value or XML element body.

	Replacement is done as follows:
	| special character | escape sequence |
	| ----------------- | --------------- |
	| <                 | `&lt;`          |
	| >                 | `&gt;`          |
	| &                 | `&amp;`         |
	| '                 | `&apos;`        |
	| "                 | `&quot;`        |
	*/
	static proto string EscapeForXml(string text);
	//! Obtain all types derived from TestBase which are decorated with Test attribute.
	static proto void GetTestTypes(notnull out array<typename> outTypes);
}

/*!
\}
*/
