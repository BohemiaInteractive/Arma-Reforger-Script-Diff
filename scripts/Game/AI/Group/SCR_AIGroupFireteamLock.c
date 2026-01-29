/*!
Lock object for aquiring free fireteams.
On destruction it releases the fireteam.
*/

typedef array<ref SCR_AIGroupFireteamLock> TFireteamLockRefArray;
typedef array<SCR_AIGroupFireteamLock> TFireteamLockArray;

class SCR_AIGroupFireteamLock : Managed
{
	protected ref SCR_AIGroupFireteam m_Fireteam;
	
	//--------------------------------------------------------------------------------------------------------
	void SCR_AIGroupFireteamLock(SCR_AIGroupFireteam ft)
	{
		m_Fireteam = ft;
	}
	
	//--------------------------------------------------------------------------------------------------------
	void ~SCR_AIGroupFireteamLock()
	{
		if (m_Fireteam)
			m_Fireteam.Internal_OnLockDestroyed();
	}
	
	//--------------------------------------------------------------------------------------------------------
	SCR_AIGroupFireteam GetFireteam()
	{
		return m_Fireteam;
	}
	
	
	
	//--------------------------------------------------------------------------------------------------------
	//! Helper functions to manipulate fireteam locks in arrays
	
	//--------------------------------------------------------------------------------------------------------
	//! Returns ID of fireteam lock which references the passed fireteam
	//! Two functions, for TFireteamLockRefArray and TFireteamLockArray
	static int FindFireteamLock(notnull TFireteamLockRefArray locksArray, SCR_AIGroupFireteam ft)
	{
		foreach (int i, SCR_AIGroupFireteamLock l : locksArray)
		{
			if (l.m_Fireteam == ft)
				return i;
		}
		return -1;
	}
	static int FindFireteamLock(notnull TFireteamLockArray locksArray, SCR_AIGroupFireteam ft)
	{
		foreach (int i, SCR_AIGroupFireteamLock l : locksArray)
		{
			if (l.m_Fireteam == ft)
				return i;
		}
		return -1;
	}
	
	//--------------------------------------------------------------------------------------------------------
	//! Returns ID of fireteam lock
	//! Two functions, for TFireteamLockRefArray and TFireteamLockArray
	static int FindFireteamLock(notnull TFireteamLockRefArray locksArray, SCR_AIGroupFireteamLock ft)
	{
		foreach (int i, SCR_AIGroupFireteamLock l : locksArray)
		{
			if (l == ft)
				return i;
		}
		return -1;
	}
	static int FindFireteamLock(notnull TFireteamLockArray locksArray, SCR_AIGroupFireteamLock ft)
	{
		foreach (int i, SCR_AIGroupFireteamLock l : locksArray)
		{
			if (l == ft)
				return i;
		}
		return -1;
	}
	
	//--------------------------------------------------------------------------------------------------------
	//! Removes fireteam lock which references the passed fireteam
	//! Returns true if the fireteam lock was found
	//! Two functions, for TFireteamLockRefArray and TFireteamLockArray
	static bool RemoveFireteamLock(TFireteamLockRefArray locksArray, SCR_AIGroupFireteam ft)
	{
		int id = FindFireteamLock(locksArray, ft);
		if (id == -1)
			return false;
		
		locksArray.Remove(id);
		return true;
	}
	static bool RemoveFireteamLock(TFireteamLockArray locksArray, SCR_AIGroupFireteam ft)
	{
		int id = FindFireteamLock(locksArray, ft);
		if (id == -1)
			return false;
		
		locksArray.Remove(id);
		return true;
	}
	
	//--------------------------------------------------------------------------------------------------------
	//! Tries to lock all fireteams, returns a new array of locks for those fireteams it was able to lock
	static void TryLockFireteams(notnull array<SCR_AIGroupFireteam> fireteams, notnull TFireteamLockRefArray locks, bool clearLockArray)
	{
		if (clearLockArray)
			locks.Clear();
		
		foreach (SCR_AIGroupFireteam ft : fireteams)
		{
			SCR_AIGroupFireteamLock lock = ft.TryLock();
			if (lock)
				locks.Insert(lock);
		}
	}
	
	//--------------------------------------------------------------------------------------------------------
	//! Creates a copy of locks array
	static TFireteamLockRefArray CopyLockArray(TFireteamLockRefArray other)
	{
		TFireteamLockRefArray a = {};
		foreach (auto l : other)
			a.Insert(l);
		return a;
	}
};