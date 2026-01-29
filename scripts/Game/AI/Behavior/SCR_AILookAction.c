class SCR_AILookAction : Managed 
{
	SCR_AIUtilityComponent m_Utility;
	
	vector m_vPosition;		// {0,0,0} is an invalid position
	bool m_bCancelLook;		// There was a request to cancel character looking
	bool m_bRestartLook;	// Restart might happen if we have a new request while previous one is still not Complete()
	float m_fDuration;		// How long to look at something
	
	static const float PRIO_COMMANDER = 100;
	static const float PRIO_ENEMY_TARGET = 80;
	static const float PRIO_UNKNOWN_TARGET = 50;
	static const float PRIO_DANGER_EVENT = 20;
	
	protected float m_fPriority;
	
	void SCR_AILookAction(SCR_AIUtilityComponent utility, bool prioritize)
	{
		m_Utility = utility;
	}
	
	void LookAt(vector pos, float priority, float duration = 0.8)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("LookAt: pos: %1, priority: %2/%3", pos, priority, m_fPriority));
		#endif
		
		if (priority < m_fPriority)
			return;
		
		// Restart if a previous look action is in process
		if (m_vPosition != vector.Zero)
			m_bRestartLook = true;
		
		m_vPosition = pos;
		m_fPriority = priority;
		m_fDuration = duration;
	}
	
	void LookAt(IEntity ent, float priority, float duration = 0.8)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("LookAt: ent: %1, priority: %2/%3", ent, priority, m_fPriority));
		#endif
		
		if (priority < m_fPriority && ent)
			return;

		// Restart if a previous look action is in process
		if (m_vPosition != vector.Zero)
			m_bRestartLook = true;
		
		vector min, max;
		ent.GetBounds(min, max);
		m_vPosition = ent.GetOrigin() + (min + max) * 0.5;
		m_fPriority = priority;
		m_fDuration = duration;
	}
	
	void Cancel()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Cancel"));
		#endif
		
		Complete();
		m_bCancelLook = true;
	}
	
	void Complete()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Complete"));
		#endif
		
		m_vPosition = vector.Zero;
		m_fPriority = 0;
	}
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		SCR_AIInfoBaseComponent infoComp = m_Utility.m_AIInfo;
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.LOOK, logLevel);
	}
	#endif
	
	//! Must be called by look behavior tree  to move data to it.
	//! This call also resets various flags.
	void MoveLookParametersToNode(out bool outCanLook, out vector outLookPos, out float outLookDuration, out bool outCancelLook, out bool outRestartLook)
	{
		outCanLook = m_vPosition != vector.Zero;
		
		outCancelLook = m_bCancelLook;
		m_bCancelLook = false; // Reset the flag if it was set
		
		outRestartLook = m_bRestartLook;
		m_bRestartLook = false; // Reset the flag if it was set

		outLookPos = m_vPosition;
		outLookDuration = m_fDuration;
	}
};