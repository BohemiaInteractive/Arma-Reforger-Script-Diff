[ComponentEditorProps(category: "GameScripted/Character", description: "Holds flavour information of character")]
class SCR_ExtendedCharacterIdentityComponentClass : SCR_ExtendedIdentityComponentClass
{
};
class SCR_ExtendedCharacterIdentityComponent : SCR_ExtendedIdentityComponent
{		
	[Attribute("1", desc: "Gender of character. Never overwrite in runtime.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EIdentityGender))]
	protected SCR_EIdentityGender m_eGender;
		
	[Attribute("0.25", desc: "The chance of the identity documentation in the characters inventory being valuable meaning the player can hand them in for a bonus", params: "0 1")]
	protected float m_fChanceOfIdentityDocumentBeingValuable;
	
	//~ Set on entity spawned. Will be 0 if not a player character
	protected int m_iPlayerId;
	
	//~ If identity item was added to the character at least once
	protected bool m_bIdentityItemAdded;
	
	//======================================== GENDER ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Get gender of character.
	//! \return Gender
	SCR_EIdentityGender GetGender()
	{
		return m_eGender;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected override void RandomizeIdentity(int combinedBioIndex)
	{
		super.RandomizeIdentity(combinedBioIndex);
		
		if (m_ExtendedIdentity == null || (m_ExtendedIdentity.Type() != SCR_ExtendedCharacterIdentity && m_ExtendedIdentity.Type().IsInherited(SCR_ExtendedCharacterIdentity)))
		{
			Print("'SCR_ExtendedCharacterIdentityComponent' could not find 'SCR_ExtendedCharacterIdentity' it is either missing or does not use (or inherent from) SCR_ExtendedCharacterIdentity class! Identity is not set for character!", LogLevel.ERROR);
			return;
		}	
		
		SCR_ExtendedCharacterIdentity charExtendedIdentity = SCR_ExtendedCharacterIdentity.Cast(m_ExtendedIdentity);
		
		//~ Assign bloodtype if not yet assigned
		if (charExtendedIdentity.GetBloodType() == SCR_EBloodType.AUTO)
		{
			SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
			if (!identityManager)
				return;
			
			charExtendedIdentity.SetBloodType(identityManager.GetRandomBloodTypeIndex(m_Randomizer));
		}
	}
	
	//======================================== OVERWRITE ========================================\\
	//------------------------------------------------------------------------------------------------
	//~ When Bio (or other system) overwrites the current Identity
	override void OverwriteExtendedIdentity(SCR_ExtendedIdentity newExtendedIdentity)
	{
		super.OverwriteExtendedIdentity(newExtendedIdentity);
		
		SCR_ExtendedCharacterIdentity newCharacterExtendedIdentity = SCR_ExtendedCharacterIdentity.Cast(newExtendedIdentity);
		SCR_ExtendedCharacterIdentity characterExtendedIdentity = SCR_ExtendedCharacterIdentity.Cast(m_ExtendedIdentity);
		
		if (!newCharacterExtendedIdentity || !characterExtendedIdentity)
			return;
		
		if (newCharacterExtendedIdentity.GetBloodType() != SCR_EBloodType.AUTO)
			characterExtendedIdentity.SetBloodType(newCharacterExtendedIdentity.GetBloodType());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Overwrite names specific for characters
	//! Called when Bio overwrites names.
	//! \param[in] overwriteName Overwrite Name (Ignored if empty)
	//! \param[in] overwriteAlias Overwrite Alias (Ignored if empty)
	//! \param[in] overwriteSurname Overwrite Surname (Ignored if empty)
	protected void OverwriteCharacterName(string overwriteName, string overwriteAlias, string overwriteSurname)
	{
		IEntity owner = GetOwner();
		
		if (!owner)
			return;
		
		CharacterIdentityComponent characterIdentityComponent = CharacterIdentityComponent.Cast(owner.FindComponent(CharacterIdentityComponent));
		if (!characterIdentityComponent)
			return;
		
		Identity identity = characterIdentityComponent.GetIdentity();
		if (!identity)
			return;

		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(overwriteName))
			identity.SetName(overwriteName);
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(overwriteAlias))
			identity.SetAlias(overwriteAlias);
	
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(overwriteSurname))
			identity.SetSurname(overwriteSurname);				
	}
	
	//~ TODO: Overwrite identity does not work with Identity Class and Identity class is fully code so no access to it
	/*
	protected void OverwriteCharacterIdentity(notnull Identity overwriteIdentity)
	{		
		IEntity owner = GetOwner();
		
		if (!owner)
			return;
		
		CharacterIdentityComponent characterIdentityComponent = CharacterIdentityComponent.Cast(owner.FindComponent(CharacterIdentityComponent));
		if (!characterIdentityComponent)
			return;
		
		Identity identity = characterIdentityComponent.GetIdentity();
		if (!identity)
			return;
		
		//~ This should overwrite the identity and set any vars that are assigned and ignore the ones that aren't
		characterIdentityComponent.SetIdentity(overwriteIdentity);
	}*/
	
	//! \return True if at least once an identity item was added (Server only)
	bool WasIdentityItemAddedOnce_S()
	{
		return m_bIdentityItemAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//! When identity item is added to the character.
	//! If generate valuable intel is true it will also try to set the randomized valuable intel on the identity item if no other identity item was added
	//! \param[in] item The item added to the character's inventory
	//! \param[in] generateValuableIntel If it should try generating valuable intel for the character if the item was not added at least once
	//! \return -1 if not valuable intel, otherwise it will return faction index of which faction the valuable intel is
	int OnIdentityItemAdded_S(SCR_IdentityInventoryItemComponent item, bool generateValuableIntel)
	{
		//~ Item was already added once so no need to generate valuable intel
		if (m_bIdentityItemAdded)
			return -1;
		
		//~ Make sure that checking if the item is valuable intel is always only done once
		m_bIdentityItemAdded = true;
		
		//~ Does not generate intel so no need to continue
		if (!generateValuableIntel)
			return -1;
		
		//~ Chance of identity documentation being valuable
		if (m_fChanceOfIdentityDocumentBeingValuable <= 0) //m_iPlayerId > 0 -> Might want to turn it off for players
			return -1;
		
		//~ Randomize if identity is valuable
		if (m_fChanceOfIdentityDocumentBeingValuable <= Math.RandomFloatInclusive(0, 1))
			return -1;
		
		IEntity owner = GetOwner();
		if (!owner)
			return -1;
		
		FactionAffiliationComponent affiliationComp = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		if (!affiliationComp)
			return -1;
		
		//~ Set intel faction to character faction index
		return GetGame().GetFactionManager().GetFactionIndex(affiliationComp.GetAffiliatedFaction());
	}
	
	//======================================== DEATH DATE ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_CharacterIdentityManagerComponent when character dies (Server Only)
	void OnCharacterDeath()
	{
		SCR_ExtendedCharacterIdentity characterIdentity = SCR_ExtendedCharacterIdentity.Cast(m_ExtendedIdentity);
		if (!characterIdentity)
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(gameMode.GetWorld());
		if (!world)
			return;
		
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return;
		
		int day, month, year, hour, minute, seconds;
		
		timeManager.GetDate(year, month, day);
		timeManager.GetHoursMinutesSeconds(hour, minute, seconds);
	
		int totalDeathDateMinutes = SCR_DateTimeHelper.ConvertDateIntoMinutes(year, month, day, hour, minute);
		
		OnCharacterDeathBroadCast(totalDeathDateMinutes);
		Rpc(OnCharacterDeathBroadCast, totalDeathDateMinutes);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnCharacterDeathBroadCast(int totalDeathDateMinutes)
	{
		SCR_ExtendedCharacterIdentity characterIdentity = SCR_ExtendedCharacterIdentity.Cast(m_ExtendedIdentity);
		if (!characterIdentity)
			return;
		
		//~ No death value set
		if (totalDeathDateMinutes <= 0)
			return;
		
		//~ Death date is set, abstract each value from total minutes
		int deathYear, deathMonth, deathDay, deathHour, deathMinute;
		SCR_DateTimeHelper.ConvertMinutesIntoDate(totalDeathDateMinutes, deathYear, deathMonth, deathDay, deathHour, deathMinute);
		
		characterIdentity.SetDeathDateAndTime(deathDay, deathMonth, deathYear, deathHour, deathMinute);
	}
	
	//======================================== BIO ========================================\\		
	//------------------------------------------------------------------------------------------------
	//~ Bio assigned locally
	protected override void AssignBio(SCR_IdentityBio identityBio)
	{
		super.AssignBio(identityBio);
		
		if (!m_IdentityBio)
			return;
		
		SCR_IdentityBioCharacterExtendedOverwrite advanceCharacterOverwrite = SCR_IdentityBioCharacterExtendedOverwrite.Cast(m_IdentityBio);
		if (!advanceCharacterOverwrite)
			return;
		
		//~ TODO: Overwrite identity does not work with Identity Class and Identity class is fully code so no access to it
		/*Identity identity = advanceCharacterOverwrite.GetOverwriteIdentity();
		if (identity)
			GetGame().GetCallqueue().CallLater(OverwriteCharacterIdentity, 0, false, identity);*/
		
		string overwriteName, overwriteAlias, overwriteSurname;		
		//~ Get is overwriting names, if true overwrite after 1 frame (Else they get overwritten by default naming system)
		if (advanceCharacterOverwrite.GetOverwriteNames(overwriteName, overwriteAlias, overwriteSurname))
			GetGame().GetCallqueue().CallLater(OverwriteCharacterName, 0, false, overwriteName, overwriteAlias, overwriteSurname);
	}
	
	//======================================== BIRTH PLACE ========================================\\	
	//------------------------------------------------------------------------------------------------
	//~ Overwrite to set place of birth similar to group leader if any
	//~ Server Only
	protected override void RandomizePlaceOfOrigin()
	{		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode || !gamemode.IsMaster())
			return;
		
		if (!m_ExtendedIdentity || !m_Randomizer)
			return; 
		
		//~ No faction so no origin
		int FactionOfOrigin = m_ExtendedIdentity.GetFactionOfOriginIndex();
		if (FactionOfOrigin <= -1)
			return;	
		
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		AIControlComponent aiControlComp = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (!aiControlComp)
			return;
		
		AIAgent aIAgent = aiControlComp.GetAIAgent();
		if (!aIAgent)
			return;
		
		SCR_Faction scrFaction;
		SCR_FactionHomeTerritoryConfig placeOfBirthConfig;
		
		//~ Has AI group
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(aIAgent.GetParentGroup());
		if (aiGroup)	
		{
			scrFaction = SCR_Faction.Cast(aiGroup.GetFaction());
			if (!scrFaction)
				return;
			
			//~ Faction was changed by Bio
			if (m_ExtendedIdentity.GetFactionOfOriginKey() != scrFaction.GetFactionKey())
			{
				FactionManager factionManager = GetGame().GetFactionManager();
				if (factionManager)
					scrFaction = SCR_Faction.Cast(factionManager.GetFactionByIndex(m_ExtendedIdentity.GetFactionOfOriginIndex()));
				
				if (scrFaction)
				{
					placeOfBirthConfig = scrFaction.GetFactionHomeTerritoryConfig();
					if (placeOfBirthConfig)
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
						return;
					}
				}
			}
			
			placeOfBirthConfig = scrFaction.GetFactionHomeTerritoryConfig();
			if (placeOfBirthConfig)
			{
				//~ If leader (Or no leader) set place of birth random
				if (aiGroup.GetLeaderAgent() == null || aiGroup.GetLeaderAgent() == aIAgent)
				{
					if (placeOfBirthConfig)
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
						return;
					}
				}
				//~ If not leader set place of birth to leader or random
				else 
				{
					IEntity leaderEntity = aiGroup.GetLeaderEntity();
					
					if (!leaderEntity)
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
						return;
					}
						
					SCR_ExtendedCharacterIdentityComponent leaderExtendedIdentityComponent = SCR_ExtendedCharacterIdentityComponent.Cast(leaderEntity.FindComponent(SCR_ExtendedCharacterIdentityComponent));
					if (!leaderExtendedIdentityComponent)
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
						return;
					}
					
					SCR_ExtendedIdentity leaderExtendedIdentity = leaderExtendedIdentityComponent.GetExtendedIdentity();
					if (!leaderExtendedIdentity)
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
						return;
					}
					
						
					//~ Not the same initial faction
					if (leaderExtendedIdentity.GetFactionOfOriginIndex() < 0 || leaderExtendedIdentity.GetFactionOfOriginIndex() != m_ExtendedIdentity.GetFactionOfOriginIndex())
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex());
						return;
					}
						
					//~ Check if should have same birth index as leader or have seperate one
					float randomPercentage = m_Randomizer.RandIntInclusive(0, 100);
					
					//~ Grab same place of birth as leader
					if (randomPercentage / 100 <= placeOfBirthConfig.GetSharedPlaceOfBirthWeight())
					{
						SetPlaceOfOriginIndex(leaderExtendedIdentity.GetPlaceOfOriginIndex());
						return;
					}
					//~ Set random place of birth
					else 
					{
						SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
						return;
					}
				}
			}
		}
		
		//~ Did not successfully set place of birth so try using initial faction index (Not in AI group)
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			scrFaction = SCR_Faction.Cast(factionManager.GetFactionByIndex(m_ExtendedIdentity.GetFactionOfOriginIndex()));
	
		if (!scrFaction)
			return;
		
		placeOfBirthConfig = scrFaction.GetFactionHomeTerritoryConfig();
		if (!placeOfBirthConfig)
			return;
		
		SetPlaceOfOriginIndex(placeOfBirthConfig.GetRandomHomeTerritoryIndex(m_Randomizer));
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Server only
	protected void SetPlaceOfOriginIndex(int index)
	{
		SetCharacterPlaceOfOriginBroadcast(index);
		Rpc(SetCharacterPlaceOfOriginBroadcast, index);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetCharacterPlaceOfOriginBroadcast(int index)
	{
		m_ExtendedIdentity.SetPlaceOfOriginIndex(index);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set this character player ID. Handled in the preparing of this character by the SCR_IdentityManagerComponent
	void SetPlayerID(int playerId)
	{
		m_iPlayerId = playerId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Player ID of the character (Can be 0 if AI)
	int GetPlayerID()
	{
		return m_iPlayerId;
	}
	
	//======================================== LIFE STATE CHANGED ========================================\\
	//------------------------------------------------------------------------------------------------
	protected void OnCharacterLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState != ECharacterLifeState.INCAPACITATED)
			return;
		
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return;
		
		identityManager.OnCharacterBecomeUnconscious(GetOwner());
	}
	
	//======================================== INIT/DESTROY ========================================\\
	//------------------------------------------------------------------------------------------------
	override void DelayedInit(IEntity owner)
	{
		super.DelayedInit(owner);
		
		//~ No need to inform the identity manager on state changed if not server
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controllerComponent)
			return;
		
		controllerComponent.m_OnLifeStateChanged.Insert(OnCharacterLifeStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (!m_bDelayedInitCalled)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controllerComponent)
			return;
		
		controllerComponent.m_OnLifeStateChanged.Remove(OnCharacterLifeStateChanged);
	}
	
	//======================================== RPL ========================================\\	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{	
		super.RplSave(writer);
		
		SCR_ExtendedCharacterIdentity charExtendedIdentity = SCR_ExtendedCharacterIdentity.Cast(m_ExtendedIdentity);
		if (!charExtendedIdentity)
			return false;
		
		int deathYear, deathMonth, deathDay, deathHour, deathMinute;
		charExtendedIdentity.GetDeathDateAndTime(deathDay, deathMonth, deathYear, deathHour, deathMinute);
	
		int totalDeathDateMinutes;
		
		//~ Check if a death value is set
		if (deathYear <= 0)
			totalDeathDateMinutes = 0;
		//~ Death date is set. Combine it into total minutes
		else 
			totalDeathDateMinutes = SCR_DateTimeHelper.ConvertDateIntoMinutes(deathYear, deathMonth, deathDay, deathHour, deathMinute);
		
		writer.WriteInt(totalDeathDateMinutes); 
		writer.WriteInt(charExtendedIdentity.GetPlaceOfOriginIndex());
		
        return true;
    }
     
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		super.RplLoad(reader);

		if (m_ExtendedIdentity == null || (m_ExtendedIdentity.Type() != SCR_ExtendedCharacterIdentity && m_ExtendedIdentity.Type().IsInherited(SCR_ExtendedCharacterIdentity)))
			return false;
		
		int totalDeathDateMinutes, placeOfOrginIndex;
		reader.ReadInt(totalDeathDateMinutes);
		reader.ReadInt(placeOfOrginIndex);

		OnCharacterDeathBroadCast(totalDeathDateMinutes);
		SetCharacterPlaceOfOriginBroadcast(placeOfOrginIndex);
		
        return true;
    }
}


