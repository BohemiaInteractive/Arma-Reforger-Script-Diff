class SCR_TestLib
{
	//! Objects that are supposed to be automatically deleted between test runs
	static ref array<Managed> s_aTestObjects = {};
	
	//! Logs to test specific log output.
	static void Log(string msg, LogLevel level = LogLevel.NORMAL)
	{
		SCR_AutotestPrinter logger = SCR_AutotestHarness.GetLogger();
		if (!logger)
			return;

		if (SCR_AutotestHarness.ActiveTestCase())
			msg = "\t" + msg;

		logger.LogOnce(msg, level);
	}
	
	//! Use SCR_Timer.Start instead
	static void TimerStart(float timeToWait, bool log = false)
	{
		SCR_Timer.Start(timeToWait, log: log);
	}

	//! Use SCR_TimerFinished instead
	static bool TimerFinished()
	{
		return SCR_Timer.Finished();
	}

	//! Use SCR_TimerRunning instead
	static bool TimerRunning()
	{
		return !TimerFinished();
	}
	
	//! Register Managed object for deletion after the test run.
	protected static void RegisterForDeletion(Managed object)
	{
		s_aTestObjects.Insert(object);
	}
	
	//! Deletes all test related objects registered for auto deletion.
	static void DeleteObjects()
	{
		foreach (Managed object : s_aTestObjects)
		{
			delete object;
		}

		s_aTestObjects = {};
	}
}