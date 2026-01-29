//! For each faction, holds available companies, which in turn hold available platoons, which in turn hold available squads
class SCR_FactionCallsignData
{
	protected ref map<int, ref SCR_CallsignCompanyData> m_mCompanyCallsigns = new map<int, ref SCR_CallsignCompanyData>();
	protected ref map<int, ref SCR_CallsignCompanyData> m_mCompanyOverflowCallsigns = new map<int, ref SCR_CallsignCompanyData>();

	protected int m_iOverflowIndex;
	protected bool m_bRandomizedCallsigns;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] factionCallsignInfo
	void SCR_FactionCallsignData(SCR_FactionCallsignInfo factionCallsignInfo)
	{
		m_iOverflowIndex = factionCallsignInfo.GetCompanyOverflowIndex();
		m_bRandomizedCallsigns = factionCallsignInfo.GetIsAssignedRandomly();

		array<ref SCR_CallsignInfo> companyArray = {};
		factionCallsignInfo.GetCompanyArray(companyArray);
		int count = companyArray.Count();

		for(int i = 0; i < count; i++)
		{
			SCR_CallsignCompanyData companyData = SCR_CallsignCompanyData();
			companyData.Init(factionCallsignInfo);

			//Check if overflow company and add it to correct map
			if (i < m_iOverflowIndex)
				m_mCompanyCallsigns.Insert(i, companyData);
			//Overflow means it only grabs these if non of the default companies are available
			else
				m_mCompanyOverflowCallsigns.Insert(i, companyData);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets company, platoon and squad index randomly from available callsigns
	//! Will first go through default companies, if these are all taken then go though overflow companies
	//! \param[out] company index
	//! \param[out] platoon index
	//! \param[out] squad index
	//! \return true if succesfully found a callsign
	bool GetRandomCallsign(out int company, out int platoon, out int squad)
	{
		SCR_CallsignCompanyData randomCompany;
		int random;

		//Default companies still available
		if (!m_mCompanyCallsigns.IsEmpty())
		{
			random = Math.RandomInt(0, m_mCompanyCallsigns.Count());
			company = m_mCompanyCallsigns.GetKey(random);
			randomCompany = m_mCompanyCallsigns.GetElement(random);
		}

		//Use overflow companies
		else if (!m_mCompanyOverflowCallsigns.IsEmpty())
		{
			random = Math.RandomInt(0, m_mCompanyOverflowCallsigns.Count());
			company = m_mCompanyOverflowCallsigns.GetKey(random);
			randomCompany = m_mCompanyOverflowCallsigns.GetElement(random);
		}
		//No companies available
		else
		{
			return false;
		}

		randomCompany.GetRandomCallsign(platoon, squad);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Goes through all available companies, platoons and squads and gets the first available. Meaning if platoon 1 is taken it will get 2 next not 3.
	//! Will first go through default companies, if these are all taken then go though overflow companies
	//! \param[out] firstAvailableCompany company index
	//! \param[out] firstAvailablePlatoon platoon index
	//! \param[out] firstAvailableSquadsquad index
	//! \return true if succesfully found a callsign
	// TODO: availible -> available
	bool GetFirstAvailibleCallsign(out int firstAvailableCompany, out int firstAvailablePlatoon, out int firstAvailableSquad)
	{
		firstAvailableCompany = int.MAX;
		SCR_CallsignCompanyData firstAvailableCompanyData;

		//Default companies still available
		if (!m_mCompanyCallsigns.IsEmpty())
		{
			foreach (int index, SCR_CallsignCompanyData company: m_mCompanyCallsigns)
			{
				if (index < firstAvailableCompany)
				{
					firstAvailableCompany = index;
					firstAvailableCompanyData = company;
				}
			}
		}
		//Use overflow companies
		else if (!m_mCompanyOverflowCallsigns.IsEmpty())
		{
			foreach (int index, SCR_CallsignCompanyData company: m_mCompanyOverflowCallsigns)
			{
				if (index < firstAvailableCompany)
				{
					firstAvailableCompany = index;
					firstAvailableCompanyData = company;
				}
			}
		}
		//No companies available
		else
		{
			return false;
		}

		firstAvailableCompanyData.GetFirstAvailibleCallsign(firstAvailablePlatoon, firstAvailableSquad);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets a callsign for a specific company given.
	//! Will however use either GetRandomCallsign or GetFirstAvailibleCallsign (depending on callsign settings) if specific company is not available
	//! \param[out] company company index
	//! \param[out] platoon platoon index
	//! \param[out] squad squad index
	//! \return true if successfully found a callsign (either the first available or GetRandomCallsign/GetFirstAvailibleCallsign)
	bool GetSpecificCompanyCallsign(out int company, out int platoon, out int squad)
	{
		 SCR_CallsignCompanyData companyData;

		//Get specific company
		if (!m_mCompanyCallsigns.Find(company, companyData))
			m_mCompanyOverflowCallsigns.Find(company, companyData);

		//Specific company found
		if (companyData)
		{
			if (!m_bRandomizedCallsigns)
				companyData.GetFirstAvailibleCallsign(platoon, squad);
			else
				companyData.GetRandomCallsign(platoon, squad);
		}
		//Company not available, use default logics
		else
		{
			if (!m_bRandomizedCallsigns)
				return GetFirstAvailibleCallsign(company, platoon, squad);
			else
				return GetRandomCallsign(company, platoon, squad);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds callsign back to available callsign pool
	//! \param[in] companyIndex company index
	//! \param[in] platoonIndex platoon index
	//! \param[in] squadIndex squad index
	void AddCallsign(int companyIndex, int platoonIndex, int squadIndex)
	{
		SCR_CallsignCompanyData companyData;

		if (companyIndex < m_iOverflowIndex)
		{
			if (m_mCompanyCallsigns.Find(companyIndex, companyData))
			{
				companyData.AddCallsign(platoonIndex, squadIndex);
			}
			else
			{
				companyData = new SCR_CallsignCompanyData();
				companyData.AddCallsign(platoonIndex, squadIndex);
				m_mCompanyCallsigns.Insert(companyIndex, companyData);
			}
		}
		else
		{
			if (m_mCompanyOverflowCallsigns.Find(companyIndex, companyData))
			{
				companyData.AddCallsign(platoonIndex, squadIndex);
			}
			else
			{
				companyData = new SCR_CallsignCompanyData();
				companyData.AddCallsign(platoonIndex, squadIndex);
				m_mCompanyOverflowCallsigns.Insert(companyIndex, companyData);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes callsign from available callsign pool
	//! \param[in] company index
	//! \param[in] platoon index
	//! \param[in] squad index
	void RemoveCallsign(int companyIndex, int platoonIndex, int squadIndex)
	{
		SCR_CallsignCompanyData companyData;

		if (companyIndex < m_iOverflowIndex)
		{
			if (m_mCompanyCallsigns.Find(companyIndex, companyData))
			{
				//Check if all platoons are used then remove company
				if (companyData.RemoveCallsign(platoonIndex, squadIndex))
				{
					m_mCompanyCallsigns.Remove(companyIndex);
				}
			}
		}
		else
		{
			if (m_mCompanyOverflowCallsigns.Find(companyIndex, companyData))
			{
				//Check if all platoons are used then remove company
				if (companyData.RemoveCallsign(platoonIndex, squadIndex))
				{
					m_mCompanyOverflowCallsigns.Remove(companyIndex);
				}
			}
		}
	}
}
