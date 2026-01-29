//! Fireteam base class
//! Fireteam is like a collection of agents in a group
class SCR_AIGroupFireteam : Managed
{
	protected ref array<AIAgent> m_aAgents = {};
	
	protected bool m_bLocked = false;
	
	//--------------------------------------------------------------------------
	void AddMember(AIAgent agent)
	{
		if (!m_aAgents.Contains(agent))
			m_aAgents.Insert(agent);
	}
	
	//--------------------------------------------------------------------------
	void RemoveMember(AIAgent agent)
	{
		int id = m_aAgents.Find(agent);
		if (id == -1)
			return;
		m_aAgents.Remove(id);
	}
	
	//--------------------------------------------------------------------------
	protected void RemoveMember(int id, out AIAgent outAgent)
	{
		outAgent = m_aAgents[id];
		
		m_aAgents.Remove(id);
	}
	
	//--------------------------------------------------------------------------
	AIAgent GetMember(int id)
	{
		if (!m_aAgents.IsIndexValid(id))
			return null;
		
		return m_aAgents[id];
	}
	
	//--------------------------------------------------------------------------
	IEntity GetFirstMemberEntity()
	{
		if (m_aAgents.IsEmpty())
			return null;
		foreach (AIAgent agent : m_aAgents)
		{
			if (!agent)
				continue;
			IEntity controlledEntity = agent.GetControlledEntity();
			return controlledEntity;
		}
		return null;
	}
	
	//--------------------------------------------------------------------------
	void GetMembers(notnull array<AIAgent> outAgents)
	{
		outAgents.Clear();
		foreach (auto a : m_aAgents)
			outAgents.Insert(a);
	}
	
	//--------------------------------------------------------------------------
	//! Moves 'count' members from other fireteam to this one
	void MoveMembersFrom(notnull SCR_AIGroupFireteam otherFt, int count)
	{
		// Bail if wrong count
		if (count <= 0)
			return;
		
		// Clamp count
		count = Math.ClampInt(count, 0, otherFt.m_aAgents.Count());
		
		// Remove the required amount of members
		for (int i = 0; i < count; i++)
		{
			// Remove last member from other fireteam
			int lastId = otherFt.m_aAgents.Count() - 1;
			AIAgent agent;
			otherFt.RemoveMember(lastId, agent);
			
			// Add the member to our fireteam
			AddMember(agent);
		}
	}
	
	//--------------------------------------------------------------------------
	bool HasMember(AIAgent agent)
	{
		return m_aAgents.Find(agent) != -1;
	}
	
	//--------------------------------------------------------------------------
	int GetMemberCount()
	{
		return m_aAgents.Count();
	}
	
	//--------------------------------------------------------------------------
	//! Don't ever try to free a fireteam yourself, use SCR_AIGroupFireteamLock instead
	void Internal_OnLockDestroyed()
	{
		m_bLocked = false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Acquiring fireteams
	
	//--------------------------------------------------------------------------
	bool IsLocked()
	{
		return m_bLocked;
	}
	
	//--------------------------------------------------------------------------
	//! Tries to lock the fireteam, on success returns the Lock object.
	//! Store the returned Lock object as strong reference.
	SCR_AIGroupFireteamLock TryLock()
	{
		if (m_bLocked)
			return null;
		
		m_bLocked = true;
		
		SCR_AIGroupFireteamLock lock = new SCR_AIGroupFireteamLock(this);
		return lock;
	}
};

//! Base class for fireteams dedicated to vehicle
//! This class must not be instantiated! It's base class for other classes.
class SCR_AIGroupFireteamVehicleBase : SCR_AIGroupFireteam
{
	protected SCR_AIVehicleUsageComponent m_VehicleUsageComponent;
	
	void SetVehicle(notnull SCR_AIVehicleUsageComponent vehicleComp)
	{
		if (m_VehicleUsageComponent)
			return;
		
		m_VehicleUsageComponent = vehicleComp;
	}
	
	SCR_AIVehicleUsageComponent GetVehicle()
	{
		return m_VehicleUsageComponent;
	}
}

//! Fireteam for vehicle crew
class SCR_AIGroupFireteamVehicleCrew : SCR_AIGroupFireteamVehicleBase
{
	
}

//! Fireteam for vehicle cargo
class SCR_AIGroupFireteamVehicleCargo : SCR_AIGroupFireteamVehicleBase
{
	
}