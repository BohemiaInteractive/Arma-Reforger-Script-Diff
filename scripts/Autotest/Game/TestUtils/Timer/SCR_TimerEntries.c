class SCR_TimerEntryBase
{
	protected string m_sName;
	
	void SCR_TimerEntryBase(string name)
	{
		m_sName = name;
	}
	
	string GetName()
	{
		return m_sName;
	}
	
	void SetTimer(float seconds);
	
	bool Finished();
	
	float GetRemaining();
	
	bool IsRealTime();
}

class SCR_WorldTimerEntry : SCR_TimerEntryBase
{
	WorldTimestamp m_Timestamp;
	World m_World;
	
	void SCR_WorldTimerEntry(string name, World world)
	{
		m_sName = name;
		m_World = world;
	}
	
	override void SetTimer(float seconds)
	{
		m_Timestamp = SCR_Timer.GetCurrentWorldTime(m_World).PlusSeconds(seconds);
	}
	
	override bool Finished()
	{
		return m_Timestamp.LessEqual(SCR_Timer.GetCurrentWorldTime(m_World));
	}
	
	override float GetRemaining()
	{
		if (Finished())
			return 0;
		
		return m_Timestamp.DiffSeconds(SCR_Timer.GetCurrentWorldTime(m_World));
	}
	
	override bool IsRealTime()
	{
		return false;
	}
}

class SCR_RealTimerEntry : SCR_TimerEntryBase
{
	int m_Timestamp; // milliseconds since the game/workbench launched
	
	override void SetTimer(float seconds)
	{
		m_Timestamp = System.GetTickCount() + SCR_Timer.ToMilliseconds(seconds);
	}
	
	override bool Finished()
	{
		return m_Timestamp <= System.GetTickCount();
	}
	
	override float GetRemaining()
	{
		if (Finished())
			return 0;
		
		return SCR_Timer.ToSeconds(m_Timestamp - System.GetTickCount());
	}
		
	override bool IsRealTime()
	{
		return true;
	}
}