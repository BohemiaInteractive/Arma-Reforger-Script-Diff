[BaseContainerProps(configRoot: true)]
class SCR_FactionHomeTerritoryConfig
{
	[Attribute(desc: "Array of all locations from which the people of this factions can be from")]
	protected ref array<ref SCR_FactionHomeTerritory> m_aFactionHomeTerritories;
	
	[Attribute("0.1", desc: "The percentage chance of a AI group to all share the same place of birth as leader. The higher this number the higher the chance", params: "0 1")]
	protected float m_fSharedPlaceOfBirthWeight;
	
	//~ Total weight of all entries
	protected int m_iTotalWeight;
	
	//------------------------------------------------------------------------------------------------
	//! Get UI info of place of birth
	//! \param[in] useWeightedRandom if true it will randomize the entries with the given weight
	//! \return Random (weighted) index
	SCR_UIInfo GetHomeTerritoryUIInfo(int index)
	{
		if (!m_aFactionHomeTerritories.IsIndexValid(index))
			return null;
		
		return m_aFactionHomeTerritories[index].m_UIInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get random place of birth index of faction using random Generator
	//! \param[in] random Generator. Set null it should not use Generator
	//! \param[in] useWeightedRandom if true it will randomize the entries with the given weight
	//! \return Random (weighted) index
	int GetRandomHomeTerritoryIndex(RandomGenerator randomizer = null, bool useWeightedRandom = true)
	{
		if (useWeightedRandom && m_iTotalWeight <= 0)
			return -1;
		
		int count = m_aFactionHomeTerritories.Count();
		
		if (count == 0)
			return -1;
		
		if (!useWeightedRandom)
		{
			if (randomizer)
				return randomizer.RandInt(0, count);
			else 
				return Math.RandomInt(0, count);
		}
			
		//~ Randomize with total weights
		int randomWeight;
		
		if (randomizer)
			randomWeight = randomizer.RandIntInclusive(0, m_iTotalWeight);
		else 
			randomWeight = Math.RandomIntInclusive(0, m_iTotalWeight);
		
		int checkedWeight = 0;
		
		foreach (int i, SCR_FactionHomeTerritory territory : m_aFactionHomeTerritories)
		{
			//~ Last in list no need to calculate
			if (i == count -1)
				return i;
			
			checkedWeight += territory.m_iWeight;
			if (randomWeight <= checkedWeight)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get percentage of AI group members sharing the same place of birth
	//! \return Percentage
	float GetSharedPlaceOfBirthWeight()
	{
		return m_fSharedPlaceOfBirthWeight;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_FactionHomeTerritoryConfig()
	{
		m_iTotalWeight = 0;
		
		//~ Get all weights
		foreach (SCR_FactionHomeTerritory homeTerritory: m_aFactionHomeTerritories)
		{
			m_iTotalWeight += homeTerritory.m_iWeight;
		}
	}
}

[BaseContainerProps(), BaseContainerCustomDoubleTitleField("m_sID", "m_iWeight")]
class SCR_FactionHomeTerritory
{
	[Attribute(desc: "Unique ID for place of birth")]
	string m_sID;
	
	[Attribute(desc: "UI Info for place of birth")]
	ref SCR_UIInfo m_UIInfo;
	
	[Attribute ("1", desc: "Weight of place of birth for randomization. Higher number means higher chance of being picked")]
	int m_iWeight;
	
	[Attribute(desc: "EACH GENDER SHOULD ONLY HAVE ONE ENTRY IN THIS LIST! List of genders that contain identity overrides that have a chance to be assigned to a character that is part of this terratory")]
	ref array<ref SCR_TerritoryIdentityOverride> m_aTerritoryIdentityOverrides;
}

[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleEnum(SCR_EIdentityGender, "m_Gender")]
class SCR_TerritoryIdentityOverride
{
	[Attribute("1", desc: "Gender of overwrite. The character needs to be the set gender to have the override assigned.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EIdentityGender))]
	SCR_EIdentityGender m_eGender;
	
	[Attribute(desc: "List of visuals that have a chance to be assigned to the character that is part of this Territory")]
	ref array<ref SCR_TerritoryIdentityVisualOverride> m_aVisualOverrides;
	
	[Attribute(desc: "List of first names that have a chance to be assigned to the character that is part of this Territory")]
	ref SCR_TerritoryIdentityNameOverride m_FirstNameOverrides;
	
	[Attribute(desc: "List of surnames that have a chance to be assigned to the character that is part of this Territory")]
	ref SCR_TerritoryIdentityNameOverride m_SurnameOverrides;
}

[BaseContainerProps()]
class SCR_TerritoryIdentityVisualOverride
{
	//~ Unused atm
	/*[Attribute(desc: "Body of this visual override")]
	ResourceName m_sBodyOverride;
	
	[Attribute(desc: "Faces of this visual override. Each face has a random chance to be assigned")]
	ref array<ResourceName> m_aFaceOverride;
	
	[Attribute("1", desc: "Weight for this Visual to be used as an override. Take into account that multiple visuals. 1 means 100% (or multiple with one means multiple with 100%. If two entries have 0.5 then the calculation is as following (if calc <= ArrayCount Pick) So in the case it is a 25% to pick A, 25% to pick and 50% to pick default)", params: "0 1 0.001")]
	float m_fWeight;*/
}

[BaseContainerProps()]
class SCR_TerritoryIdentityNameOverride
{
	[Attribute(desc: "List of names that can be overridden. Each name has a random chance to be assigned")]
	ref array<LocalizedString> m_aNames;
	
	[Attribute("1", desc: "Weight for a random Territory name to be chosen. 1 mean 100%", params: "0 1 0.001")]
	float m_fChanceToOverrideName;
}
