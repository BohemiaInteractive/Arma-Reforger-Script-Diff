[BaseContainerProps(configRoot: true), BaseContainerCustomDoubleTitleField("m_sBioGroupID", "m_iWeight")]
class SCR_CharacterIdentityBioGroupConfig: SCR_IdentityBioGroupConfig
{		
	[Attribute(desc: "A list of Bio Lists that are specific for a gender (other then NEUTRAL). Each list needs a specific gender, never have two list holders with the same gender!", category: "Identities")]
	protected ref array<ref SCR_GenderSpecificIdentityBios> m_aGenderSpecificIdentityLists;
	
	[Attribute(SCR_ECharacterIdentityBioGroupType.MILITARY_AND_CIVILIAN.ToString(), desc: "If the identity is valid for Military and/or Civilian factions. Note that giving a Civilian only idientity to a military faction (added in the Needs Faction list) will cause the bio's to never be randomized", uiwidget:  UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ECharacterIdentityBioGroupType), category: "Requirements")]
	protected SCR_ECharacterIdentityBioGroupType m_eIdentityGroupType;
	
	[Attribute(SCR_EIdentityCharacterControlType.PLAYER_AND_AI.ToString(), desc: "Who can have the identities? Players and AI or just Players/AI only", uiwidget:  UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EIdentityCharacterControlType), category: "Requirements")]
	protected SCR_EIdentityCharacterControlType m_eCharacterControlTypes;
	
	protected ref map<SCR_EIdentityGender, ref array<int>> m_mAvailableGenderSpecificBioIndexLists;
	protected ref map<SCR_EIdentityGender, ref array<int>> m_mUnavailableGenderSpecificBioIndexLists;
	
	//------------------------------------------------------------------------------------------------
	override bool IsValidFaction(string factionKey)
	{
		if (!super.IsValidFaction(factionKey))
			return false;
		
		//~ Doesn't matter if it is military or civilian
		if (m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.MILITARY_AND_CIVILIAN)
			return true;
		
		SCR_Faction faction = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey(factionKey));
		if (!faction)
			return m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.MILITARY_ONLY;
		
		//~ No faction found for some reason so default to if military
		if (faction.IsMilitary())
			return m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.MILITARY_ONLY;
		else 
			return m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.CIVILIAN_ONLY;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValidFaction(int factionIndex)
	{
		if (!super.IsValidFaction(factionIndex))
			return false;
		
		//~ Doesn't matter if it is military or civilian
		if (m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.MILITARY_AND_CIVILIAN)
			return true;
		
		//~ No faction found for some reason so default to if military
		SCR_Faction faction = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));
		
		if (!faction || faction.IsMilitary())
			return m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.MILITARY_ONLY;
		else 
			return m_eIdentityGroupType == SCR_ECharacterIdentityBioGroupType.CIVILIAN_ONLY;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValidForRandomization(IEntity entity, SCR_ExtendedIdentityComponent extendedIdentity)
	{			
		//~ List is already valid so no need to check genders
		if (super.IsValidForRandomization(entity, extendedIdentity))
			return true;

		//~ Check if Weight is 0 or less else check if there are availible character identities
		if (m_iWeight <= 0)
			return false;
		
		//~ Check the control type (Player/AI)
		if (m_eCharacterControlTypes != SCR_EIdentityCharacterControlType.PLAYER_AND_AI)
		{
			SCR_ExtendedCharacterIdentityComponent extendedCharacterIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(extendedIdentity);
			if (extendedCharacterIdentity)
			{
				if (extendedCharacterIdentity.GetPlayerID() > 0)
				{
					if (m_eCharacterControlTypes == SCR_EIdentityCharacterControlType.AI_ONLY)
						return false;
				}
				else
				{
					if (m_eCharacterControlTypes == SCR_EIdentityCharacterControlType.PLAYER_ONLY)
						return false;
				}
			}
		}
		
		SCR_EIdentityGender entityGender = GetEntityGender(entity);
		//~ Gender neutral list where already checked thus there are no identities left for this character
		if (entityGender == SCR_EIdentityGender.NEUTRAL)
			return false;
		
		array<int> foundIndexes;
		if (m_mAvailableGenderSpecificBioIndexLists.Find(entityGender, foundIndexes))
			return !foundIndexes.IsEmpty();
		
		//~ No valid list for given gender
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_IdentityBio GetIdentityBio(IEntity entity, int index)
	{		
		SCR_IdentityBio bio = super.GetIdentityBio(entity, index);
		if (bio)
			return bio;
		
		index -= m_aIdentityList.Count();
		
		if (index < 0)
			return null;
		
		SCR_EIdentityGender gender = GetEntityGender(entity);
		if (gender == SCR_EIdentityGender.NEUTRAL || !m_mAvailableGenderSpecificBioIndexLists.Contains(gender))
			return null;
		
		foreach(SCR_GenderSpecificIdentityBios genderSpecificList: m_aGenderSpecificIdentityLists)
		{
			if (genderSpecificList.GetGender() == gender)
			{
				array<ref SCR_IdentityBio> identityBioList = {};
				if (index >= genderSpecificList.GetIdentityBioList(identityBioList))
					return null;
				
				return identityBioList[index];
			}
		}
		
		//~ Index invalid
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void AssignRandomAvailableBio(RandomGenerator randomizer, IEntity entity, out int index, out SCR_IdentityBio bio)
	{
		//~ Gender is neutral or no list for specific gender so grab from default list only
		SCR_EIdentityGender gender = GetEntityGender(entity);
		if (gender == SCR_EIdentityGender.NEUTRAL || !m_mAvailableGenderSpecificBioIndexLists.Contains(gender))
		{
			super.AssignRandomAvailableBio(randomizer, entity, index, bio);
			return;
		}
		
		int count = m_aAvailableIdentityBioIndexList.Count();
		int genderSpecificCount = m_mAvailableGenderSpecificBioIndexLists[gender].Count();
		
		int random = randomizer.RandInt(0, count + genderSpecificCount);
		
		//~ Neutral gender bio
		if (random < count)
		{
			index = m_aAvailableIdentityBioIndexList[random];
			bio = OnCharacterBioAssigned(entity, index);
			return;
		}
		//~ Gender Specific bio
		else 
		{
			index = m_mAvailableGenderSpecificBioIndexLists[gender][random - count] + m_aIdentityList.Count();
			bio = OnCharacterBioAssigned(entity, index);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_IdentityBio OnCharacterBioAssigned(IEntity entity, int index)
	{		
		//~ Check if gender neutral bio was assigned
		SCR_IdentityBio bio = super.OnCharacterBioAssigned(entity, index);
		if (bio != null)
			return bio;
		
		//~ Gender Specific Bio was assigned
		index -= m_aIdentityList.Count();
		
		if (index < 0)
			return null;
		
		//~ Gender is NEUTRAL so index is invalid
		SCR_EIdentityGender gender = GetEntityGender(entity);
		if (gender == SCR_EIdentityGender.NEUTRAL || !m_mAvailableGenderSpecificBioIndexLists.Contains(gender))
			return null;
		
		array<ref SCR_IdentityBio> identityBioList = {};
		
		foreach(SCR_GenderSpecificIdentityBios specificBioList: m_aGenderSpecificIdentityLists)
		{
			if (specificBioList.GetGender() == gender)
			{
				specificBioList.GetIdentityBioList(identityBioList);
				break;
			}
		}
		
		//~ Invalid index
		if (!identityBioList.IsIndexValid(index))
			return null;

		bio = identityBioList[index];
		if (!bio)
			return null;
		
		//~ Weight zero will never remove from available
		if (m_iWeight <= 0)
			return bio;
		
		//~ Remove from available bios	
		m_mAvailableGenderSpecificBioIndexLists[gender].RemoveItem(index);
		
		//~ Add to unavailible bios (If not unique as unique bio's can only be assigned once per game)
		if (!bio.IsUnique() && !m_mUnavailableGenderSpecificBioIndexLists[gender].Contains(index))
		{
			m_mUnavailableGenderSpecificBioIndexLists[gender].Insert(index);
		}
		
		//~ Refill available list if empty
		if (m_mAvailableGenderSpecificBioIndexLists[gender].IsEmpty())
		{
			foreach (int unavailableIndex : m_mUnavailableGenderSpecificBioIndexLists[gender])
			{
				m_mAvailableGenderSpecificBioIndexLists[gender].Insert(unavailableIndex);
			}
			
			m_mUnavailableGenderSpecificBioIndexLists[gender].Clear();
		}
		
		return bio;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ResetAvailable()
	{
		super.ResetAvailable();
		
		if (m_mAvailableGenderSpecificBioIndexLists)
			m_mAvailableGenderSpecificBioIndexLists.Clear();
		else 
			m_mAvailableGenderSpecificBioIndexLists = new map<SCR_EIdentityGender, ref array<int>>();
		
		if (m_mUnavailableGenderSpecificBioIndexLists)
			m_mUnavailableGenderSpecificBioIndexLists.Clear();
		else 
			m_mUnavailableGenderSpecificBioIndexLists = new map<SCR_EIdentityGender, ref array<int>>();
		
		int count;
		SCR_EIdentityGender gender;
		
		foreach(SCR_GenderSpecificIdentityBios genderSpecificList: m_aGenderSpecificIdentityLists)
		{
			gender = genderSpecificList.GetGender();
			
			m_mAvailableGenderSpecificBioIndexLists.Insert(gender, new array<int>);
			m_mUnavailableGenderSpecificBioIndexLists.Insert(gender, new array<int>);
			
			count = genderSpecificList.GetIdentityBioListCount();
			
			for(int i = 0; i < count; i++)
	       	{
				m_mAvailableGenderSpecificBioIndexLists[gender].Insert(i);
	       	}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Returns NEUTRAL if no gender found
	protected SCR_EIdentityGender GetEntityGender(IEntity entity)
	{
		SCR_ExtendedCharacterIdentityComponent characterExtendedIdentityComponent = SCR_ExtendedCharacterIdentityComponent.Cast(entity.FindComponent(SCR_ExtendedCharacterIdentityComponent));
		if (!characterExtendedIdentityComponent)
			return SCR_EIdentityGender.NEUTRAL;
		
		return characterExtendedIdentityComponent.GetGender();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit()
	{
		super.DelayedInit();
		
		int genderSpecificEntryCount = 0;
		
		foreach (SCR_GenderSpecificIdentityBios genderSpecific : m_aGenderSpecificIdentityLists)
		{
			if (!genderSpecific)
				continue;
			
			genderSpecificEntryCount += genderSpecific.GetIdentityBioListCount();
		}
		
		//~ Highly unlikely that this happens but will send an error if it happens
		if (m_aIdentityList.Count() + genderSpecificEntryCount > SCR_IdentityManagerComponent.MAX_IDENTITY_ENTRIES)
		{
			PrintFormat("'SCR_CharacterIdentityBioGroupConfig' - %1: Identity entries count is greater than %2 which will break replication", m_sBioGroupID, SCR_IdentityManagerComponent.MAX_IDENTITY_ENTRIES, level: LogLevel.ERROR);		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void SetAdditionalWeight()
	{
		if (m_iWeight <= 0 || m_fEntryAmountWeightMulti <= 0)
			return;
		
		int genderSpecificEntryCount = 0;
		
		foreach (SCR_GenderSpecificIdentityBios genderSpecific : m_aGenderSpecificIdentityLists)
		{
			if (!genderSpecific)
				continue;
			
			genderSpecificEntryCount += genderSpecific.GetIdentityBioListCount();
		}
		
		//~ Set Additional Weight for each entry
		if (m_iWeight > 0 && m_fEntryAmountWeightMulti > 0)
			m_iWeight = m_iWeight + ((m_aIdentityList.Count() + genderSpecificEntryCount) * m_fEntryAmountWeightMulti);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CharacterIdentityBioGroupConfig()
	{
		if(SCR_Global.IsEditMode()) 
			return;
		
		//~ Safty check
		foreach(SCR_GenderSpecificIdentityBios genderSpecificList: m_aGenderSpecificIdentityLists)
		{
			if (genderSpecificList.GetGender() == SCR_EIdentityGender.NEUTRAL)
			{
				Print(string.Format("'SCR_CharacterIdentityBioGroupConfig', CharacterIdentityBioGroup '%1' has a specific gender list for gender NEUTRAL! use the default list for gender neutral Bios!", m_sBioGroupID), LogLevel.ERROR); 
				continue;
			}
			
			foreach(SCR_GenderSpecificIdentityBios genderCheck: m_aGenderSpecificIdentityLists)
			{
				if (genderSpecificList != genderCheck && genderSpecificList.GetGender() == genderCheck.GetGender())
				{
					Print(string.Format("'SCR_CharacterIdentityBioGroupConfig', CharacterIdentityBioGroup '%1' has two (or more) gender specific lists for the same gender! This should never happen!", m_sBioGroupID), LogLevel.ERROR); 
					return;
				}
			}
		}
	}
}

[BaseContainerProps(),  SCR_BaseContainerCustomTitleEnum(SCR_EIdentityGender, "m_eSpecificGender")]
class SCR_GenderSpecificIdentityBios
{
	[Attribute("1", desc: "Genders required for the bio's to be valid. For gender neutral use the default entity list (m_aIdentityList)", uiwidget: UIWidgets.SearchComboBox, enumType: SCR_EIdentityGender)]
	protected SCR_EIdentityGender m_eSpecificGender;
	
	[Attribute(desc: "List of character bio's in group for a specific gender. If the bio group is chosen in the randomizer and the entity is the correct gender then these are added to the randomizer.")]
	protected ref array<ref SCR_IdentityBio> m_aIdentityList;
	
	//------------------------------------------------------------------------------------------------
	SCR_EIdentityGender GetGender()
	{
		return m_eSpecificGender;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIdentityBioList(notnull array<ref SCR_IdentityBio> identityBioList)
	{
		identityBioList.Clear();
		
		foreach(SCR_IdentityBio bio: m_aIdentityList)
		{
			identityBioList.Insert(bio);
		}

		return identityBioList.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIdentityBioListCount()
	{
		return m_aIdentityList.Count();
	}
}

enum SCR_EIdentityCharacterControlType
{
	PLAYER_AND_AI,
	PLAYER_ONLY,
	AI_ONLY,
}