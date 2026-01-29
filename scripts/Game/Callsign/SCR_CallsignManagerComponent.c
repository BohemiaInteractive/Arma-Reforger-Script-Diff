//! Manages available callsigns for each faction
[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

//~ ScriptInvokers
//~ Called when callsign changed or is assigned
void SCR_GroupsManagerComponent_OnPlayerCallsignChanged(int playerId, int companyCallsignIndex, int platoonCallsignIndex, int squadCallsignIndex, int characterCallsignNumber, ERoleCallsign characterRole);
typedef func SCR_GroupsManagerComponent_OnPlayerCallsignChanged;

class SCR_CallsignManagerComponent : SCR_BaseGameModeComponent
{
	//! All available callsigns are stored here
	protected ref map<Faction, ref SCR_FactionCallsignData> m_mAvailableCallsigns = new map<Faction, ref SCR_FactionCallsignData>;
	
	//! Assigned Duplicate callsigns. If all callsigns are assigned then this stores any assigned dupplicates.
	protected ref array<ref array<int>> m_aDuplicateCallsigns = {};
	
	//! Holds all the callsigns that are assigned to players
	protected ref map<int, ref SCR_PlayerCallsignData> m_mPlayerCallsignData = new map<int, ref SCR_PlayerCallsignData>;
	
	//~ Script invoker (Server only)
	protected ref ScriptInvokerBase<SCR_GroupsManagerComponent_OnPlayerCallsignChanged> m_OnPlayerCallsignChanged = new ScriptInvokerBase<SCR_GroupsManagerComponent_OnPlayerCallsignChanged>();
	
	//~ Ref (Server only)
	SCR_GroupsManagerComponent m_GroupManager;
	
	
	//======================================== GET CALLSIGN ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Get the callsign indexes assigned to entity
	//! \param[in] entity IEntity to get callsign off
	//! \param[out] company index
	//! \param[out] platoon index
	//! \param[out] squad index
	//! \param[out] character index, either role or character index, will return -1 if not a character
	//! \return bool returns false if indexes are not assigned
	//!
	bool GetEntityCallsignIndexes(IEntity entity, out int companyCallsignIndex, out int platoonCallsignIndex, out int squadCallsignIndex, out int characterCallsignIndex)
	{
		SCR_CallsignBaseComponent callsignComponent = SCR_CallsignBaseComponent.Cast(entity.FindComponent(SCR_CallsignBaseComponent));
		
		return callsignComponent && callsignComponent.GetCallsignIndexes(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the callsign indexes assigned to entity
	//! \param[in] editableEntity SCR_EditableEntityComponent to get callsign off
	//! \param[out] company index
	//! \param[out] platoon index
	//! \param[out] squad index
	//! \param[out] character index, either role or character index, will return -1 if not a character
	//! \return bool returns false if indexes are not assigned
	//!
	bool GetEntityCallsignIndexes(SCR_EditableEntityComponent editableEntity, out int companyCallsignIndex, out int platoonCallsignIndex, out int squadCallsignIndex, out int characterCallsignIndex)
	{
		return GetEntityCallsignIndexes(editableEntity.GetOwner(), companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the callsign names assigned to entity
	//! \param[in] entity IEntity to get callsign off
	//! \param[out] company name
	//! \param[out] platoon name
	//! \param[out] squad name
	//! \param[out] character name (Optinal if callsign assigned to a character)
	//! \param[out] format
	//! \return bool returns true if names are succesfully found
	//!
	bool GetEntityCallsignNames(IEntity entity, out string companyCallsignName, out string platoonCallsignName, out string squadCallsignName, out string characterCallsignName, out string format)
	{
		SCR_CallsignBaseComponent callsignComponent = SCR_CallsignBaseComponent.Cast(entity.FindComponent(SCR_CallsignBaseComponent));
		
		if (!callsignComponent)
			return false;
		
		return callsignComponent.GetCallsignNames(companyCallsignName, platoonCallsignName, squadCallsignName, characterCallsignName, format);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the callsign names assigned to editableEntity
	//! \param[in] editableEntity SCR_EditableEntityComponent to get callsign off
	//! \param[out] company name
	//! \param[out] platoon name
	//! \param[out] squad name
	//! \param[out] character name (Optinal if callsign assigned to a character)
	//! \param[out] format
	//! \return bool returns true if names are succesfully found
	//!
	bool GetEntityCallsignNames(SCR_EditableEntityComponent editableEntity, out string companyCallsignName, out string platoonCallsignName, out string squadCallsignName, out string characterCallsignName, out string format)
	{
		return GetEntityCallsignNames(editableEntity.GetOwner(), companyCallsignName, platoonCallsignName, squadCallsignName, characterCallsignName, format);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Uses the faction to get available company, platoon and squad index and makes these unavailable so they are not picked until available again
	//! \param[in] faction faction to assign callsigns
	//! \param[out] companyIndex to assign
	//! \param[out] platoonIndex to assign
	//! \param[out] squadIndex to assign
	void AssignCallGroupCallsign(Faction faction, SCR_CallsignGroupComponent masterCallsignComponent, out int companyIndex, out int platoonIndex, out int squadIndex)
	{		
		companyIndex = -1;
		platoonIndex = -1;
		squadIndex = -1;
		
		//~ Use master callsign
		if (masterCallsignComponent)
		{
			masterCallsignComponent.GetCallsignIndexes(companyIndex, platoonIndex, squadIndex);
			return;
		}
		
		//Faction has no callsigns
		if (!m_mAvailableCallsigns.Contains(faction))
			return;
		
		SCR_Faction ScrFaction = SCR_Faction.Cast(faction);
		
		if (!ScrFaction)
			return;
		
		SCR_FactionCallsignInfo callsignInfo = ScrFaction.GetCallsignInfo();
		if (!callsignInfo)
			return;
		
		//~Todo: Add logic for assigning same company callsigns by getting closest (same faction) company
		
		//Assign the first available callsign
		if (!callsignInfo.GetIsAssignedRandomly())
			AssignFirstAvailableGroupCallsign(faction, companyIndex, platoonIndex, squadIndex);
		//Assign a random available callsign
		else 
			AssignRandomGroupCallsigns(faction, companyIndex, platoonIndex, squadIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignFirstAvailableGroupCallsign(Faction faction, out int companyIndex, out int platoonIndex, out int squadIndex)
	{
		SCR_FactionCallsignData factionCallsign; 
		if (m_mAvailableCallsigns.Find(faction, factionCallsign))
		{
			if (factionCallsign.GetFirstAvailibleCallsign(companyIndex, platoonIndex, squadIndex))
			{
				RemoveAvailableGroupCallsign(faction, companyIndex, platoonIndex, squadIndex);
				return;
			}	
		}
		
		//Callsign was not assigned as all callsigns are taken
		AssignRandomDuplicateCallsign(faction, companyIndex, platoonIndex, squadIndex);
		Print(string.Format("All available callsigns are taken for faction '%1', so a random duplicate is assigned instead. If this happenes a lot then more callsigns should be added for this faction!", faction.GetFactionName()), LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//~Todo: Not yet used. Should check neighbouring groups and assign the same company if close to same faction group
	protected void AssignCompanySpecificGroupCallsign(Faction faction, out int specificCompanyIndex, out int platoonIndex, out int squadIndex)
	{
		SCR_FactionCallsignData factionCallsign; 
		if (m_mAvailableCallsigns.Find(faction, factionCallsign))
		{
			if (factionCallsign.GetSpecificCompanyCallsign(specificCompanyIndex, platoonIndex, squadIndex))
			{
				RemoveAvailableGroupCallsign(faction, specificCompanyIndex, platoonIndex, squadIndex);
				return;
			}
		}
		
		//Callsign was not assigned as all callsigns are taken
		AssignRandomDuplicateCallsign(faction, specificCompanyIndex, platoonIndex, squadIndex);
		Print(string.Format("All available callsigns are taken for faction '%1', so a random duplicate is assigned instead. If this happenes a lot then more callsigns should be added for this faction!", faction.GetFactionName()), LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	//! \param[out] companyIndex
	//! \param[out] platoonIndex
	//! \param[out] squadIndex
	void AssignRandomGroupCallsigns(Faction faction, out int companyIndex, out int platoonIndex, out int squadIndex)
	{
		SCR_FactionCallsignData factionCallsign; 
		if (m_mAvailableCallsigns.Find(faction, factionCallsign))
		{
			if (factionCallsign.GetRandomCallsign(companyIndex, platoonIndex, squadIndex))
			{
				RemoveAvailableGroupCallsign(faction, companyIndex, platoonIndex, squadIndex);
				return;
			}	
		}
		
		//Callsign was not assigned as all callsigns are taken
		AssignRandomDuplicateCallsign(faction, companyIndex, platoonIndex, squadIndex);
		Print(string.Format("All available callsigns are taken for faction '%1', so a random duplicate is assigned instead. If this happenes a lot then more callsigns should be added for this faction!", faction.GetFactionName()), LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Makes the given callsign indexes available again for the faction.
	//! Called when the entity with the callsign is destroyed or switches factions
	//! \param[in] faction faction of callsigns
	//! \param[in] companyIndex to make available
	//! \param[in] platoonIndex to make available
	//! \param[in] squadIndex to make available
	// TODO: available -> available
	void MakeGroupCallsignAvailible(Faction faction, int companyIndex, int platoonIndex, int squadIndex)
	{		
		//Check if callsign was assigned duplicate
		if (!m_aDuplicateCallsigns.IsEmpty())
		{
			int count = m_aDuplicateCallsigns.Count();
			
			for(int i = 0; i < count; i++)
			{				
				//The assigned callsign was a duplicate safty. So it is removed from the duplicate list instead of making it available again
				if (m_aDuplicateCallsigns[i][0] == companyIndex && m_aDuplicateCallsigns[i][1] == platoonIndex && m_aDuplicateCallsigns[i][2] == squadIndex)
				{
					m_aDuplicateCallsigns.Remove(i);
					return;
				}
			}
		}
		
		SCR_FactionCallsignData factionCallsignData;
		
		if (m_mAvailableCallsigns.Find(faction, factionCallsignData))
			factionCallsignData.AddCallsign(companyIndex, platoonIndex, squadIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//A callsign is assigned
	[Friend(SCR_CallsignBaseComponentSerializer)]
	protected void RemoveAvailableGroupCallsign(Faction faction, int companyIndex, int platoonIndex, int squadIndex)
	{
		SCR_FactionCallsignData factionCallsignData;
		if (m_mAvailableCallsigns.Find(faction, factionCallsignData))
			factionCallsignData.RemoveCallsign(companyIndex, platoonIndex, squadIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! If all callsigns are taken then assign a random callsign. This is an edge case safety as otherwise entities will be without callsigns
	protected void AssignRandomDuplicateCallsign(Faction faction, out int company, out int platoon, out int squad)
	{		
		SCR_Faction ScrFaction = SCR_Faction.Cast(faction);
		if (!ScrFaction)
			return;
		
		SCR_FactionCallsignInfo factionCallsignInfo = ScrFaction.GetCallsignInfo();
		if (!factionCallsignInfo)
			return;
		
		if (factionCallsignInfo.GetRandomCallsign(company, platoon, squad))
		{
			array<int> duplicateCallsign = {};
			duplicateCallsign.Insert(company);
			duplicateCallsign.Insert(platoon);
			duplicateCallsign.Insert(squad);
			
			m_aDuplicateCallsigns.Insert(duplicateCallsign);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assign callsign for players when they are spawned (Server Only)
	protected void OnPlayerSpawn(int playerId, IEntity playerEntity)
	{		
		if (!m_GroupManager || !playerEntity)
			return;
		
		SCR_CallsignCharacterComponent characterCallsign = SCR_CallsignCharacterComponent.Cast(playerEntity.FindComponent(SCR_CallsignCharacterComponent));
		if (!characterCallsign)
			return;
		
		//~ Init player Callsign component to make sure it knows it is a player and it listens to on Callsign Changed
		characterCallsign.InitPlayerOnServer(playerId);
		
		//~ Get player Group
		SCR_AIGroup playerGroup = m_GroupManager.GetPlayerGroup(playerId);
		if (!playerGroup)
			return;
		
		//~ Get master if group is slave
		if (playerGroup.IsSlave())
		{
			SCR_AIGroup master = playerGroup.GetMaster();
			
			if (master)
				playerGroup = master;
		}
		
		int companyIndex, platoonIndex, squadIndex, characterNumber;
		ERoleCallsign characterRole;
		
		//~ Get player callsign from manager (If any)
		if (!GetPlayerCallsign(playerId, companyIndex, platoonIndex, squadIndex, characterNumber, characterRole))
			return;

		//~ Assign player callsign if found
		characterCallsign.AssignCharacterCallsign(playerGroup.GetFaction(), companyIndex, platoonIndex, squadIndex, characterNumber, characterRole, playerGroup.GetPlayerAndAgentCount(true) <= 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \param[in] companyIndex
	//! \param[in] platoonIndex
	//! \param[in] squadIndex
	//! \param[in] characterNumber
	//! \param[in] characterRole
	void SetPlayerCallsign(int playerId, int companyIndex, int platoonIndex, int squadIndex, int characterNumber, ERoleCallsign characterRole = ERoleCallsign.NONE)
	{
		if (playerId <= 0)
		{
			Print("'SetPlayerCallsign': Invalid player ID: " + playerId, LogLevel.ERROR);
			return;
		}
		
		SCR_PlayerCallsignData playerCallsignData;
		
		//~ Create new Data if non existing
		if (!m_mPlayerCallsignData.Find(playerId, playerCallsignData))
			playerCallsignData = new SCR_PlayerCallsignData();
		
		//~ Set new player callsign data
		playerCallsignData.SetPlayerCallsignIndexes(companyIndex, platoonIndex, squadIndex, characterNumber, characterRole);
		m_mPlayerCallsignData.Set(playerId, playerCallsignData);
		
		//~ Call On player callsign changed
		m_OnPlayerCallsignChanged.Invoke(playerId, companyIndex, platoonIndex, squadIndex, characterNumber, characterRole);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerLeftGame(int playerId)
	{
		if (m_mPlayerCallsignData.Contains(playerId))
			m_mPlayerCallsignData.Remove(playerId);
	}
		
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \param[out] companyIndex
	//! \param[out] platoonIndex
	//! \param[out] squadIndex
	//! \param[out] characterNumber
	//! \param[out] characterRole
	//! \return success or failure in obtaining data
	bool GetPlayerCallsign(int playerId, out int companyIndex, out int platoonIndex, out int squadIndex, out int characterNumber = -1, out ERoleCallsign characterRole = ERoleCallsign.NONE)
	{
		SCR_PlayerCallsignData playerCallsignData;
		if (m_mPlayerCallsignData.Find(playerId, playerCallsignData))
			return playerCallsignData.GetPlayerCallsignIndexes(companyIndex, platoonIndex, squadIndex, characterNumber, characterRole);
		else 
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns ScriptInvoker on player callsign changed
	ScriptInvokerBase<SCR_GroupsManagerComponent_OnPlayerCallsignChanged> GetOnPlayerCallsignChanged()
	{
		return m_OnPlayerCallsignChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillAvailableCallsigns()
	{
		array<Faction> factions = {};
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;
		
		factionManager.GetFactionsList(factions);
		
		foreach (Faction faction: factions)
		{
			SCR_Faction ScrFaction = SCR_Faction.Cast(faction);
			
			if (!ScrFaction)
				continue;
		
			SCR_FactionCallsignInfo factionCallsignInfo = ScrFaction.GetCallsignInfo();
			
			if (!factionCallsignInfo)
				continue;
			
			array<ref SCR_CallsignInfo> companyArray = {};
			array<ref SCR_CallsignInfo> platoonArray = {};
			array<ref SCR_CallsignInfo> squadArray = {};
			
			factionCallsignInfo.GetCompanyArray(companyArray);
			if (companyArray.IsEmpty())
				continue;
			
			factionCallsignInfo.GetPlatoonArray(platoonArray);
			if (platoonArray.IsEmpty())
				continue;
			
			factionCallsignInfo.GetSquadArray(squadArray);
			if (squadArray.IsEmpty())
				continue;
			
			SCR_FactionCallsignData factionCallsignData = new SCR_FactionCallsignData(factionCallsignInfo);
			
			m_mAvailableCallsigns.Insert(faction, factionCallsignData);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		FillAvailableCallsigns();
		
		if (GetGameMode().IsMaster())
		{
			m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!m_GroupManager)
				Debug.Error2("SCR_CallsignManagerComponent: EOnInit", "Could not find SCR_GroupsManagerComponent!");
			
			GetGameMode().GetOnPlayerDisconnected().Insert(OnPlayerLeftGame);
			GetGameMode().GetOnPlayerSpawned().Insert(OnPlayerSpawn);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if (SCR_Global.IsEditMode(owner))
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CallsignManagerComponent()
	{
		if (!GetGameMode())
			return;
		
		if (GetGameMode().IsMaster())
		{
			GetGameMode().GetOnPlayerDisconnected().Remove(OnPlayerLeftGame);
			GetGameMode().GetOnPlayerSpawned().Remove(OnPlayerSpawn);
		}
	}
}
