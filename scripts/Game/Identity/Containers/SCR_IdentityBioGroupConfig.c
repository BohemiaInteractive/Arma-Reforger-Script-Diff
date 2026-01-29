[BaseContainerProps(configRoot: true), BaseContainerCustomDoubleTitleField("m_sBioGroupID", "m_iWeight")]
class SCR_IdentityBioGroupConfig
{
	[Attribute("1", desc: "If false will disable this entry from the pool.", category: "Settings")]
	protected bool m_bEnabled;
	
	[Attribute(desc: "Unique ID of group", category: "Settings")]
	protected string m_sBioGroupID;
	
	[Attribute("1", desc: "The higher the weight the heigher the chance of a bio in this group being picked by the randomizer (Compared with the other groups in the IdentityManager). Bio's in group will never be randomly assigned nor keep track of assigned bio's if weight is 0 or less. Use this for custom identities you want to assign only through script.\n\nWARNING: All weights (as well as aditional entry weight * list lenght) are counted up together. ake make sure all totals do not exceed max int value: '2147483647'", params: "-1, 9999 1", category: "Settings")]
	protected int m_iWeight; //0 is never chosen by randomizer. Only set by system
	
	[Attribute("10", desc: "Additional Weight for the amount of entries the BioGroup has. Calculation: (m_iWeight = m_iWeight + (entries * m_fEntryAmountWeightMulti)) This is to make sure that groups with lesser entries are not chosen as much\n\nWARNING: All weights (as well as aditional entry weight * list lenght) are counted up together. Make sure all totals do not exceed max int value: '2147483647'", params: "0, 100", category: "Settings")]
	protected float m_fEntryAmountWeightMulti;
	
	[Attribute(desc: "Faction the entity needs in order for it to choose the bio. Leave empty if any faction (or no faction) is valid.", category: "Requirements")]
	protected ref array<string> m_aNeedsFaction;
	
	[Attribute(desc: "List of all character bio's in group (Gender Neutral). If the bio group is chosen in the randomizer then one of the bios within will be randomly assigned.", category: "Identities")]
	protected ref array<ref SCR_IdentityBio> m_aIdentityList;
	
	//~ Indexes of m_aNeedsFaction
	protected ref array<int> m_aNeedsFactionIndexes;
	
	//~ Bio's that can be picked
	protected ref array<int> m_aAvailableIdentityBioIndexList = {};
	
	//~ Bio's that cannot be picked but will be resuffled once available is empty
	protected ref array<int> m_aUnavailableIdentityBioIndexList = {};
	
	//------------------------------------------------------------------------------------------------
	//! Check if Bio Config is enabled. If not it will never be randomized.
	//! \return If config is enabled
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! General check if group is valid for randomization
	//! param[in] entity Entity which will get a identity
	//! param[in] extendedIdentity Extended Identity Component of the entity
	//! \return Returns true if the group has a weight and if there are available identities's
	bool IsValidForRandomization(IEntity entity, SCR_ExtendedIdentityComponent extendedIdentity)
	{	
		//~ Note that this is in part checked in SCR_CharacterIdentityBioGroupConfig
		return m_iWeight > 0 && !m_aAvailableIdentityBioIndexList.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return randomization weight
	//! \return randomization weight
	int GetWeight()
	{
		return m_iWeight;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is Valid faction
	//! \param[in] factionKey faction key
	//! \return Returns true if no faction needed for group if if given faction is within the needed faction list
	bool IsValidFaction(string factionKey)
	{
		if (m_aNeedsFaction.IsEmpty())
			return true;
		
		return m_aNeedsFaction.Contains(factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is Valid faction
	//! \param[in] factionIndex faction Index
	//! \return Returns true if no faction needed for group if if given faction is within the needed faction list
	bool IsValidFaction(int factionIndex)
	{
		if (!m_aNeedsFactionIndexes || m_aNeedsFactionIndexes.IsEmpty())
			return true;
		
		return m_aNeedsFactionIndexes.Contains(factionIndex);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get bio group ID
	//! \return Bio groupID
	string GetBioGroupID()
	{
		return m_sBioGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get an bio from Index
	//! \param[in] entity The entity that has the identity
	//! \param[in] index Index of bio
	//! \return Bio
	SCR_IdentityBio GetIdentityBio(IEntity entity, int index)
	{		
		if (!m_aIdentityList.IsIndexValid(index))
			return null;
		
		return m_aIdentityList[index]; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get an random available bio (Server Only)
	//! Also removes the bio from available list
	//! \param[out] index Bio index
	//! \param[out] bio Bio reference
	void AssignRandomAvailableBio(RandomGenerator randomizer, IEntity entity, out int index, out SCR_IdentityBio bio)
	{
		if (m_aAvailableIdentityBioIndexList.IsEmpty())
			return;
		
		//~ Get random entry
		int count = m_aAvailableIdentityBioIndexList.Count();
		int random = randomizer.RandInt(0, count);
		
		//~ Get actual index of randomized available bio
		index = m_aAvailableIdentityBioIndexList[random];		
		//~ Remove from available bios	
		bio = OnCharacterBioAssigned(entity, index);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Character bio is assigned (Server Only)
	//! Also removes the bio from available list (Unless the group has no weight and thus need to be randonly assigned)
	//! \param[in] index Index of assigned Bio
	SCR_IdentityBio OnCharacterBioAssigned(IEntity entity, int index)
	{		
		if (index < 0 || index >= m_aIdentityList.Count())
			return null;
		
		SCR_IdentityBio bio = m_aIdentityList[index];
		if (!bio)
			return null;
		
		//~ Weight zero will never remove from available
		if (m_iWeight <= 0)
			return bio;
		
		//~ Remove from available bios	
		m_aAvailableIdentityBioIndexList.RemoveItem(index);
		
		//~ Add to unavailable bios (If not unique as unique bio's can only be assigned once per game)
		if (!bio.IsUnique() && !m_aUnavailableIdentityBioIndexList.Contains(index))
			m_aUnavailableIdentityBioIndexList.Insert(index);
		
		//~ Refill available list if empty
		if (m_aAvailableIdentityBioIndexList.IsEmpty())
		{
			foreach (int unavailableIndex : m_aUnavailableIdentityBioIndexList)
			{
				m_aAvailableIdentityBioIndexList.Insert(unavailableIndex);
			}
			
			m_aUnavailableIdentityBioIndexList.Clear();
		}
		
		return bio;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resets all available indexes. Including Unique (Server Only)
	void ResetAvailable()
	{
		m_aAvailableIdentityBioIndexList.Clear();
		m_aUnavailableIdentityBioIndexList.Clear();
		
		int count = m_aIdentityList.Count();
		
		for(int i = 0; i < count; i++)
		{
            m_aAvailableIdentityBioIndexList.Insert(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Init
	void SCR_IdentityBioGroupConfig()
	{
		if (SCR_Global.IsEditMode()) 
			return; 
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode || !gamemode.IsMaster())
			return;
		
		//~ Init available character indexes		
		ResetAvailable();
		GetGame().GetCallqueue().CallLater(DelayedInit);
		
		SetAdditionalWeight();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAdditionalWeight()
	{
		//~ Set Additional Weight for each entry
		if (m_iWeight > 0 && m_fEntryAmountWeightMulti > 0)
			m_iWeight = m_iWeight + (m_aIdentityList.Count() * m_fEntryAmountWeightMulti);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit()
	{
		//~ Highly unlikely that this happens but will send an error if it happens
		if (m_aIdentityList.Count() > SCR_IdentityManagerComponent.MAX_IDENTITY_ENTRIES)
		{
			Print(string.Format("'SCR_IdentityBioGroupConfig' - %1: Bio entries count is greater than %2 which will break replication", m_sBioGroupID, SCR_IdentityManagerComponent.MAX_IDENTITY_ENTRIES), LogLevel.ERROR);
		}
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Print("'SCR_CharacterBioGroupConfig' Needs faction manager else all bio's can be assigned to every faction (or no faction)", LogLevel.WARNING);
			return;
		}
		
		Faction faction;
		
		foreach(string factionKey: m_aNeedsFaction)
		{
			if (!m_aNeedsFactionIndexes)
				m_aNeedsFactionIndexes = {};
			
			faction = factionManager.GetFactionByKey(factionKey);
			
			//~ If faction not found in the FactionManager make sure it is never used in the randomization by setting it to -1
			if (!faction && !m_aNeedsFactionIndexes.Contains(-1))
				m_aNeedsFactionIndexes.Insert(-1);
			
			m_aNeedsFactionIndexes.Insert(factionManager.GetFactionIndex(faction));
		}
	}
}