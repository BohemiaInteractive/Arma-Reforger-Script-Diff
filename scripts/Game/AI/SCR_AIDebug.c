//! Game core which persists through whole game and stores various data for AI debugging.
[BaseContainerProps(configRoot: true)]
class SCR_AIDebug : SCR_GameCoreBase
{
	const string logFileName = "$logs:ai_script.log";
	
	protected static ref SCR_AIDebug s_Instance;
	
	protected ref FileHandle m_FileHandle;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_AIDebug GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		s_Instance = this;
						
		#ifdef AI_DEBUG
		m_FileHandle = FileIO.OpenFile(logFileName, FileMode.WRITE);
		if (!m_FileHandle)
			Print("SCR_AIDebug: Failed to create log file", LogLevel.ERROR);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		if (m_FileHandle)
		{
			m_FileHandle.Close();
			m_FileHandle = null;
		}
		
		if (s_Instance)
		{
			s_Instance = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param behavior
	//! \return empty string if behavior is null
	static string GetBehaviorName(SCR_AIBehaviorBase behavior)
	{
		if (!behavior)
			return "";
		
		return behavior.Type().ToString();
	}
	
	#ifdef AI_DEBUG
	//------------------------------------------------------------------------------------------------
	//!
	//! \param str
	//! \param logLevel
	static void DebugLog(string str, LogLevel logLevel = LogLevel.DEBUG)
	{	
		SCR_AIDebug inst = GetInstance();
		
		if(!inst.m_FileHandle)
			return;
		
		// Resolve out log level
		bool copyToStdLog = false;
		string logLevelStr;
		switch(logLevel)
		{
			case LogLevel.SPAM:		logLevelStr = "S"; break;
			case LogLevel.VERBOSE:	logLevelStr = "v"; break;
			case LogLevel.DEBUG:	logLevelStr = "D"; break;
			case LogLevel.NORMAL:	logLevelStr = "N"; break;
			case LogLevel.WARNING:	logLevelStr = "W"; copyToStdLog = true; break;
			case LogLevel.ERROR:	logLevelStr = "E"; copyToStdLog = true; break;
			case LogLevel.FATAL:	logLevelStr = "F"; copyToStdLog = true; break;
		}
		
		// Get time
		int hr, min, sec;
		System.GetHourMinuteSecond(hr, min, sec);
		int ticks = System.GetTickCount();
		
		// Format string
		string formattedStr = string.Format(
			"%1:%2:%3 %4 (%5): %6",
			hr.ToString(2),
			min.ToString(2),
			sec.ToString(2),
			ticks,
			logLevelStr,
			str
		);
		
		inst.m_FileHandle.Write(formattedStr);
		inst.m_FileHandle.Write("\n");
		
		if (copyToStdLog)
			Print(str, logLevel);
	}
	#endif
}
