class SCR_InstigatorContextData
{
	protected int m_iVictimPlayerID;
	protected int m_iKillerPlayerID;
	
	protected IEntity m_VictimEntity;
	protected IEntity m_KillerEntity;
	
	protected ref Instigator m_Instigator;
	
	protected SCR_ECharacterDeathStatusRelations m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.UNKNOWN;
	
	protected SCR_ECharacterControlType m_eVictimControlType = SCR_ECharacterControlType.UNKNOWN;
	protected SCR_ECharacterControlType m_eKillerControlType = SCR_ECharacterControlType.UNKNOWN;
	
	protected SCR_ECharacterDisguiseType m_eKillerDisguiseType = SCR_ECharacterDisguiseType.DEFAULT_FACTION;
	protected SCR_ECharacterDisguiseType m_eVictimDisguiseType = SCR_ECharacterDisguiseType.DEFAULT_FACTION;
	
	//------------------------------------------------------------------------------------------------
	//! If the relation ship between the killer and victim is the given relation. Note that this is a flag and it can check multiple relation types at the same time
	//! \param[in] relation Relation (or relations as it is a flag) to check for
	//! \return Will return true if any of the given relations is true
	bool HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations relation)
	{
		return SCR_Enum.HasPartialFlag(m_eVictimKillerRelation, relation);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The relation type between victim and killer. Eg: Suicide, Killed by enemy, killed by GM etc
	SCR_ECharacterDeathStatusRelations GetVictimKillerRelation()
	{
		return m_eVictimKillerRelation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The control type of victim. Eg: AI, Player, Possessed AI, GM/Admin
	SCR_ECharacterControlType GetVictimCharacterControlType()
	{
		return m_eVictimControlType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] controlType Control type to check 
	//! \return Returns true if victim has any of the give control type flags. Eg: AI, Player, Possessed AI, GM/Admin
	bool HasAnyVictimCharacterControlType(SCR_ECharacterControlType controlType)
	{
		return SCR_Enum.HasPartialFlag(m_eVictimControlType, controlType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The control type of killer. Eg: AI, Player, Possessed AI, GM/Admin
	SCR_ECharacterControlType GetKillerCharacterControlType()
	{
		return m_eKillerControlType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] controlType Control type to check 
	//! \return Returns true if killer has any of the give control type flags. Eg: AI, Player, Possessed AI, GM/Admin
	bool HasAnyKillerCharacterControlType(SCR_ECharacterControlType controlType)
	{
		return SCR_Enum.HasPartialFlag(m_eKillerControlType, controlType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns player ID of victim
	int GetVictimPlayerID()
	{
		return m_iVictimPlayerID;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns player ID of killer
	int GetKillerPlayerID()
	{
		return m_iKillerPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns Controlled Entity of Victim
	IEntity GetVictimEntity()
	{
		return m_VictimEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns Controlled Entity of killer (Might change if entity was player and changes character)
	IEntity GetKillerEntity()
	{
		return m_KillerEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns instigator which holds additional information of the killer
	Instigator GetInstigator()
	{
		return m_Instigator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return killer disguise type. Which is the relation between the perceived faction and affiliated faction. Will return DEFAULT_FACTION if disabled
	SCR_ECharacterDisguiseType GetKillerDisguiseType()
	{
		return m_eKillerDisguiseType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return victim disguise type. Which is the relation between the perceived faction and affiliated faction. Will return DEFAULT_FACTION if disabled
	SCR_ECharacterDisguiseType GetVictimDisguiseType()
	{
		return m_eVictimDisguiseType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method to check wether or not the kill of this context action counts as team killing
	//! \param[in] editorKillsCount If true then admins, GMs and players on admin list are not ignored
	//! \param[in] possessedKillsCount If true then Kills done while possessing an AI are not ignored
	//! \return True if it counts as a team kill
	bool DoesPlayerKillCountAsTeamKill(bool editorKillsCount = false, bool possessedKillsCount = false)
	{
		//~ No friendly fire involved or not killed by player character
		if (m_eVictimKillerRelation != SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER)
			return false;
		
		//~ Friendly kills do not count for admins, GMs
		if (!editorKillsCount && m_eKillerControlType == SCR_ECharacterControlType.UNLIMITED_EDITOR)
			return false;
		
		//~ Check if friendly kills count for AI that are possed by GM
		if (!possessedKillsCount && m_eKillerControlType == SCR_ECharacterControlType.POSSESSED_AI)
			return false;
		
		//~ Victim was disguised as a unknown or hostile faction so do not concider it friendly fire
		if (m_eVictimDisguiseType == SCR_ECharacterDisguiseType.UNKNOWN_FACTION || m_eVictimDisguiseType == SCR_ECharacterDisguiseType.HOSTILE_FACTION)
			return false;
		
		//~ Friendly kills only counted if friendly fire is punished
		SCR_AdditionalGameModeSettingsComponent additionalGameModeSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (additionalGameModeSettings && !additionalGameModeSettings.IsTeamKillingPunished())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] punishmentToCheck Which flag(s) are checked to see if the enemy kill is punished
	//! \param[in] editorKillsCount If true then admins, GMs and players on admin list are not ignored
	//! \param[in] checkIfAllFlagsValid Will check all flags are equal to the setting if true. Otherwise if one of them is true the condition will return true
	//! \return True if the enemy kill is punished
	bool IsEnemyKillPunished(SCR_EDisguisedKillingPunishment punishmentToCheck, bool editorKillsCount = false, bool checkIfAllFlagsValid = false)
	{
		//~ Not disguised so no need to check
		if (m_eKillerDisguiseType == SCR_ECharacterDisguiseType.DEFAULT_FACTION)
			return false;
		
		//~ Needs to be killing a hostile
		if (m_eVictimKillerRelation != SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER)
			return false;
		
		//~ Disguised kills do not count for admins, GMs
		if (!editorKillsCount && m_eKillerControlType == SCR_ECharacterControlType.UNLIMITED_EDITOR)
			return false;
		
		//~ Check if team killing is punished
		SCR_AdditionalGameModeSettingsComponent additionalGameModeSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (additionalGameModeSettings && !additionalGameModeSettings.IsTeamKillingPunished())
			return false;
		
		SCR_PerceivedFactionManagerComponent perceivedFactionManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedFactionManager)
			return false;
		
		//~ Check if all flags are valid
		if (checkIfAllFlagsValid)
			return SCR_Enum.HasFlag(perceivedFactionManager.GetPunishmentKillingWhileDisguisedFlags(), punishmentToCheck);

		return SCR_Enum.HasPartialFlag(perceivedFactionManager.GetPunishmentKillingWhileDisguisedFlags(), punishmentToCheck);
	}
	
	//------------------------------------------------------------------------------------------------
	//! This method sets death relation and status based on victim, killer, and instigator entities, considering their control types, faction and if GM or not
	//! \param[in] victimPlayerID Represents the ID of the player who is the victim in the context of this method.
	//! \param[in] victimEntity The victimEntity represents the entity killed in the context, which can be a player or an AI character.
	//! \param[in] killerEntity Killer entity represents the killer in the context of this method, which is used to determine the relationship between the killer and victim
	//! \param[in] instigator Instigator To obtain the actual killer and other information about the killer
	//! \param[in] isDeleted If isDeleted is true, it represents that the victim entity has been deleted by the editor or other function
	void SCR_InstigatorContextData(int victimPlayerID, IEntity victimEntity, IEntity killerEntity, notnull Instigator instigator, bool isDeleted = false)
	{		
		CreateInstigatorData(victimPlayerID, victimEntity, killerEntity, instigator, isDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	//! This method sets death relation and status based on victim, killer, and instigator entities, considering their control types, faction and if GM or not
	//! \param[in] victimPlayerID Represents the ID of the player who is the victim in the context of this method.
	//! \param[in] victimEntity The victimEntity represents the entity killed in the context, which can be a player or an AI character.
	//! \param[in] killerEntity Killer entity represents the killer in the context of this method, which is used to determine the relationship between the killer and victim
	//! \param[in] instigator Instigator To obtain the actual killer and other information about the killer
	//! \param[in] isDeleted If isDeleted is true, it represents that the victim entity has been deleted by the editor or other function
	void CreateInstigatorData(int victimPlayerID, IEntity victimEntity, IEntity killerEntity, notnull Instigator instigator, bool isDeleted = false)
	{		
		m_iVictimPlayerID = victimPlayerID;
		m_VictimEntity = victimEntity;
		
		//~ Makes sure it only gets the active killer entity if the current one does no longer exist to avoid Issues with players changing teams
		if (!killerEntity)
			m_KillerEntity = instigator.GetInstigatorEntity();
		else 
			m_KillerEntity = killerEntity;
		
		m_Instigator = instigator;
		m_iKillerPlayerID = m_Instigator.GetInstigatorPlayerID();
		
		//~ Set perceived faction vars of killer and victim
		SetPerceivedFaction(m_iVictimPlayerID, m_VictimEntity, m_KillerEntity, instigator, isDeleted);
		
		//~ Character was deleted
		if (isDeleted)
		{
			if (m_iKillerPlayerID > 0)
				m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.DELETED_BY_EDITOR;
			else			
				m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.DELETED;
			
			return;
		}
		
		//~ Entity destroyed is not a character so no need to track
		if (!ChimeraCharacter.Cast(m_VictimEntity))
		{
			m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.NOT_A_CHARACTER;
			return;
		}
		
		InstigatorType instigatorType = m_Instigator.GetInstigatorType();
		
		//~ Get player ID if non is set
		if (m_iVictimPlayerID <= 0 && m_VictimEntity)
			m_iVictimPlayerID = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(m_VictimEntity);
		
		//~ Character was killed by an AI
		if (instigatorType == InstigatorType.INSTIGATOR_AI)
		{
			m_eKillerControlType = SCR_ECharacterControlType.AI;
		}
		//~ Character was killed by a Player
		else if (instigatorType == InstigatorType.INSTIGATOR_PLAYER)
		{
			if (m_KillerEntity)
				m_eKillerControlType = SCR_CharacterHelper.GetCharacterControlType(m_KillerEntity);
			else if (m_iKillerPlayerID > 0)
				m_eKillerControlType = SCR_CharacterHelper.GetPlayerControlType(m_iKillerPlayerID);
		}
		//~ Instigator type is set to GM so player is killed by a GM action
		else if (instigatorType == InstigatorType.INSTIGATOR_GM)
		{
			//~ Get killer controller type if any
			if (m_iKillerPlayerID > 0)
				m_eKillerControlType = SCR_CharacterHelper.GetPlayerControlType(m_iKillerPlayerID);
			
			//~ Get victim controller type if any
			if (m_iVictimPlayerID <= 0)
				m_eVictimControlType = SCR_ECharacterControlType.AI;
			else if (m_VictimEntity)
				m_eVictimControlType = SCR_CharacterHelper.GetCharacterControlType(m_VictimEntity);
			else 
				m_eVictimControlType = SCR_CharacterHelper.GetPlayerControlType(m_iVictimPlayerID);
			
			//~ Set relation to GM and return
			m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.KILLED_BY_UNLIMITED_EDITOR;
			return;
		}
		else
		{
			if (instigatorType == InstigatorType.INSTIGATOR_NONE)
				Print("SCR_InstigatorContextData: No instigator type is set on Character death. It is likely that the KillerVictimRelation will be set to OTHER_DEATH which will not punish the victim for suicide.", LogLevel.WARNING);
			else 
				Print("SCR_InstigatorContextData: instigatorType: '" + typename.EnumToString(InstigatorType, instigatorType) + "' is not supported!", LogLevel.ERROR);
		}
		
		//~ Get the control type of the victim
		if (m_VictimEntity)
		{
			if (m_iVictimPlayerID <= 0)
				m_eVictimControlType = SCR_ECharacterControlType.AI;
			else
				m_eVictimControlType = SCR_CharacterHelper.GetCharacterControlType(m_VictimEntity);
		}
		else if (m_iVictimPlayerID > 0)
		{
			m_eVictimControlType = SCR_CharacterHelper.GetPlayerControlType(m_iVictimPlayerID);
		}
			
		//~ Killed self (If not the same control type than the player shot himself by possessing another Entity
		if (m_eVictimControlType == m_eKillerControlType && ((m_iVictimPlayerID > 0 && m_iVictimPlayerID == m_iKillerPlayerID) || m_VictimEntity == m_KillerEntity))
		{
			m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.SUICIDE;
			return;
		}
		
		//~ There is no killer. It is not a suicide so is not punished but it is not any of the other deaths either
		if (!m_KillerEntity && m_iKillerPlayerID <= 0)
		{
			m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.OTHER_DEATH;
			return;
		}
		
		//~ Get the Faction of the victim character
		Faction victimFaction = GetFaction(m_VictimEntity, m_iVictimPlayerID);
		if (!victimFaction)
		{
			m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.VICTIM_IS_NEUTRAL_OR_FACTIONLESS;
			return;
		}
			
		//~ Get the Faction of the killer character
		Faction killerFaction = GetFaction(m_KillerEntity, m_iKillerPlayerID);
		if (!killerFaction)
		{
			m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.KILLED_BY_NEUTRAL_OR_FACTIONLESS;
			return;
		}
		
		SCR_Faction scrVictimFaction = SCR_Faction.Cast(victimFaction);
		
		//~ Killed by Enemy
		if ((scrVictimFaction && !scrVictimFaction.DoCheckIfFactionFriendly(killerFaction)) || (!scrVictimFaction && victimFaction.IsFactionEnemy(killerFaction)))
		{
			if (m_eKillerControlType == SCR_ECharacterControlType.PLAYER || m_eKillerControlType == SCR_ECharacterControlType.POSSESSED_AI || m_eKillerControlType == SCR_ECharacterControlType.UNLIMITED_EDITOR)
				m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER;
			else
				m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_AI;
			
			return;
		}
		//~ Killed by Friendly
		else 
		{
			if (m_eKillerControlType == SCR_ECharacterControlType.PLAYER || m_eKillerControlType == SCR_ECharacterControlType.POSSESSED_AI || m_eKillerControlType == SCR_ECharacterControlType.UNLIMITED_EDITOR)
				m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER;
			else
				m_eVictimKillerRelation = SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_AI;
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPerceivedFaction(int victimPlayerID, IEntity victimEntity, IEntity killerEntity, notnull Instigator instigator, bool isDeleted = false)
	{
		SCR_PerceivedFactionManagerComponent perceivedFactionManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedFactionManager || perceivedFactionManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.DISABLED)
			return;
		
		SCR_CharacterFactionAffiliationComponent charFactionAffiliation;
		if (killerEntity)
		{
			charFactionAffiliation = SCR_CharacterFactionAffiliationComponent.Cast(killerEntity.FindComponent(SCR_CharacterFactionAffiliationComponent));
			if (charFactionAffiliation)
				m_eKillerDisguiseType = charFactionAffiliation.GetCharacterDisguiseType();
		}
		
		if (victimEntity)
		{
			charFactionAffiliation = SCR_CharacterFactionAffiliationComponent.Cast(victimEntity.FindComponent(SCR_CharacterFactionAffiliationComponent));
			if (charFactionAffiliation)
				m_eVictimDisguiseType = charFactionAffiliation.GetCharacterDisguiseType();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Prioritizes getting the faction of the entity but will try get player faction if the faction changes
	protected Faction GetFaction(IEntity entity, int playerID)
	{
		if (!entity)
			return GetFactionFromPlayerID(playerID);
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return GetFactionFromPlayerID(playerID);
		
		return factionAffiliation.GetAffiliatedFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get faction off player ID
	protected Faction GetFactionFromPlayerID(int playerID)
	{
		if (playerID <= 0)
			return null;
		
		return SCR_FactionManager.SGetPlayerFaction(playerID);
	}
}
