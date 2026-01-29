[ComponentEditorProps(visible: false)]
class SCR_AIInfoBaseComponentClass : ScriptComponentClass
{
}

//! Base class for AIInfoComponent and AIGroupInfoComponent
class SCR_AIInfoBaseComponent : ScriptComponent
{
	// Debug history
	#ifdef AI_DEBUG
	ref array<ref SCR_AIDebugMessage> m_aDebugMessages = {};
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] str
	//! \param[in] msgType
	//! \param[in] logLevel
	void AddDebugMessage(string str, EAIDebugMsgType msgType = EAIDebugMsgType.NONE, LogLevel logLevel = LogLevel.NORMAL)
	{
		SCR_AIDebugMessage msg = new SCR_AIDebugMessage(str, msgType, logLevel);
		AddDebugMessageObject(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msg
	void AddDebugMessageObject(SCR_AIDebugMessage msg)
	{
		m_aDebugMessages.Insert(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dumps all debug messages of this AI.
	//! \param[in] msgTypeFilter
	//! \param[in] useTypeFilter
	//! \param[in] ageThresholdMs will dump most recent messages younger than given threshold. Ignored when -1.
	void DumpDebugMessages(EAIDebugMsgType msgTypeFilter = EAIDebugMsgType.NONE, bool useTypeFilter = false, int ageThresholdMs = -1)
	{	
		int idStart = 0;
		
		// If age threshold is provided, find from which message to start logging
		if (ageThresholdMs != -1)
		{
			idStart = -1;
			int timestampMsThreshold = System.GetTickCount() - ageThresholdMs;
			for (int i = m_aDebugMessages.Count() - 1; i >= 0; i--)
			{
				if (m_aDebugMessages[i].m_TimestampMs >= timestampMsThreshold)
					idStart = i;
				else
					break;
			}
		}
		
		// Log messages
		const string strLine  = "----------------------------------------------------------------------------------";
		
		LogToFile("\n\n\n\n");
		LogToFile(strLine);
		LogToFile(string.Format("Debug Message Dump for %1", GetOwner()));
		if (ageThresholdMs != -1)
			LogToFile(string.Format("Logging messages for past %1 milliseconds", ageThresholdMs));
		
		int msgCount = m_aDebugMessages.Count();
		for (int i = idStart; i < msgCount; i++)
		{
			SCR_AIDebugMessage msg = m_aDebugMessages[i];
			if (msg.m_Type == msgTypeFilter || !useTypeFilter)
				LogToFile(msg.FormatMessage(), msg.m_LogLevel);
		}
		
		LogToFile("End Debug Message Dump");
		LogToFile(strLine);
		LogToFile("\n\n\n\n");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LogToFile(string msg, LogLevel logLevel = LogLevel.NORMAL)
	{
		Print(msg, logLevel);
		SCR_AIDebug.DebugLog(msg, logLevel);
	}
	
	#endif // AI_DEBUG
}
