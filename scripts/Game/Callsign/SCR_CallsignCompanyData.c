/*!
For each company, holds availible platoons, which in turn holds availible squads
*/
class SCR_CallsignCompanyData
{
	protected ref map<int, ref SCR_CallsignPlatoonData> m_mPlatoonCallsigns = new map<int, ref SCR_CallsignPlatoonData>;

	//---------------------------------------- On Init ----------------------------------------\\
	/*!
	Init Company data, creating all the platoons within which in turn create the availible squads within
	\param factionCallsignInfo specific callsign info for faction
	*/
	void Init(SCR_FactionCallsignInfo factionCallsignInfo)
	{
		array<ref SCR_CallsignInfo> platoonArray = new array<ref SCR_CallsignInfo>;
		factionCallsignInfo.GetPlatoonArray(platoonArray);
		int count = platoonArray.Count();

		for(int i = 0; i < count; i++)
		{
			SCR_CallsignPlatoonData platoonData = SCR_CallsignPlatoonData();
			platoonData.Init(factionCallsignInfo);
			m_mPlatoonCallsigns.Insert(i, platoonData);
		}
	}

	//---------------------------------------- Get Random Callsign ----------------------------------------\\
	/*!
	Gets platoon and squad index randomly from availible callsigns
	\param[out] platoon index
	\param[out] squad index
	\return bool true if succesfully found a callsign
	*/
	void GetRandomCallsign(out int platoon, out int squad)
	{
		int random = Math.RandomInt(0, m_mPlatoonCallsigns.Count());
		platoon = m_mPlatoonCallsigns.GetKey(random);
		SCR_CallsignPlatoonData randomPlatoon = m_mPlatoonCallsigns.GetElement(random);

		squad = randomPlatoon.GetRandomSquad();
	}

	//---------------------------------------- Get first availible Callsign ----------------------------------------\\
	/*!
	Goes through all availible platoons and squads and gets the first availible. Meaning if platoon 1 is taken it will get 2 next not 3.
	\param[out] platoon index
	\param[out] squad index
	\return bool true if succesfully found a callsign
	*/
	void GetFirstAvailibleCallsign(out int firstAvailiblePlatoon, out int firstAvailibleSquad)
	{
		firstAvailiblePlatoon = int.MAX;
		SCR_CallsignPlatoonData firstAvailiblePlatoonData;

		foreach (int index, SCR_CallsignPlatoonData platoon: m_mPlatoonCallsigns)
		{
			if (index < firstAvailiblePlatoon)
			{
				firstAvailiblePlatoon = index;
				firstAvailiblePlatoonData = platoon;
			}
		}

		firstAvailibleSquad = firstAvailiblePlatoonData.GetFirstAvailibleSquad();
	}

	//---------------------------------------- Add availible Callsign ----------------------------------------\\
	/*!
	Adds callsign back to availible callsign pool
	\param platoon index
	\param squad index
	*/
	void AddCallsign(int platoonIndex, int squadIndex)
	{
		SCR_CallsignPlatoonData platoonData;

		if (m_mPlatoonCallsigns.Find(platoonIndex, platoonData))
		{
			platoonData.AddSquad(squadIndex);
		}
		else
		{
			platoonData = new SCR_CallsignPlatoonData();
			platoonData.AddSquad(squadIndex);
			m_mPlatoonCallsigns.Insert(platoonIndex, platoonData);
		}
	}

	//---------------------------------------- Remove availible Callsign ----------------------------------------\\
	/*!
	Removes callsign from availible callsign pool
	\param platoon index
	\param squad index
	*/
	bool RemoveCallsign(int plattonIndex, int squadIndex)
	{
		SCR_CallsignPlatoonData platoonData;

		if (m_mPlatoonCallsigns.Find(plattonIndex, platoonData))
		{
			//Check if all squads are used then remove platoon
			if (platoonData.RemoveSquad(squadIndex))
			{
				m_mPlatoonCallsigns.Remove(plattonIndex);
				return m_mPlatoonCallsigns.IsEmpty();
			}
		}

		return false;
	}
};