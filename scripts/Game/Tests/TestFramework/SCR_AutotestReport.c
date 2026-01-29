
class SCR_AutotestReport
{
	protected static const string JUNIT_PATH = "$logs:/junit.xml";
	protected static const string FAILED_LIST_PATH = "$logs:/autotest_failed.log";

	protected string m_sJUnitXml;
	protected ref set<TestBase> m_aFailedTests = new set<TestBase>();
	protected bool m_bIsFailure;

#ifdef WORKBENCH
	protected string m_sDialogText;
#endif

	void CollectResults()
	{
		SetJUnit(TestHarness.Report());
		FillFailed();
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

	void WriteFailedList()
	{
		string path = FAILED_LIST_PATH;
		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		foreach (TestBase test : m_aFailedTests)
		{
			file.WriteLine(test.GetName());
		}

		file.Close();

		PrintFormat("Autotest failed list saved to: %1", path, level: LogLevel.NORMAL);
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
	protected void FillFailed()
	{
		int suitesCount = TestHarness.GetNSuites();
		for (int i; i < suitesCount; i++)
		{
			TestSuite suite = TestHarness.GetSuite(i);
			if (!suite.IsEnabled())
				continue;

			int testsCount = suite.GetNTests();
			for (int j; j < testsCount; j++)
			{
				TestBase test = suite.GetTest(j);
				if (!test.IsEnabled())
					continue;

				TestResultBase result = test.GetResult();
				if (!result || result.Failure())
				{
					SetFailed();
					m_aFailedTests.Insert(test);
				}
			}
		}
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected void FillDialog()
	{
		int failed, total;

		int suitesCount = TestHarness.GetNSuites();
		for (int i; i < suitesCount; i++)
		{
			TestSuite suite = TestHarness.GetSuite(i);
			if (!suite.IsEnabled())
				continue;

			m_sDialogText += string.Format("%1:\n", suite.ClassName());

			int testsCount = suite.GetNTests();
			for (int j; j < testsCount; j++)
			{
				TestBase test = suite.GetTest(j);
				if (!test.IsEnabled())
					continue;

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
	private string GetTestResultLine(TestBase test, out int failedCount)
	{
		TestResultBase result = test.GetResult();
		if (!result)
		{
			SetFailed();
			failedCount++;
			return string.Format("⚠️ %1: NO_RESULT", test.GetName());
		}

		if (TestResultTimeout.Cast(result))
		{
			SetFailed();
			failedCount++;
			return string.Format("⌚ %1: FAILURE", test.GetName());
		}

		if (result.Failure())
		{
			SetFailed();
			failedCount++;
			return string.Format("⛔ %1: FAILURE", test.GetName());
		}

		return string.Format("✅ %1: SUCCESS", test.GetName());
	}
#endif

}
