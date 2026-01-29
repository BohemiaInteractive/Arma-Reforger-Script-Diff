void SCR_AIOnFireteamRemoved(SCR_AIGroupFireteam fireteam);
typedef func SCR_AIOnFireteamRemoved;

typedef array<ref SCR_AIGroupFireteam> TAIFireteamArray;

class SCR_AIGroupFireteamManager : Managed
{
	protected const int FIRETEAM_MIN_SIZE = 1;
	
	protected SCR_AIGroup m_Group;
	protected ref array<ref SCR_AIGroupFireteam> m_aFireteams = {};
	bool m_bRebalanceFireteams = false; // True when fireteams become unbalanced
	
	// Fireteam events
	protected ref ScriptInvokerBase<SCR_AIOnFireteamRemoved> Event_OnFireteamRemoved = new ScriptInvokerBase<SCR_AIOnFireteamRemoved>();
	
	//---------------------------------------------------------------------------------------------------
	void SCR_AIGroupFireteamManager(SCR_AIGroup group)
	{
		m_Group = group;
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnFireteamRemoved> GetOnFireteamRemoved()
	{
		return Event_OnFireteamRemoved;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called from SCR_AIGroupUtilityComponent
	void OnAgentAdded(AIAgent agent)
	{	
		// Add to fireteam
		// We find smallest fireteam and rebalance them later
		typename ftTypename = SCR_AIGroupFireteam;
		SCR_AIGroupFireteam destFt = FindSmallestFireteam(m_aFireteams, ftTypename);
		if (!destFt)
			destFt = CreateFireteam(m_aFireteams, ftTypename);
		destFt.AddMember(agent);
		
		m_bRebalanceFireteams = true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called from SCR_AIGroupUtilityComponent
	void OnAgentRemoved(AIAgent agent)
	{
		// Remove from fireteam
		SCR_AIGroupFireteam ft = FindFireteam(agent);
		if (ft)
		{
			ft.RemoveMember(agent); // bye
			
			int ftSize = ft.GetMemberCount();
			if (ftSize == 0)
				RemoveFireteam(m_aFireteams, ft);
			
			// Rebalance fireteams later
			m_bRebalanceFireteams = true;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnAgentAssignedToVehicle(notnull AIAgent agent, notnull SCR_AIVehicleUsageComponent vehicleComp, ECompartmentType compType)
	{		
		// Remove from current fireteam
		SCR_AIGroupFireteam prevFt = FindFireteam(agent);
		
		if (prevFt)
		{
			prevFt.RemoveMember(agent);
			
			int ftSize = prevFt.GetMemberCount();
			if (ftSize == 0)
				RemoveFireteam(m_aFireteams, prevFt);
		}
		
		// Depending on compartment type, we will add the agent to specific type of fireteam
		// Which was created for this vehicle
		typename ftTypename;
		if (compType == ECompartmentType.CARGO)
			ftTypename = SCR_AIGroupFireteamVehicleCargo;
		else
			ftTypename = SCR_AIGroupFireteamVehicleCrew;
		
		array<ref SCR_AIGroupFireteam> ftsOfVehicle = {};
		FindFireteamsOfVehicle(ftsOfVehicle, vehicleComp, ftTypename);
		SCR_AIGroupFireteamVehicleBase ft = SCR_AIGroupFireteamVehicleBase.Cast(FindSmallestFireteam(ftsOfVehicle, ftTypename));
		if (!ft)
		{
			ft = SCR_AIGroupFireteamVehicleBase.Cast(CreateFireteam(m_aFireteams, ftTypename));
			ft.SetVehicle(vehicleComp);
		}
		ft.AddMember(agent);
		
		m_bRebalanceFireteams = true;
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnAgentUnassignedFromVehicle(AIAgent agent, notnull SCR_AIVehicleUsageComponent vehicleComp)
	{
		// Remove from current fireteam
		SCR_AIGroupFireteam prevFt = FindFireteam(agent);
		
		if (prevFt)
		{
			prevFt.RemoveMember(agent);
			
			int ftSize = prevFt.GetMemberCount();
			if (ftSize == 0)
				RemoveFireteam(m_aFireteams, prevFt);
		}
		
		// Add to a generic fireteam
		typename ftTypename = SCR_AIGroupFireteam;
		SCR_AIGroupFireteam destFt = FindSmallestFireteam(m_aFireteams, ftTypename);
		if (!destFt)
			destFt = CreateFireteam(m_aFireteams, ftTypename);
		destFt.AddMember(agent);
		
		// Rebalance fireteams later
		m_bRebalanceFireteams = true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Removes all fireteams related to this vehicle, soldiers are redistributed
	void OnVehicleRemoved(SCR_AIVehicleUsageComponent vehicleComp)
	{
		array<ref SCR_AIGroupFireteam> fireteamsOfVehicle = {};
		FindFireteamsOfVehicle(fireteamsOfVehicle, vehicleComp, SCR_AIGroupFireteamVehicleBase);
		
		array<AIAgent> agents = {};
		foreach (SCR_AIGroupFireteam ft : fireteamsOfVehicle)
		{
			agents.Clear();
			ft.GetMembers(agents);
			foreach (AIAgent agent : agents)
				OnAgentUnassignedFromVehicle(agent, vehicleComp);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	// Tries to find at least 'count' free fireteams. Returns true if it was able to achieve this amount of fireteams.
	bool FindFreeFireteams(notnull array<SCR_AIGroupFireteam> outFireteams, int count, typename ftTypename, array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		outFireteams.Clear();
		for (int i = 0; i < m_aFireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = m_aFireteams[i];
			
			if (ft.Type() != ftTypename)
				continue;
			
			if (!ft.IsLocked())
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					outFireteams.Insert(ft);
					if (outFireteams.Count() == count)
						return true;
				}
			}
		}
		
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------
	// Returns all free fireteams
	void GetFreeFireteams(notnull array<SCR_AIGroupFireteam> outFireteams, typename ftTypename, array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		outFireteams.Clear();
		for (int i = 0; i < m_aFireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = m_aFireteams[i];
			
			if (ft.Type() != ftTypename)
				continue;
			
			if (!ft.IsLocked())
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					outFireteams.Insert(ft);
				}
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Finds fireteam which has the provided agent
	SCR_AIGroupFireteam FindFireteam(AIAgent agent)
	{
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			if (ft.HasMember(agent))
				return ft;
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	void FindFireteamsOfVehicle(notnull array<ref SCR_AIGroupFireteam> outFireteams, notnull SCR_AIVehicleUsageComponent vehicleComp, typename ftType)
	{
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			SCR_AIGroupFireteamVehicleBase ftVehicle = SCR_AIGroupFireteamVehicleBase.Cast(ft);
			if (!ftVehicle)
				continue;
			if (ftVehicle.Type() != ftType)
				continue;
			if (ftVehicle.GetVehicle() == vehicleComp)
				outFireteams.Insert(ftVehicle);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	int GetFireteamCount()
	{
		return m_aFireteams.Count();
	}
	
	//---------------------------------------------------------------------------------------------------
	int GetFireteamId(notnull SCR_AIGroupFireteam ft)
	{
		foreach (int id, auto _ft : m_aFireteams)
		{
			if (_ft == ft)
				return id;
		}
		return -1;
	}
	
	//---------------------------------------------------------------------------------------------------
	void RebalanceAllFireteams()
	{
		// Rebalance all normal fireteams
		array<ref SCR_AIGroupFireteam> normalFireteams = {};
		foreach (auto ft : m_aFireteams)
		{
			if (ft.Type() == SCR_AIGroupFireteam)
				normalFireteams.Insert(ft);
		}
		
		if (RebalanceFireteams(normalFireteams, SCR_AIGroupFireteam))
			m_bRebalanceFireteams = false;
		
		// Rebalance all vehicle-associated cargo fireteams
		// For each vehicle, find all cargo fireteams associated with it
		map<SCR_AIVehicleUsageComponent, ref TAIFireteamArray> m = new map<SCR_AIVehicleUsageComponent, ref TAIFireteamArray>();
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			SCR_AIGroupFireteamVehicleCargo ftCargo = SCR_AIGroupFireteamVehicleCargo.Cast(ft);
			if (!ftCargo || !ftCargo.GetVehicle())
				continue;
			
			TAIFireteamArray ftArray;
			if (!m.Find(ftCargo.GetVehicle(), ftArray))
			{
				ftArray = new TAIFireteamArray();
				m.Insert(ftCargo.GetVehicle(), ftArray);
			}
			ftArray.Insert(ftCargo);
		}
		
		foreach (SCR_AIVehicleUsageComponent vehicleComp, TAIFireteamArray ftArray : m)
		{
			// All cargo fireteams of that vehicle are rebalanced together
			RebalanceFireteams(ftArray, SCR_AIGroupFireteamVehicleCargo);
			
			// All fireteams which have been created must be associated with this vehicle
			foreach (SCR_AIGroupFireteam ft : ftArray)
				SCR_AIGroupFireteamVehicleBase.Cast(ft).SetVehicle(vehicleComp);
		}
	}
	
	
	
	//---------------------------------------------------------------------------------------------------
	// PROTECTED / INTERNAL
	
	//---------------------------------------------------------------------------------------------------
	//! Creates a fireteam and registers it
	protected SCR_AIGroupFireteam CreateFireteam(notnull array<ref SCR_AIGroupFireteam> fireteams, typename t)
	{
		SCR_AIGroupFireteam ft = FireteamFactory(t);
		fireteams.Insert(ft);
		if (fireteams != m_aFireteams)
			m_aFireteams.Insert(ft);
		return ft;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static SCR_AIGroupFireteam FireteamFactory(typename t)
	{
		switch (t)
		{
			case SCR_AIGroupFireteam:
				return new SCR_AIGroupFireteam();
			case SCR_AIGroupFireteamVehicleCrew:
				return new SCR_AIGroupFireteamVehicleCrew();
			case SCR_AIGroupFireteamVehicleCargo:
				return new SCR_AIGroupFireteamVehicleCargo();
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void RemoveFireteam(notnull array<ref SCR_AIGroupFireteam> fireteams, SCR_AIGroupFireteam ft)
	{
		int id = fireteams.Find(ft);
		if (id == -1)
			return;
		
		if (ft.GetMemberCount() != 0)
		{
			Print("SCR_AIGroupUtilityComponent: removing a non-empty fireteam", LogLevel.ERROR);
			return;
		}
		
		Event_OnFireteamRemoved.Invoke(ft);
		
		fireteams.Remove(id);
		
		if (fireteams != m_aFireteams)
		{
			int _id = m_aFireteams.Find(ft);
			if (_id != -1)
				m_aFireteams.Remove(_id);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static SCR_AIGroupFireteam FindSmallestFireteam(notnull array<ref SCR_AIGroupFireteam> fireteams, typename fireteamTypename, array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		if (fireteams.IsEmpty())
			return null;
		
		int minSize = int.MAX;
		SCR_AIGroupFireteam outFt;
		for (int i = 0; i < fireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = fireteams[i];
			
			if (ft.Type() != fireteamTypename)
				continue;
			
			int size = ft.GetMemberCount();
			if (size < minSize)
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					minSize = size;
					outFt = ft;
				}
			}
		}
		
		return outFt;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static SCR_AIGroupFireteam FindBiggestFireteam(notnull array<ref SCR_AIGroupFireteam> fireteams, typename fireteamTypename, notnull array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		if (fireteams.IsEmpty())
			return null;
		
		int maxSize = int.MIN;
		SCR_AIGroupFireteam outFt;
		for (int i = 0; i < fireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = fireteams[i];
			
			if (ft.Type() != fireteamTypename)
				continue;
			
			int size = ft.GetMemberCount();
			if (size > maxSize)
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					maxSize = size;
					outFt = ft;
				}
			}
		}
		
		return outFt;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static int GetMaxFireteamSize(int groupSize)
	{
		if (groupSize >= 12)
			return 4;
		else if (groupSize > 4)
			return 3;
		else if (groupSize == 4)
			return 2; // When exactly 4 members, we want two fireteams of 2 members
		else if (groupSize == 3)
			return 2; // When 3 members, one fireteam of 2, one fireteam of 1
		else if (groupSize == 2)
			return 1;
		else
			return groupSize; // When below 4 members, one fireteam or 1-2
	}
	
	//---------------------------------------------------------------------------------------------------
	protected bool RebalanceFireteams(array<ref SCR_AIGroupFireteam> existingFireteams, typename ftTypename)
	{
		int agentsCount = 0;
		foreach (SCR_AIGroupFireteam ft : existingFireteams)
			agentsCount += ft.GetMemberCount();
		
		// Bail if group size is 0
		if (agentsCount == 0)
			return true;
				
		int maxFtSize = GetMaxFireteamSize(agentsCount);
		float fMaxFtSize = maxFtSize;
		float fAgentsCount = agentsCount;
		
		int desiredFtCount = Math.Ceil(fAgentsCount / fMaxFtSize);
		
		if (existingFireteams.Count() < desiredFtCount)
		{
			// Create new fireteams
			int newFtCount = desiredFtCount - existingFireteams.Count();
			for (int i = 0; i < newFtCount; i++)
				CreateFireteam(existingFireteams, ftTypename);
		}
		else if (existingFireteams.Count() > desiredFtCount)
		{
			// Delete fireteams ...
			
			int deleteFtCount = existingFireteams.Count() - desiredFtCount;
			array<SCR_AIGroupFireteam> fireteamsDelete = {};
			
			// Find smallelst fireteams for deletion
			for (int i = 0; i < deleteFtCount; i++)
			{
				SCR_AIGroupFireteam smallestFt = FindSmallestFireteam(existingFireteams, ftTypename, fireteamsDelete);
				fireteamsDelete.Insert(smallestFt);
			}
			
			// Move members from those selected fireteams, and delete them
			foreach (SCR_AIGroupFireteam fireteamDelete : fireteamsDelete)
			{
				SCR_AIGroupFireteam destinationFt = FindSmallestFireteam(existingFireteams, ftTypename, fireteamsDelete);
				destinationFt.MoveMembersFrom(fireteamDelete, fireteamDelete.GetMemberCount());
				RemoveFireteam(existingFireteams, fireteamDelete);
			}
		}
		
		array<SCR_AIGroupFireteam> fireteamsTooBig = {};
		array<SCR_AIGroupFireteam> fireteamsTooSmall = {};
		CountUnbalancedFireteams(existingFireteams, maxFtSize, fireteamsTooBig, fireteamsTooSmall);
		
		//array<SCR_AIGroupFireteam> fireteamsExclude = {};
		bool failed = false;
		int nIterations = 0;
		const int maxIterations = 128;
		while ((!fireteamsTooBig.IsEmpty() || !fireteamsTooSmall.IsEmpty()) && !failed && nIterations < maxIterations)
		{
			if (!fireteamsTooBig.IsEmpty())
			{
				// First split big fireteams
				SCR_AIGroupFireteam srcFt = fireteamsTooBig[0];
				
				// Take one big fireteam and move some members to smaller fireteams
				int nExcessMembers = srcFt.GetMemberCount() - maxFtSize;
				for (int i = 0; i < nExcessMembers; i++)
				{
					SCR_AIGroupFireteam dstFt = FindSmallestFireteam(existingFireteams, ftTypename, fireteamsTooBig);
					if (!dstFt)
					{
						// It shouldn't be possible
						Print(string.Format("SCR_AIGroupUtilityComponent: failed to reorganize fireteams, all other fireteams are full. %1",
							DiagGetFireteamsData()), LogLevel.ERROR);
						failed = true;
						break;
					}
					else
					{
						dstFt.MoveMembersFrom(srcFt, 1);
					}
				}
			}
			else if (!fireteamsTooSmall.IsEmpty())
			{
				// Second fill up the smallest fireteams
				SCR_AIGroupFireteam dstFt = fireteamsTooSmall[0];
				
				int nLackMembers = FIRETEAM_MIN_SIZE - dstFt.GetMemberCount(); // How many more members we need
				
				for (int i = 0; i < nLackMembers; i++)
				{
					SCR_AIGroupFireteam srcFt = FindBiggestFireteam(existingFireteams, ftTypename, fireteamsTooSmall);
					
					if (!srcFt)
					{
						// It shouldn't be possible in general case
						// It could only happen when group size is very small
						//if (agentsCount > maxFtSize)
							Print(string.Format("SCR_AIGroupUtilityComponent: failed to reorganize fireteams, all other fireteams are too small. %1",
								DiagGetFireteamsData()), LogLevel.ERROR);
						failed = true; // For very small group size (1) it might be impossible to make 'balanced' fireteams at all, due to lack of group members
						break;
					}
					else
					{
						dstFt.MoveMembersFrom(srcFt, 1);
					}
				}
				
			}
			
			CountUnbalancedFireteams(existingFireteams, maxFtSize, fireteamsTooBig, fireteamsTooSmall);
			nIterations++;
		}
		
		if (nIterations == maxIterations)
		{
			Print(string.Format("SCR_AIGroupUtilityComponent: RebalanceFireteams: max amount of iterations has been reached. %1", DiagGetFireteamsData()), LogLevel.ERROR);
		}
		
		return true;
	}
	protected static void CountUnbalancedFireteams(notnull array<ref SCR_AIGroupFireteam> inFireteams, int maxFtSize, notnull array<SCR_AIGroupFireteam> fireteamsTooBig, notnull array<SCR_AIGroupFireteam> fireteamsTooSmall)
	{
		fireteamsTooBig.Clear();
		fireteamsTooSmall.Clear();
		foreach (SCR_AIGroupFireteam ft : inFireteams)
		{
			int size = ft.GetMemberCount();
			if (size == maxFtSize) // Perfect
				continue;
			
			if (size > maxFtSize)
				fireteamsTooBig.Insert(ft);
			else if (size < FIRETEAM_MIN_SIZE)
				fireteamsTooSmall.Insert(ft); // Includes empty or with one member
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Returns string with data about fireteams
	string DiagGetFireteamsData()
	{
		string s = string.Format("Fireteams: %1: ", m_aFireteams.Count());
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			string strLocked = string.Empty;
			if (ft.IsLocked())
				strLocked = "L";
			
			s = s + string.Format("%1%2, ", ft.GetMemberCount(), strLocked);
		}
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	void DiagDrawFireteams()
	{
		int bgColors[8] = {
			Color.DARK_RED,
			Color.DARK_GREEN,
			Color.DARK_BLUE,
			Color.DARK_CYAN,
			Color.DARK_MAGENTA,
			Color.DARK_YELLOW,
			Color.DODGER_BLUE,
			Color.VIOLET
		};
		const int bgColorsCount = 8;
		
		/*
			static const int DARK_BLUE = 0xff000080; // navy
			static const int DARK_CYAN = 0xff008080; // teal
			static const int DARK_MAGENTA = 0xff800080; // purple
			static const int DARK_YELLOW = 0xff808000; // olive
		};
		*/
		
		array<AIAgent> members = {};
		foreach (int fireteamId, SCR_AIGroupFireteam ft : m_aFireteams)
		{
			ft.GetMembers(members);
			int bgColor = bgColors[fireteamId % bgColorsCount];
			foreach (AIAgent agent : members)
			{
				IEntity e = agent.GetControlledEntity();
				if (!e)
					continue;
				vector textPos = e.GetOrigin() + Vector (0, 0.5, 0);
				
				string strFtType;
				if (SCR_AIGroupFireteamVehicleCrew.Cast(ft))
				{
					strFtType = "Crew  ";
					textPos[1] = textPos[1] + 0.25;
				}
				else if (SCR_AIGroupFireteamVehicleCargo.Cast(ft))
				{
					strFtType = "Cargo ";
					textPos[1] = textPos[1] + 0.5;
				}
					
				string text = string.Format("%1FT: %2", strFtType, fireteamId);
				DebugTextWorldSpace.Create(GetGame().GetWorld(), text, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
					textPos[0], textPos[1], textPos[2], color: Color.WHITE, bgColor: bgColor,
					size: 13.0); 
			}
		}
	}
}