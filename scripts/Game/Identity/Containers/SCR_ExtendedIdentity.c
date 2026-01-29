[BaseContainerProps()]
class SCR_ExtendedIdentity
{
	[Attribute("-1", desc: "Leave -1 to auto assign.", params: "-1 inf 1")]
	protected int m_iAge;
	
	[Attribute("-1", desc: "Day character is born or entity is created. Leave -1 to auto assign.", params: "-1 31 1")]
	protected int m_iDayOfCreation;
	
	[Attribute("-1", desc:  "Month character is born or entity is created. Leave -1 to auto assign.", params: "-1 12 1")]
	protected int m_iMonthOfCreation;
	
	[Attribute(desc: "Original faction of entity. Leave empty to auto assign (Uses AIGroup or FactionAffiliationComponent to get faction). This is used to get random place of creation/birth within the faction.")]
	protected string m_sFactionOfOriginKey;
	
	[Attribute(desc: "Birth or creation place. Leave empty to auto assign.")]
	protected ref SCR_UIInfo m_PlaceOfOrigin;
	
	protected int m_iFactionOfOriginIndex = -1;
	protected int m_iPlaceOfOriginIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	//! Set age of entity
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! \param[i] age Age of entity
	void SetAge(int age)
	{
		m_iAge = age;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get age of entity
	//! \return Age of entity
	int GetAge()
	{
		return m_iAge;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set day entity is created or character is born
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! \param[in] day Day born/created
	void SetDayOfCreation(int day)
	{
		m_iDayOfCreation = day;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get day entity is created or character is born
	//! \return Day born/created
	int GetDayOfCreation()
	{
		return m_iDayOfCreation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set month entity is created or character is born
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! \param[in] month Day born/created
	void SetMonthOfCreation(int month)
	{
		m_iMonthOfCreation = month;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get month entity is created or character is born
	//! \return Month born/created
	int GetMonthOfCreation()
	{
		return m_iMonthOfCreation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction of origin. Used to get random place of creation/birth
	//! This should never change once set even if faction is changed
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! \param[in] factionKey Faction key
	void SetFactionOfOrigin(string factionKey)
	{
		m_sFactionOfOriginKey = factionKey.Trim();
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByKey(m_sFactionOfOriginKey);
		if (!faction)
			return;
		
		m_iFactionOfOriginIndex = factionManager.GetFactionIndex(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction of origin. Used to get random place of creation/birth
	//! This should never change once set even if faction is changed
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! \param[in] factionIndex Faction index
	void SetFactionOfOrigin(int factionIndex)
	{
		m_iFactionOfOriginIndex = factionIndex;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(m_iFactionOfOriginIndex);
		if (!faction)
			return;
		
		m_sFactionOfOriginKey = faction.GetFactionKey();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get faction of origin key. Used to get random place of creation/birth
	//! This should never change once set even if faction is changed
	//! \return Faction of origin key
	string GetFactionOfOriginKey()
	{
		//~ Set if not yet set 
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sFactionOfOriginKey) && m_iFactionOfOriginIndex > -1)
			SetFactionOfOrigin(m_iFactionOfOriginIndex);
	
		return m_sFactionOfOriginKey.Trim();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get faction of origin index. Used to get random place of creation/birth
	//! This should never change once set even if faction is changed
	//! \return Faction of origin index
	int GetFactionOfOriginIndex()
	{
		//~ Set if not yet set
		if (m_iFactionOfOriginIndex <= -1 && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sFactionOfOriginKey))
			SetFactionOfOrigin(m_sFactionOfOriginKey);
		
		return m_iFactionOfOriginIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set place of creation. This is the birth place of a character or place of creation of an entity
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! FactionOfOrigin needs to be set to use index.
	//! \param[in] place UIInfo to directly set. Is only set when provided.
	//! \param[in] isOverwrite If true will set m_iPlaceOfOriginIndex to -1
	void SetPlaceOfOrigin(SCR_UIInfo place, bool isOverwrite)
	{
		m_PlaceOfOrigin = place;
		
		if (isOverwrite)
			m_iPlaceOfOriginIndex = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set place of creation. This is the birth place of a character or place of creation of an entity
	//! Use SCR_ExtendedIdentityComponent to set to make sure it is replicated!
	//! FactionOfOrigin needs to be set to use index.
	//! \param[in] index index to get place of creation from faction.
	void SetPlaceOfOriginIndex(int index)
	{		
		m_iPlaceOfOriginIndex = index;
		
		if (index < 0)
			return;
		
		int factionOfOrigin = GetFactionOfOriginIndex();
		if (factionOfOrigin < 0)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionOfOrigin));
		if (!scrFaction)
			return;
		
		SCR_FactionHomeTerritoryConfig territory = scrFaction.GetFactionHomeTerritoryConfig();
		if (!territory)
			return;
		
		SCR_UIInfo uinfo = territory.GetHomeTerritoryUIInfo(m_iPlaceOfOriginIndex);
		if (!uinfo)
			return;
		
		SetPlaceOfOrigin(uinfo, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the place of creation UI info
	//! \return Place of creation UI info
	SCR_UIInfo GetPlaceOfOriginUIInfo()
	{
		return m_PlaceOfOrigin;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the place of creation index. Can be used with SCR_Faction to obtain the place of creation
	//! \return Place of creation index
	int GetPlaceOfOriginIndex()
	{
		return m_iPlaceOfOriginIndex;
	}
}