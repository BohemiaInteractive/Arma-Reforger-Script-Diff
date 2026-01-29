enum EAIDebugMsgType
{
	NONE	= 0,
	UTILITY	= 1,
	MAILBOX	= 2,
	COMBAT	= 3,
	INFO	= 4,
	ACTION	= 5,
	THREAT	= 6,
	WEAPON	= 7,
	LOOK	= 8,
	CLUSTER	= 9,
	REACTION = 10,
	COMMS	= 11,
	SETTINGS = 12
};

class SCR_AIDebugMessage
{
	// Texts which match EAIDebugMsgType, make sure they all have same length so they are printed nicely
	static const ref array<string> s_aAiDebugMsgTypeLabels = {
		"NONE  ",
		"UTIL  ",
		"MAIL  ",
		"COMBAT",
		"INFO  ",
		"ACTION",
		"THREAT",
		"WEAPON",
		"LOOK  ",
		"CLUSTR",
		"REACT ",
		"COMMS ",
		"SETTIN"
	};
	
	int m_Hours;
	int m_Minutes;
	int m_Seconds;
	int m_TimestampMs;	// Total amount of milliseconds since game start
	
	EAIDebugMsgType m_Type;
	LogLevel m_LogLevel;
	
	string m_Message;
	
	void SCR_AIDebugMessage(string message, EAIDebugMsgType type = EAIDebugMsgType.NONE, LogLevel logLevel = LogLevel.NORMAL)
	{
		m_Message = message;
		m_LogLevel = logLevel;
		m_Type = type;
		System.GetHourMinuteSecond(m_Hours, m_Minutes, m_Seconds);
		m_TimestampMs = System.GetTickCount();
	}
	
	string FormatMessage()
	{
		/*
		string logLevelStr;
		switch (m_LogLevel)
		{
			case LogLevel.SPAM: 	logLevelStr = "S"; break;
			case LogLevel.VERBOSE: 	logLevelStr = "V"; break;
			case LogLevel.DEBUG: 	logLevelStr = "D"; break;
			case LogLevel.NORMAL: 	logLevelStr = "N"; break;
			case LogLevel.WARNING: 	logLevelStr = "W"; break;
			case LogLevel.ERROR: 	logLevelStr = "E"; break;
			case LogLevel.FATAL: 	logLevelStr = "F"; break;
		}
		*/
		
		int timestampMsCurrent = System.GetTickCount();
		float tDeltaSeconds = (timestampMsCurrent - m_TimestampMs) / 1000.0;
		
		return string.Format("(%1:%2:%3 %4 T-%5s) %6 %7",
			m_Hours.ToString(2),
			m_Minutes.ToString(2),
			m_Seconds.ToString(2),
			m_TimestampMs.ToString(6),
			tDeltaSeconds.ToString(5, 1),
			s_aAiDebugMsgTypeLabels[m_Type],
			//logLevelStr,
			m_Message
		);
	}
};