[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignGroupComponentClass : SCR_CallsignBaseComponentClass
{
}

//! Component of assigning and storing squad names
class SCR_CallsignGroupComponent : SCR_CallsignBaseComponent
{
	protected bool m_bGroupInitCalled;
	protected SCR_AIGroup m_Group;

	//~ TODO: Investigate why AI leaders need to be group character nr 1 if they become leader
	protected const int iLEADER_ROLE_CHARACTER_CALLSIGN = 1;

	//! Keeps track of AI callsigns in group
	protected ref map<int, AIAgent> m_mAICallsigns = new map<int, AIAgent>();

	//! Keeps track of specific AI role callsigns in group.
	protected ref map<int, AIAgent> m_mAIRoleCallsigns = new map<int, AIAgent>();

	//! Keeps track of player callsigns in group <Callsign, PlayerID>. Player callsigns never change
	protected ref map<int, int> m_mPlayerCallsigns = new map<int, int>();

	//! Keeps track of specific AI role callsigns in group. map<RoleID, PlayerID>
	protected ref map<int, int> m_mPlayerRoleCallsigns = new map<int, int>();

	//~ If the group is a slave than all logic will be handled by the master rather than the slave
	protected bool m_bIsSlave;
	protected SCR_AIGroup m_MasterGroup;
	protected SCR_CallsignGroupComponent m_MasterGroupCallsignComponent;

	//------------------------------------------------------------------------------------------------
	//If server assign callsign
	override void InitOnServer(IEntity owner)
	{
		// Always verify the pointer. The object could be deleted by the time it gets here.
		if (!owner)
			return;

		super.InitOnServer(owner);

		m_Group = SCR_AIGroup.Cast(owner);
		if (!m_Group)
			return;

		//~ Set master group if group is slave
		if (m_Group.IsSlave())
		{
			m_MasterGroup = m_Group.GetMaster();

			if (m_MasterGroup)
			{
				m_bIsSlave = true;
				m_MasterGroupCallsignComponent = SCR_CallsignGroupComponent.Cast(m_MasterGroup.FindComponent(SCR_CallsignGroupComponent));
			}
		}

		//If leader assigned then init group, else wait until leader is assigned
		IEntity leader = m_Group.GetLeaderEntity();
		Faction groupFaction;
		if (leader)
			groupFaction = SCR_EditableEntityComponent.GetEditableEntity(leader).GetFaction();
		else
			groupFaction = m_Group.GetFaction();

		if (groupFaction)
			FactionInit(groupFaction);

		//~ If AI characters are added/removed from group or leader changed
		m_Group.GetOnAgentAdded().Insert(OnAIAddedToGroup);
		m_Group.GetOnAgentRemoved().Insert(OnAIRemovedFromGroup);
		m_Group.GetOnLeaderChanged().Insert(OnAILeaderAssigned);

		//~ If players are added or removed from group or leader changed
		m_Group.GetOnPlayerAdded().Insert(OnPlayerAddedToGroup);
		m_Group.GetOnPlayerRemoved().Insert(OnPlayerRemovedFromGroup);
		m_Group.GetOnPlayerLeaderChanged().Insert(OnPlayerLeaderAssigned);

		//~ On faction changed
		m_Group.GetOnFactionChanged().Insert(OnFactionChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerAddedToGroup(SCR_AIGroup aiGroup, int playerID)
	{
		if (!m_Group || m_Group != aiGroup)
			return;

		if (!m_bGroupInitCalled)
			return;

		AssignPlayerCallsign(playerID);

		//No longer one character in group so update callsigns (This includes squad leader)
		if (m_Group && m_Group.GetPlayerAndAgentCount(true) == 2)
			UpdateAllCharacterRoleCallsigns();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerRemovedFromGroup(SCR_AIGroup aiGroup, int playerID)
	{
		if (!m_Group || m_Group != aiGroup)
			return;

		foreach (int characterCallsignNumber, int mapPlayerID : m_mPlayerCallsigns)
		{
			if (playerID == mapPlayerID)
			{
				m_mPlayerCallsigns.Remove(characterCallsignNumber);
				break;
			}
		}

		foreach (int characterUniqueRole, int mapPlayerID : m_mPlayerRoleCallsigns)
		{
			if (playerID == mapPlayerID)
			{
				m_mPlayerRoleCallsigns.Remove(characterUniqueRole);
				break;
			}
		}

		UpdatePlayerRoles();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerLeaderAssigned(int groupID, int playerID)
	{
		if (!m_Group || m_Group.GetGroupID() != groupID || playerID <= 0)
			return;

		//~ If AI is Squad leader remove it
		if (m_mAIRoleCallsigns.Contains(ERoleCallsign.SQUAD_LEADER))
			MakeAIRoleAvailable(ERoleCallsign.SQUAD_LEADER);

		UpdatePlayerRoles();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerRoles()
	{
		m_mPlayerRoleCallsigns.Clear();

		foreach (int characterNumber, int player : m_mPlayerCallsigns)
		{
			AssignPlayerCallsign(player, characterNumber);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignPlayerCallsign(int playerID, int characterCallsign = -1)
	{
		if (!m_CallsignInfo)
			return;

		//Assign first availible callsign
		if (characterCallsign <= 0)
		{
			characterCallsign = 1;

			while (m_mAICallsigns.Contains(characterCallsign) || m_mPlayerCallsigns.Contains(characterCallsign))
			{
				characterCallsign++;
			}
		}

		ERoleCallsign roleIndex = -1;
		bool roleIsUnique;
		if (m_CallsignInfo.GetCharacterRoleCallsign(SCR_PossessingManagerComponent.GetPlayerMainEntity(playerID), playerID, m_Group, roleIndex, roleIsUnique) && roleIsUnique)
		{
			m_mPlayerRoleCallsigns.Set(roleIndex, playerID);
		}

		//~ Assigned callsigns will only replicate if the CallsignCharacterComponent has diffrent values
		m_CallsignManager.SetPlayerCallsign(playerID, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, characterCallsign, roleIndex);

		if (characterCallsign >= 0)
			m_mPlayerCallsigns.Set(characterCallsign, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//~ Clears and reassigns all Player and AI callsigns
	protected void UpdateAllCharacterRoleCallsigns()
	{
		UpdatePlayerRoles();

		m_mAIRoleCallsigns.Clear();
		array<AIAgent> agents = {};
		SCR_CallsignCharacterComponent characterCallsignComponent;

		if (m_Group)
			m_Group.GetAgents(agents);

		foreach (AIAgent agent : agents)
		{
			characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));

			if (characterCallsignComponent)
			{
				int characterCallsign, roleCallsign;
				characterCallsignComponent.GetCharacterAndRoleCallsign(characterCallsign, roleCallsign);
				characterCallsignComponent.ClearCharacterRoleCallsign();
				AssignAICallsign(agent, characterCallsignComponent, characterCallsign);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Check if unique role is in use by Players or AI in group
	//! None Unique roles are not checked
	//! Will always check Master if group is slave
	//! \param roleToCheck Role to check if it is in use
	//! \return true if the role is in use
	bool IsUniqueRoleInUse(int roleToCheck)
	{
		if (m_Group.IsSlave())
		{
			SCR_AIGroup master = m_Group.GetMaster();

			if (master)
			{
				SCR_CallsignGroupComponent masterCallsignComponent = SCR_CallsignGroupComponent.Cast(master.FindComponent(SCR_CallsignGroupComponent));

				if (masterCallsignComponent)
					return masterCallsignComponent.IsUniqueRoleInUse(roleToCheck);
			}
		}

		//~ Was not slave or master group could not be obtained for some reason
		return m_mAIRoleCallsigns.Contains(roleToCheck) || m_mPlayerRoleCallsigns.Contains(roleToCheck);
	}

	//------------------------------------------------------------------------------------------------
	//Called if group is created by editor to get correct faction
	protected void OnAILeaderAssigned(AIAgent currentLeader, AIAgent prevLeader)
	{
		//~ Player has become leader not AI
		if (currentLeader && SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(currentLeader.GetControlledEntity()) > 0)
			return;

		//Group init
		if (!m_bGroupInitCalled)
		{
			if (!currentLeader)
				return;

			IEntity leader = currentLeader.GetControlledEntity();
			FactionInit(SCR_EditableEntityComponent.GetEditableEntity(leader).GetFaction());
		}
		else
		{
			ReplaceAILeaderRole(currentLeader, prevLeader);
		}
	}

	//------------------------------------------------------------------------------------------------
	//If leader was replaced and both characters are still alive and in the same group
	//Note this only works if the actual leader is replaced in the SCR_AIGroup component, it will not override the callsign if it is not a leader
	protected void ReplaceAILeaderRole(AIAgent currentLeader, AIAgent prevLeader)
	{
		if (!m_bIsServer)
			return;

		//~ Make sure both agents are alive, not the same character and in the same group
		if (currentLeader == null || prevLeader == null || currentLeader == prevLeader || currentLeader.GetParentGroup() != prevLeader.GetParentGroup())
			return;

		SCR_CallsignCharacterComponent prevLeaderCallsignComponent = SCR_CallsignCharacterComponent.Cast(prevLeader.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
		if (!prevLeaderCallsignComponent)
			return;

		int prevLeaderCharacterCallsign, prevLeaderRoleCallsign
		prevLeaderCallsignComponent.GetCharacterAndRoleCallsign(prevLeaderCharacterCallsign, prevLeaderRoleCallsign);

		if (prevLeaderRoleCallsign != ERoleCallsign.SQUAD_LEADER)
			return;

		//~ Have system assign a new character callsign to the leader. Will change character callsign from 1 and assign a new role if any
		AssignAICallsign(prevLeader, prevLeaderCallsignComponent);

		//~ Make leader role availible so it is auto assigned to the new leader
		MakeAIRoleAvailable(ERoleCallsign.SQUAD_LEADER);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAIAddedToGroup(notnull AIAgent character)
	{
		//~ Player was added to group not AI
		if (SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(character.GetControlledEntity()) > 0)
			return;

		if (!m_bGroupInitCalled)
			return;

		SCR_CallsignCharacterComponent characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(character.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));

		if (characterCallsignComponent)
			AssignAICallsign(character, characterCallsignComponent);

		//No longer one character in group so update callsigns (This includes squad leader)
		if (m_Group && m_Group.GetPlayerAndAgentCount(true) == 2)
			UpdateAllCharacterRoleCallsigns();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAIRemovedFromGroup(SCR_AIGroup group, AIAgent character)
	{
		//~ Player was added to group not AI
		if (SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(character.GetControlledEntity()) > 0)
			return;

		if (!m_bGroupInitCalled)
			return;

		SCR_CallsignCharacterComponent characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(character.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));

		if (!characterCallsignComponent)
			return;

		int characterIndex, roleIndex;
		characterCallsignComponent.GetCharacterAndRoleCallsign(characterIndex, roleIndex);

		//Make character callsign availible
		if (m_mAICallsigns.Contains(characterIndex))
			m_mAICallsigns.Remove(characterIndex);

		MakeAIRoleAvailable(roleIndex);

		//Only one character in group so make sure squad leader is not assigned
		if (m_Group && m_Group.GetPlayerAndAgentCount(true) == 1)
			UpdateAllCharacterRoleCallsigns();
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignAICallsign(AIAgent character, SCR_CallsignCharacterComponent characterCallsignComponent, int characterCallsign = -1)
	{
		if (!m_CallsignInfo)
			return;

		//Assign first availible callsign
		if (characterCallsign <= 0)
		{
			characterCallsign = 1;

			while (m_mAICallsigns.Contains(characterCallsign) || m_mPlayerCallsigns.Contains(characterCallsign))
			{
				characterCallsign++;
			}
		}

		//Assign specific role callsign
		int roleIndex = -1;
		bool isUnique;
		if (m_CallsignInfo.GetCharacterRoleCallsign(character.GetControlledEntity(), -1, m_Group, roleIndex, isUnique) && isUnique)
		{
			m_mAIRoleCallsigns.Set(roleIndex, character);
		}

		//Assign callsign
		characterCallsignComponent.AssignCharacterCallsign(m_Faction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, characterCallsign, roleIndex, (!m_Group || m_Group.GetPlayerAndAgentCount(true) <= 1));

		if (characterCallsign >= 0)
			m_mAICallsigns.Set(characterCallsign, character);
	}

	//------------------------------------------------------------------------------------------------
	protected void MakeAIRoleAvailable(int roleIndex)
	{
		if (roleIndex < 0)
			return;

		//Make character role callsign availible
		m_mAIRoleCallsigns.Remove(roleIndex);

		array<AIAgent> agents = {};
		SCR_CallsignCharacterComponent characterCallsignComponent;

		int characterCallsign, currentRoleCallsign;
		bool roleReassigned = false;

		m_Group.GetAgents(agents);

		//Go over each character to check if role can be reassigned to another character
		foreach (AIAgent agent : agents)
		{
			if (SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(agent.GetControlledEntity()) > 0)
				continue;

			characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
			if (!characterCallsignComponent)
				continue;

			characterCallsignComponent.GetCharacterAndRoleCallsign(characterCallsign, currentRoleCallsign);

			//Do not assign roles to characters that already have a role (Unless squad leader)
			if (currentRoleCallsign >= 0 && roleIndex != ERoleCallsign.SQUAD_LEADER)
				continue;

			bool isUnique;
			if (m_CallsignInfo.GetCharacterRoleCallsign(agent.GetControlledEntity(), -1, m_Group, roleIndex, isUnique))
			{
				//~ Update role
				if (roleIndex != ERoleCallsign.SQUAD_LEADER)
				{
					characterCallsignComponent.UpdateCharacterRoleCallsign(roleIndex);
				}
				//~ Replace leader
				else
				{
					characterCallsignComponent.UpdateCharacterCallsignAndRole(iLEADER_ROLE_CHARACTER_CALLSIGN, roleIndex);

					m_mAICallsigns.Remove(characterCallsign);

					m_mAICallsigns.Set(iLEADER_ROLE_CHARACTER_CALLSIGN, agent);
				}

				//~ Update role map
				if (isUnique)
					m_mAIRoleCallsigns.Set(roleIndex, agent);

				roleReassigned = true;
				break;
			}
		}

		//~ Had a prev role make it availible again (Only called if new role assigned is Squad leader else it will never reach this)
		if (roleReassigned && currentRoleCallsign >= 0 && currentRoleCallsign != roleIndex)
			MakeAIRoleAvailable(currentRoleCallsign);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetCallsignNames(out string company, out string platoon, out string squad, out string character, out string format)
	{
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, emptyCharacter;
		character = string.Empty;

		if (!GetCallsignIndexes(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, emptyCharacter))
			return false;

		//No callsign data
		if (!SetCallsignInfo())
		{
			company = companyCallsignIndex.ToString();
			platoon = platoonCallsignIndex.ToString();
			squad = squadCallsignIndex.ToString();

			format = "ERROR %1-%2-%3";

			return true;
		}

		company = m_CallsignInfo.GetCompanyCallsignName(companyCallsignIndex);
		platoon = m_CallsignInfo.GetPlatoonCallsignName(platoonCallsignIndex);
		squad = m_CallsignInfo.GetSquadCallsignName(squadCallsignIndex);

		format = m_CallsignInfo.GetCallsignFormat(false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetCallsignIndexes(out int companyIndex, out int platoonIndex, out int squadIndex, out int characterNumber = -1, out ERoleCallsign characterRole = ERoleCallsign.NONE)
	{
		characterNumber = -1;

		if (m_iCompanyCallsign < 0 || m_iPlatoonCallsign < 0 || m_iSquadCallsign < 0)
		{
			companyIndex = -1;
			platoonIndex = -1;
			squadIndex = -1;
			return false;
		}

		companyIndex = m_iCompanyCallsign;
		platoonIndex = m_iPlatoonCallsign;
		squadIndex = m_iSquadCallsign;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// TODO: Callsigns should have logic as to how they are assigned as now there can be duplicants and there is no structure (eg Alpha-Red-1 should be followed by Alpha-Red-2)
	protected void AssignGroupCallsign()
	{
		if (!m_CallsignManager)
			return;

		int company, platoon, squad;
		m_CallsignManager.AssignCallGroupCallsign(m_Faction, m_MasterGroupCallsignComponent, company, platoon, squad);
		DoAssignCallsign(company, platoon, squad);
	}

	//------------------------------------------------------------------------------------------------
	void DoAssignCallsign(int company, int platoon, int squad)
	{
		AssignCallsignBroadcast(company, platoon, squad);
		Rpc(AssignCallsignBroadcast, company, platoon, squad);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCallsignBroadcast(int company, int platoon, int squad)
	{
		if (company == m_iCompanyCallsign && m_iPlatoonCallsign == platoon && m_iSquadCallsign == squad)
			return;

		m_iCompanyCallsign = company;
		m_iPlatoonCallsign = platoon;
		m_iSquadCallsign = squad;

		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, -1, -1);
	}

	//------------------------------------------------------------------------------------------------
	//! On Group init and faction change
	//! \param[in] faction
	//! \param[in] isFactionChange
	void FactionInit(Faction faction, bool isFactionChange = false)
	{
		//~ Faction is changed but is the same as before
		if (isFactionChange && faction == m_Faction)
			return;

		Faction prevFaction = m_Faction;
		m_Faction = faction;

		//Server only
		if (!m_bIsServer)
			return;

		if (!SetCallsignInfo())
			return;

		//Faction is changed so make current callsign availible again
		if (isFactionChange)
		{
			if (!prevFaction || !m_CallsignManager)
				return;

			if (m_bIsSlave)
			{
				Debug.Error2("SCR_CallsignGroupComponent", "FactionInit when faction changed is called on a slave group. This is not supported as it will assign it's own group callsign seperate from the master group!");
				return;
			}

			if (m_iCompanyCallsign >= 0)
			{
				m_CallsignManager.MakeGroupCallsignAvailible(prevFaction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign);
			}
			//Clears assigned callsign
			ClearCallsigns();

			//Clear character callsigns
			m_mAICallsigns.Clear();
			m_mAIRoleCallsigns.Clear();
		}

		if (!SetCallsignInfo())
			return;

		m_bGroupInitCalled = true;

		// Already assigned through e.g. save data
		if (m_iCompanyCallsign != -1 && m_iPlatoonCallsign != -1 && m_iSquadCallsign != -1)
			return;
		
		//Assign group callsign
		AssignGroupCallsign();

		array<AIAgent> agents = {};
		SCR_CallsignCharacterComponent characterCallsignComponent;

		m_Group.GetAgents(agents);

		//If callsigns are assigned at random randomize group as well on init
		array<int> availibleRandomCallsigns = {};
		if (m_CallsignInfo.GetIsAssignedRandomly())
		{
			int count = agents.Count();

			while (count > 0)
			{
				availibleRandomCallsigns.Insert(count);
				count--;
			}
		}

		//Assign callsigns for each character in group
		foreach (AIAgent agent : agents)
		{
			characterCallsignComponent = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
			if (!characterCallsignComponent)
				continue;

			characterCallsignComponent.ClearCallsigns();
			AssignAICallsign(agent, characterCallsignComponent);

			//~ Replace line above if you want character callsigns to also be randomized
//			if (m_CallsignInfo.GetIsAssignedRandomly())
//			{
//				int randomIndex = availibleRandomCallsigns.GetRandomIndex();
//				int randomCharacterCallsign = availibleRandomCallsigns[randomIndex];
//				availibleRandomCallsigns.Remove(randomIndex);
//
//				AssignCharacterCallsign(agent, characterCallsignComponent, randomCharacterCallsign);
//			}
//			else
//			{
//				AssignCharacterCallsign(agent, characterCallsignComponent);
//			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(Faction faction)
	{
		FactionInit(faction, m_Faction && m_Faction != faction);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iCompanyCallsign);
		writer.WriteInt(m_iPlatoonCallsign);
		writer.WriteInt(m_iSquadCallsign);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int company, platoon, squad;

		reader.ReadInt(company);
		reader.ReadInt(platoon);
		reader.ReadInt(squad);

		AssignCallsignBroadcast(company, platoon, squad);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CallsignGroupComponent()
	{
		if (!m_bIsServer || m_iCompanyCallsign < 0 || !m_Faction || !m_CallsignManager || m_bIsSlave)
			return;

		m_CallsignManager.MakeGroupCallsignAvailible(m_Faction, m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign);
	}
}
