/*!
Base class for all group activities which are related to fireteams.
When an activity of this (or inherited) class exists, it prevents group from rebalancing fireteams.
When there are no fireteams left, it fails.
*/
class SCR_AIFireteamsActivity : SCR_AIActivityBase
{
	protected ref TFireteamLockRefArray m_aAssignedFireteams = {}; // Don't manipulate yourself, use register/unregister!
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIFireteamsActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint,)
	{
		SetIsUniqueInActionQueue(false);
		utility.m_FireteamMgr.GetOnFireteamRemoved().Insert(Event_OnFireteamRemovedFromGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AIFireteamsActivity()
	{
		if (m_Utility)
			m_Utility.m_FireteamMgr.GetOnFireteamRemoved().Remove(Event_OnFireteamRemovedFromGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionRemoved()
	{
		// Unregister all fireteams, don't call the event
		m_aAssignedFireteams.Clear();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void GetAssignedFireteams(notnull TFireteamLockRefArray outFireteams)
	{
		foreach (auto ft : m_aAssignedFireteams)
			outFireteams.Insert(ft);
	}
	
	//------------------------------------------------------------------------------------------------
	// Fireteam registration
	// Inherited class should register and unregister the fireteams itself.
	
	//------------------------------------------------------------------------------------------------
	//! Registers fireteam within this activity
	protected void RegisterFireteam(notnull SCR_AIGroupFireteamLock ftLock)
	{
		if (SCR_AIGroupFireteamLock.FindFireteamLock(m_aAssignedFireteams, ftLock) != -1)
			return;
		
		m_aAssignedFireteams.Insert(ftLock);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterFireteam(notnull SCR_AIGroupFireteamLock ftLock)
	{
		int id = SCR_AIGroupFireteamLock.FindFireteamLock(m_aAssignedFireteams, ftLock);
		if (id != -1)
			return;
		
		m_aAssignedFireteams.Remove(id);
		
		if (m_aAssignedFireteams.IsEmpty())
			OnAssignedFireteamsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterFireteam(notnull SCR_AIGroupFireteam ft)
	{
		int id = SCR_AIGroupFireteamLock.FindFireteamLock(m_aAssignedFireteams, ft);
		if (id == -1)
			return;
		
		m_aAssignedFireteams.Remove(id);
		
		if (m_aAssignedFireteams.IsEmpty())
			OnAssignedFireteamsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	private void Event_OnFireteamRemovedFromGroup(SCR_AIGroupFireteam ft)
	{
		// Bail if it wasn't one of FTs of this activity
		if (SCR_AIGroupFireteamLock.FindFireteamLock(m_aAssignedFireteams, ft) == -1)
			return;
		
		// Call the virtual method
		OnFireteamRemovedFromGroup(ft);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override in inherited classes to do something when all fireteams are gone
	void OnAssignedFireteamsEmpty()
	{
		#ifdef AI_DEBUG
		AddDebugMessage("All fireteams are destroyed, activity is failed");
		#endif
		Fail();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override in inherited classes to do something on fireteam deletion
	//! It is called only for registered fireteams
	void OnFireteamRemovedFromGroup(SCR_AIGroupFireteam ft);
	
	//------------------------------------------------------------------------------------------------
	override string GetDebugPanelText()
	{
		// List all assigned fireteams
		string str = "FTs: ";
		foreach (SCR_AIGroupFireteamLock ftLock : m_aAssignedFireteams)
		{
			str = str + string.Format("%1, ", m_Utility.m_FireteamMgr.GetFireteamId(ftLock.GetFireteam()));
		}
		
		return str;
	}
};