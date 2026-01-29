/*!
For each platoon, holds availible squads
*/
class SCR_CallsignPlatoonData
{
	protected ref array<int> m_mSquadCallsigns = new array<int>;
	
	//---------------------------------------- On Init ----------------------------------------\\
	/*!
	Init Platoon data, creating all the squads within
	\param factionCallsignInfo specific callsign info for faction
	*/
	void Init(SCR_FactionCallsignInfo factionCallsignInfo)
	{		
		array<ref SCR_CallsignInfo> squadArray = new array<ref SCR_CallsignInfo>;
		factionCallsignInfo.GetSquadArray(squadArray);
		int count = squadArray.Count();
		
		for(int i = 0; i < count; i++)
		{
			m_mSquadCallsigns.Insert(i);
		}
	}
	
	//---------------------------------------- Get random squad Callsign ----------------------------------------\\
	/*!
	Gets a squad index randomly from availible callsigns
	\return int squad index
	*/ 
	int GetRandomSquad()
	{
		return m_mSquadCallsigns[Math.RandomInt(0, m_mSquadCallsigns.Count())];
	}
	
	/*!
	Goes through all availible squads gets the first availible. Meaning if squad 1 is taken it will get 2 next not 3.
	\return int squad index
	*/ 
	int GetFirstAvailibleSquad()
	{
		int firstAvailible = int.MAX; 
		
		foreach (int squad: m_mSquadCallsigns)
		{
			if (squad < firstAvailible)
				firstAvailible = squad;
		}
		
		return firstAvailible;
	}
	
	//---------------------------------------- Add availible squad Callsign ----------------------------------------\\
	/*!
	Adds callsign back to availible callsign pool
	\param squad index
	*/ 
	void AddSquad(int squadIndex)
	{
		if (!m_mSquadCallsigns.Contains(squadIndex))
			m_mSquadCallsigns.Insert(squadIndex);
	}
	
	//---------------------------------------- Remove availible squad Callsign ----------------------------------------\\
	/*!
	Removes callsign from availible callsign pool
	\param squad index
	*/ 
	bool RemoveSquad(int squadIndex)
	{
		int count = m_mSquadCallsigns.Count();
		
		for(int i = 0; i < count; i++)
		{
			if (m_mSquadCallsigns[i] == squadIndex)
			{
				m_mSquadCallsigns.Remove(i);
				break;
			}
		}
		
		return m_mSquadCallsigns.IsEmpty();
	}
};