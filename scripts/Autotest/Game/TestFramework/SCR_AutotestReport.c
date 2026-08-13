
class SCR_AutotestReport
{
	protected static const string JUNIT_PATH = "$logs:/junit.xml";

	protected static const string FAILED_TESTS_LIST_PATH = "$logs:/autotest_failed.log";
	protected static const string SUCCEDED_TESTS_LIST_PATH = "$logs:/autotest_succeded.log";
	protected static const string CURRENT_TEST_NAME_PATH = "$logs:/autotest_current.log";

	protected static const string FAILED_SUITES_LIST_PATH = "$logs:/autotest_failed_suites.log";

	protected string m_sJUnitXml;
	protected bool m_bIsFailure;
	
	protected ref array<SCR_AutotestCaseBase> m_aFailedTests = {};
	protected ref array<SCR_AutotestCaseBase> m_aSuccededTests = {};
	protected ref array<SCR_AutotestSuiteBase> m_aFailedSuites = {};

#ifdef WORKBENCH
	protected string m_sDialogText;
#endif

	void CollectResults()
	{
		SetJUnit(TestHarness.Report(omitDisabled: true));
		FillStatusArrays();
#ifdef WORKBENCH
		FillDialog();
#endif
	}

	void WriteJUnitXML()
	{
		string path = JUNIT_PATH;
		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		file.Write(m_sJUnitXml);
		file.Close();

		PrintFormat("Autotest JUnit XML saved to: %1", path, level: LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Does report contain any failures?
	bool IsFailure()
	{
		return m_bIsFailure;
	}

	//------------------------------------------------------------------------------------------------
	//! Writes report status autotest_*.log files to profile directory.
	void WriteLogFiles()
	{
		WriteFailedTestsList();
		WriteSuccededTestsList();
		WriteCurrentTestName();
		
		WriteFailedSuitesList();
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteFailedTestsList()
	{
		const string path = FAILED_TESTS_LIST_PATH;
		FileIO.DeleteFile(path);
		// we do not want to create empty files
		if (m_aFailedTests.IsEmpty())
			return;

		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		foreach (SCR_AutotestCaseBase test : m_aFailedTests)
		{
			file.WriteLine(test.GetName());
		}

		file.Close();

		PrintFormat("Autotest failed tests list saved to: %1", path, level: LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void WriteSuccededTestsList()
	{
		const string path = SUCCEDED_TESTS_LIST_PATH;
		FileIO.DeleteFile(path);
		// we do not want to create empty files
		if (m_aSuccededTests.IsEmpty())
			return;

		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		foreach (SCR_AutotestCaseBase test : m_aSuccededTests)
		{
			file.WriteLine(test.GetName());
		}

		file.Close();

		PrintFormat("Autotest succeded tests list saved to: %1", path, level: LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	void WriteCurrentTestName()
	{
		const string path = CURRENT_TEST_NAME_PATH;
		FileIO.DeleteFile(path);

		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);

		TestBase activeTest = SCR_AutotestHarness.ActiveTestCase();
		if (activeTest)
			file.Write(activeTest.Type().ToString());

		file.Close();

		PrintFormat("Autotest Current Test saved to: %1", path, level: LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void WriteFailedSuitesList()
	{
		const string path = FAILED_SUITES_LIST_PATH;
		FileIO.DeleteFile(path);
		// we do not want to create empty files
		if (m_aFailedSuites.IsEmpty())
			return;

		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		foreach (SCR_AutotestSuiteBase suite : m_aFailedSuites)
		{
			file.WriteLine(suite.GetName());
		}

		file.Close();

		PrintFormat("Autotest failed tests list saved to: %1", path, level: LogLevel.NORMAL);
	}

	protected void SetJUnit(string xml)
	{
		m_sJUnitXml = xml;
	}

	protected void SetFailed()
	{
		m_bIsFailure = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void FillStatusArrays()
	{
		foreach (SCR_AutotestSuiteBase suite : SCR_AutotestHarness.s_aCurrentSuites)
		{
			// TODO missing GetFailure() API on TestSuite
			// handle suite setup failure
			
			foreach (SCR_AutotestCaseBase test : suite.GetTestCaseInstances())
			{
				TestFailureBase failure = test.GetFailure();
				if (failure)
				{
					SetFailed();
					m_aFailedTests.Insert(test);
					if (!m_aFailedSuites.Contains(suite))
						m_aFailedSuites.Insert(suite);
				}
				else
				{
					m_aSuccededTests.Insert(test);
				}
			}
		}
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected void FillDialog()
	{
		int failed, total;
		
		foreach (SCR_AutotestSuiteBase suite : SCR_AutotestHarness.s_aCurrentSuites)
		{
			m_sDialogText += string.Format("%1:\n", suite.ClassName());
			// TODO missing GetFailure() API on TestSuite
			// TODO handle test suite failure
			
			foreach (SCR_AutotestCaseBase test : suite.GetTestCaseInstances())
			{
				m_sDialogText += string.Format("\t%1\n", GetTestResultLine(test, failed));
				total++;
			}
			
			m_sDialogText += "\n";
		}

		m_sDialogText += string.Format("FAILED: %1\nTOTAL: %2", failed, total);
	}

	//------------------------------------------------------------------------------------------------
	void OpenDialog()
	{
		string title = "Test result";
		if (m_bIsFailure)
		{
			title += " FAILURE";
		}
		else
		{
			title += " SUCCESS";
		}

		Workbench.ScriptDialog(title, m_sDialogText, this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute(label: "Open autotest.log")]
	void ButtonOpenLog()
	{
		ScriptEditor se = Workbench.GetModule(ScriptEditor);
		se.SetOpenedResource(SCR_AutotestPrinter.LOG_PATH);
	}

	//------------------------------------------------------------------------------------------------
	private string GetTestResultLine(SCR_AutotestCaseBase test, out int failedCount)
	{
		// TODO missing GetFailure() API on TestSuite
		//if (test.GetSuite().GetFailure())
			//return string.Format("⚠ %1: SKIPPED");
		
		TestFailureBase result = test.GetFailure();
		if (!result)
		{
			return string.Format("✅ %1: SUCCESS", test.GetName());
		}

		if (TestResultTimeout.Cast(result))
		{
			SetFailed();
			failedCount++;
			return string.Format("⌚ %1: FAILURE", test.GetName());
		}

		SetFailed();
		failedCount++;
		return string.Format("⛔ %1: FAILURE", test.GetName());
	}
#endif

}
