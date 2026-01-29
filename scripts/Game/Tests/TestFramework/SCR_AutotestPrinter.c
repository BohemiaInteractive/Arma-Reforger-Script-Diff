
/*!
Specialized logging class for tests. Outputs logs to console and also buffers them for later output to autotest log file.
Should be used instead of Print and PrintFormat inside the SCR_AutotestSuite and SCR_AutotestCase classes.
*/
class SCR_AutotestPrinter
{
	static const string LOG_PATH = "$logs:autotest.log";
	static const int LOG_ONCE_THRESHOLD = 10000;

	protected bool m_bLogVerbose;
	protected ref FileHandle m_LogFile;

	protected ref map<typename, ref array<string>> m_mLogBuffer = new map<typename, ref array<string>>();

	protected string m_sLogOnceLastMsg;
	protected int m_iLogOnceLastMsgCount;

	//------------------------------------------------------------------------------------------------
	void PrintTestSuitePrelude(TestSuite suite)
	{
		Log("");
		Log("############################################/");
		Log(string.Format("TestSuite #%1 started", suite.ClassName()));
	}

	//------------------------------------------------------------------------------------------------
	void PrintTestSuiteEpilogue(TestSuite suite)
	{
		Log("/############################################");
		Log("");
	}

	//------------------------------------------------------------------------------------------------
	//! Should be used in tests instead of global Print. Forwards test output to separate file.
	void Log(string msg, LogLevel level = LogLevel.NORMAL, bool forceFileWrite = false, bool consoleLog = true)
	{
		// dump info how many "LogOnce"s preceeded this message
		if (m_iLogOnceLastMsgCount > 0)
		{
			string logOnceMsg = string.Format("(x%2) %1", m_sLogOnceLastMsg, m_iLogOnceLastMsgCount);
			m_iLogOnceLastMsgCount = 0;
			m_sLogOnceLastMsg = "";
			Log(logOnceMsg);
		}

		if (consoleLog)
		{
			Print("" + msg, level);
		}

		string msgFile = string.Format("%3 %1%2", GetLogPrefix(level), msg, SCR_DateTimeHelper.GetTimeLocal());

		TestBase activeTest = SCR_AutotestHarness.ActiveTestCase();
		if (activeTest && !forceFileWrite)
		{
			typename activeTestType = activeTest.Type();
			if (!m_mLogBuffer.Contains(activeTestType))
			{
				m_mLogBuffer.Insert(activeTestType, {});
			}

			array<string> buffer = m_mLogBuffer.Get(activeTestType);
			buffer.Insert(msgFile);
		}
		else
		{
			m_LogFile.WriteLine(msgFile);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Prevents duplicate printing of the same message. Intended to be used with messages printed many times a second.
	void LogOnce(string msg, LogLevel level = LogLevel.NORMAL)
	{
		if (msg == m_sLogOnceLastMsg && m_iLogOnceLastMsgCount < LOG_ONCE_THRESHOLD)
		{
			m_iLogOnceLastMsgCount++;
			return;
		}

		if (msg == m_sLogOnceLastMsg && m_iLogOnceLastMsgCount >= LOG_ONCE_THRESHOLD)
		{
			int count = m_iLogOnceLastMsgCount;
			m_iLogOnceLastMsgCount = 0;
			Log(string.Format("(x%2) %1", m_sLogOnceLastMsg, count), level);

			return;
		}

		m_sLogOnceLastMsg = msg;
		m_iLogOnceLastMsgCount = 0;
		Log(msg, level);
	}

	//------------------------------------------------------------------------------------------------
	//! Writes test result to console and autotest log files.
	//! If test is failed or verbose logging is enabled will additionaly print test log output for debugging.
	void LogTestCaseResult(TestBase test)
	{
		TestResultBase result = test.GetResult();
		if (!result)
		{
			Log(string.Format("\t⚠️ %1: NO_RESULT", test.GetName()), forceFileWrite: true);
			return;
		}

		if (TestResultTimeout.Cast(result))
		{
			Log(string.Format("\t⌚ %1: FAILURE", test.GetName()), forceFileWrite: true);
			Log(string.Format("\t\tFailure reason: %1", "timeout"), forceFileWrite: true);
			DumpTestBuffer(test.Type());
			return;
		}

		if (result.Failure())
		{
			Log(string.Format("\t⛔ %1: FAILURE", test.GetName()), forceFileWrite: true);
			
			string failureReason = result.FailureText();
			SCR_AutotestResult autotestResult = SCR_AutotestResult.Cast(result);
			if (autotestResult)
				failureReason = autotestResult.GetFailureReason();
			Log(string.Format("\t\tFailure reason: %1", failureReason), forceFileWrite: true);
			
			DumpTestBuffer(test.Type());
		}
		else
		{
			Log(string.Format("\t✅ %1: SUCCESS", test.GetName()), forceFileWrite: true);
			if (m_bLogVerbose)
			{
				DumpTestBuffer(test.Type());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Dumps buffered logs into autotest.log, allows us to output logs from test after its result was printed.
	private void DumpTestBuffer(typename testType)
	{
		array<string> buffer = m_mLogBuffer.Get(testType);
		if (!buffer)
		{
			Log("\t Output: <none>", forceFileWrite: true, consoleLog: false);
			return;
		}

		Log("\t Output:", forceFileWrite: true, consoleLog: false);
		foreach (string msgFile : buffer)
		{
			m_LogFile.WriteLine("\t" + msgFile);
		}

		m_mLogBuffer.Remove(testType);
	}

	//------------------------------------------------------------------------------------------------
	private string GetLogPrefix(LogLevel level)
	{
		if (level <= LogLevel.NORMAL)
			return "";

		return string.Format("(%1): ", typename.EnumToString(LogLevel, level));
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AutotestPrinter(bool verbose)
	{
		m_bLogVerbose = verbose;
		m_LogFile = FileIO.OpenFile(LOG_PATH, FileMode.WRITE);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AutotestPrinter()
	{
		m_LogFile.Close();
	}
}
