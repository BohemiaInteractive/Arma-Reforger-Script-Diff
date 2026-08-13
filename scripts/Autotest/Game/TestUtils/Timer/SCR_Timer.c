class SCR_Timer
{
	//! This is a database of all currently active timers
	static protected ref map<string, ref SCR_TimerEntryBase> s_mTimerMap = new map<string, ref SCR_TimerEntryBase>();
	
	//! This value is used when timer controls are called without a timer label argument
	static private const string BASE_TIMER_NAME = "BaseTimer";
	
	//! \code
	//! Start("BaseTimer", timeToWait, realTime, log);
	//! \endcode
	//static void Start(float timeToWait, bool realTime = false, World world = null, bool log = false)
	//{
	//	Start(BASE_TIMER_NAME, timeToWait, realTime, world, log);
	//}
	
	//! Creates a timer and marks its start. 
	//! Timers can be started even if they are already running. Doing so will reset the timer.
	//! \param timerName Used as the key for this timer in the database
	//! \param timeToWait Timer duration 
	//! \param realTime Specifies if world time or real time should be used. World time is bound to the world
	//! while real time will continue ticking regardless of changing maps, exiting to menu, etc.
	//! \param log Specifies if you want a log line about this timer in the output
	static void Start(float timeToWait, string timerName = BASE_TIMER_NAME, bool realtime = false, World world = null, bool log = false)
	{
		SCR_TimerEntryBase timer = GetMap().Get(timerName);
		if (timer && timer.IsRealTime() == realtime)
			timer.SetTimer(timeToWait);
		else
		{
			Stop(timerName);
			CreateTimer(timerName, timeToWait, realtime, world);
		}

		if (log)
			SCR_TestLib.Log(string.Format("Started timer for %1 seconds.", timeToWait.ToString()));
	}
	
	//! Returns the specified timer's remaining runtime
	static float GetRemaining(string timerName = BASE_TIMER_NAME)
	{
		if (!GetMap().Contains(timerName))
		{
			SCR_TestLib.Log(string.Format("Timer %1 doesn't exist!", timerName), LogLevel.ERROR);
			return 0;
		}
		
		return GetMap().Get(timerName).GetRemaining();
	}
	
	//! Checks if the timer exists in the database.
	//! This can be used to verify that the timer was started.
	static bool Exists(string timerName = BASE_TIMER_NAME)
	{
		return GetMap().Contains(timerName);
	}
	
	//! Convenience method for !TimerFinished().
	//! Use Exists() to check if the timer started
	static bool IsRunning(string timerName = BASE_TIMER_NAME)
	{
		return !Finished(timerName, false);
	}
	
	//! Returns true if the specified timer has finished
	//! \param timerName Which timer to check
	//! \param clearTimer Specifies if the timer should be removed from the database IF this call returns "true"; set to "false" by default
	static bool Finished(string timerName = BASE_TIMER_NAME, bool clearTimer = false)
	{
		if (!GetMap().Contains(timerName))
		{
			SCR_TestLib.Log(string.Format("Timer %1 doesn't exist or has finished with 'clearTimer=true'!", timerName), LogLevel.WARNING);
			return true;
		}
		
		bool result = GetMap().Get(timerName).Finished();
		
		if (clearTimer && result)
			GetMap().Remove(timerName);
		
		return result;
	}
	
	//! Returns true if ALL timers in the database have finished
	static bool AllFinished()
	{
		foreach (string key, ref SCR_TimerEntryBase value : GetMap())
		{
			if (!Finished(key, false))
				return false;
		}
		
		return true;
	}
	
	//! Returns true if ALL timers in the database, that use real time, have finished
	static bool AllFinishedRealTime()
	{
		foreach (string key, ref SCR_TimerEntryBase value : GetMap())
		{
			if (!value.IsRealTime())
				continue;
			
			if (!Finished(key, false))
				return false;
		}
		
		return true;
	}
	
	//! Returns true if ALL timers in the database, that use world time, have finished
	static bool AllFinishedWorldTime()
	{
		foreach (string key, ref SCR_TimerEntryBase value : GetMap())
		{
			if (value.IsRealTime())
				continue;
			
			if (!Finished(key, false))
				return false;
		}
		
		return true;
	}
	
	//! Removes the timer from the list of all running timers and returns whether it has finished by now or not
	static bool Stop(string timerName = BASE_TIMER_NAME)
	{
		if (!GetMap().Contains(timerName))
		{
			return true;
		}
		
		bool isFinished = Finished(timerName);
		GetMap().Remove(timerName);
		return isFinished;
	}
	
	//! Clear all currently active timers. Calling Finished(...) on them will return true with a warning
	static void ClearAll()
	{
		GetMap().Clear();
	}

	// ----------------------------			 ---------------------------------
	// ----------------------------	INTERNAL ---------------------------------
	// ----------------------------			 ---------------------------------
	
	static protected ref map<string, ref SCR_TimerEntryBase> GetMap()
	{
		return s_mTimerMap;
	}
	
	private static void CreateTimer(string timerName, float timeToWait, bool realTime, World world)
	{
		SCR_TimerEntryBase timer;
		if (realTime)
			timer = new SCR_RealTimerEntry(timerName);
		else
			timer = new SCR_WorldTimerEntry(timerName, world);
			
		timer.SetTimer(timeToWait);
		GetMap().Insert(timerName, timer);
	}
}

//! This is an override used internally by the FSM and other tools
//! in order to keep the maps separate for the sake of WaitForAll methods etc.
//! DO NOT USE in your autotests!
class SCR_TimerInternal : SCR_Timer
{
	static protected ref map<string, ref SCR_TimerEntryBase> s_mInternalMap = new map<string, ref SCR_TimerEntryBase>();;
	
	override static protected ref map<string, ref SCR_TimerEntryBase> GetMap()
	{
		return s_mInternalMap;
	} 
}
