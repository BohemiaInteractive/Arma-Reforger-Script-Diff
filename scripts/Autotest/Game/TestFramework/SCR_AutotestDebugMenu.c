#ifdef ENABLE_DIAG

modded enum SCR_DebugMenuID
{
	DEBUGUI_AUTOTEST_PROGRESS
}

class SCR_AutotestDebugMenu
{
	ref set<typename> s_aAllTests = new set<typename>();

	protected static ref SCR_AutotestDebugMenu s_Instance;
	protected static bool s_mRegistered;

	protected typename m_sLastExecutedTest;
	protected int m_iExecutedTests;
	protected int m_iLastExecutedTestStart;

	private void SCR_AutotestDebugMenu();

	static SCR_AutotestDebugMenu GetInstance()
	{
		if (!s_Instance)
			s_Instance = new SCR_AutotestDebugMenu();

		return s_Instance;
	}

	static void Terminate()
	{
		delete s_Instance;
	}

	void Update()
	{
		if (!s_mRegistered)
		{
			s_mRegistered = true;
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AUTOTEST_PROGRESS, "", "Autotest status", "GameCode", true);
		}

		DbgUI.Begin("Autotest status");
		TestBase currentTest = SCR_AutotestHarness.ActiveTestCase();
		if (currentTest)
		{
			DbgUI.Text(string.Format("%1/%2", GetCurrentTestNumber(currentTest), s_aAllTests.Count()));
			DbgUI.Text(currentTest.GetName());
			DbgUI.Text((System.GetUnixTime() - m_iLastExecutedTestStart).ToString() + "s");

		}
		DbgUI.End();
	}

	protected int GetCurrentTestNumber(TestBase test)
	{
		if (m_sLastExecutedTest != test.Type())
		{
			m_sLastExecutedTest = test.Type();
			m_iLastExecutedTestStart = System.GetUnixTime();
			m_iExecutedTests++;
		}

		return m_iExecutedTests;
	}
}
#endif
