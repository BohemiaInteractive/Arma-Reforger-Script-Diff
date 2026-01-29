[ComponentEditorProps(category: "GameScripted/AIComponent", description: "Scripted m_Mailbox component", icon: HYBRID_COMPONENT_ICON)]
class SCR_MailboxComponentClass : AICommunicationComponentClass
{
}

class SCR_MailboxComponent : AICommunicationComponent
{
	#ifdef AI_MAILBOX_OVERFLOW_DETECTION
	protected int m_iMailboxOverflowCount = 0;
	#endif
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] remove
	//! \return first message in the queue
	AIMessage ReadMessage(bool remove)
	{
		/*
		// Disabled for now
		#ifdef AI_MAILBOX_OVERFLOW_DETECTION
		if (GetMessageCount() > 100 && m_iMailboxOverflowCount < 5)
		{
			LogMailboxStatistics(LogLevel.WARNING);
			m_iMailboxOverflowCount++;
		}
		#endif
		*/
		
		AIMessage msg = GetMessage(0);
		
		if (msg && remove)
			RemoveMessage(0);
		
		return msg;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] remove
	//! \return first order in the order queue
	AIOrder ReadOrder(bool remove)
	{
		AIOrder order = GetOrder(0);
		
		if (order && remove)
			RemoveOrder(0);
		
		return order;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] logLevel
	void LogMessageStatistics(LogLevel logLevel = LogLevel.NORMAL)
	{
		map<typename, int> msgTypeCount = new map<typename, int>();
		int msgCount = GetMessageCount();
		for (int i = 0; i < msgCount; i++)
		{
			AIMessage msg = GetMessage(i);
			if (!msg)
				continue;
			typename msgFinalType = msg.Type();
			int nMsgsOfType = msgTypeCount.Get(msgFinalType);
			nMsgsOfType++;
			msgTypeCount.Set(msgFinalType, nMsgsOfType);
		}
		
		Print(string.Format("SCR_MailboxComponent statistics: count: %1, %2, %3", GetMessageCount(), this, GetAIAgent()), logLevel);
		foreach (typename t, int count : msgTypeCount)
		{
			Print(string.Format("  %1: %2", t, count), logLevel);
		}
		Print(string.Format("  Total: %1", GetMessageCount()), logLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsCommunicationEnabled()
	{
		return IsActive();
	}
	
	#ifdef AI_DEBUG
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] message
	void DebugLogBroadcastMessage(AIMessage message)
	{
		AIAgent agent = GetAIAgent();
		SCR_AIMessageBase msgBase = SCR_AIMessageBase.Cast(message);
		
		string debugTextBasic = string.Format("----> SendMessage: %1 --> %2:\n", agent, message.GetReceiver());
		
		string debugText;
		if (msgBase)
			debugText = debugTextBasic + msgBase.GetDebugText();
		else
			debugText = debugTextBasic + string.Format("\t%1", message);
		
		SCR_AIDebug.DebugLog(debugText);
		AddDebugMessage(debugText);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] message
	void DebugLogOnReceived(AIMessage message)
	{
		AIAgent agent = GetAIAgent();
		
		AIAgent sender = message.GetSender();
		
		string debugTextBasic = string.Format("<---- RecvMessage: %1 <-- %2\n", agent, sender);
		
		SCR_AIMessageBase msgBase = SCR_AIMessageBase.Cast(message);
		
		string debugText;
		if (msgBase)
			debugText = debugTextBasic + msgBase.GetDebugText();
		else
			debugText = debugTextBasic + string.Format("\t%1", message);
		
		SCR_AIDebug.DebugLog(debugText);
		AddDebugMessage(debugText);
	}
	
	//--------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(GetAIAgent().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.MAILBOX);
	}
	#endif // AI_DEBUG
}
