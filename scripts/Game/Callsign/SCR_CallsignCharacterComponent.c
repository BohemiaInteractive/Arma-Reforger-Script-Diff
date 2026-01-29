[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignCharacterComponentClass : SCR_CallsignBaseComponentClass
{
}

//! Component of assigning and storing squad names
class SCR_CallsignCharacterComponent : SCR_CallsignBaseComponent
{
	//Broadcast
	protected int m_iCharacterCallsign = -1;
	protected ERoleCallsign m_iRoleCallsign = ERoleCallsign.NONE;
	protected bool m_bAloneInGroup = false;
	protected int m_iFactioniD = -1;
	protected int m_iPlayerId = 0; //~ Set on server only

	//------------------------------------------------------------------------------------------------
	override bool GetCallsignNames(out string company, out string platoon, out string squad, out string character, out string format)
	{				
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignNumber, characterRoleCallsignIndex;
		character = string.Empty;
		
		if (!GetCallsignIndexes(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignNumber, characterRoleCallsignIndex))
			return false;
		
		//Get character callsign if not alone in group or if player, or has a role and that role is not leader (AI only) (Can't really be a leader of yourself)
		bool showCharacterCallsign = !GetIsCharacterAloneInGroup() || (m_iPlayerId > 0 || SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(GetOwner()) > 0);
		
		//No callsign data
		if (!SetCallsignInfo())
		{
			company = companyCallsignIndex.ToString();
			platoon = platoonCallsignIndex.ToString();
			squad = squadCallsignIndex.ToString();
			character = characterCallsignNumber.ToString();
			format = "ERROR %1-%2-%3-%4";
			
			return true;
		}
		
		company = m_CallsignInfo.GetCompanyCallsignName(companyCallsignIndex);
		platoon = m_CallsignInfo.GetPlatoonCallsignName(platoonCallsignIndex);
		squad = m_CallsignInfo.GetSquadCallsignName(squadCallsignIndex);

		//Character callsign
		if (showCharacterCallsign)
		{	
			if (characterRoleCallsignIndex >= 0)
				character = m_CallsignInfo.GetCharacterRoleCallsignName(characterRoleCallsignIndex);
			else 
				character = characterCallsignNumber.ToString();
		}
		
		format = m_CallsignInfo.GetCallsignFormat(showCharacterCallsign, characterRoleCallsignIndex);		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetCallsignIndexes(out int companyIndex, out int platoonIndex, out int squadIndex, out int characterNumber = -1, out ERoleCallsign characterRole = ERoleCallsign.NONE)
	{		
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
		
		GetCharacterAndRoleCallsign(characterNumber, characterRole);	
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called by group, assigns a specific character group index and role (if any assigned) to character within specific group
	//! \param[in] faction Faction of group
	//! \param[in] companyIndex Company Index
	//! \param[in] platoonIndex Platoon Index
	//! \param[in] squadIndex Squad Index
	//! \param[in] characterNumber Character number in squad (starts with 1)
	//! \param[in] role Character role (If any assigned)
	//! \param[in] aloneInGroup If character is alone in the group
	void AssignCharacterCallsign(Faction faction, int companyIndex, int platoonIndex, int squadIndex, int characterNumber, ERoleCallsign role, bool aloneInGroup)
	{
		if (!m_bIsServer)
			return;
			
		//On Faction changed
		if (m_Faction != faction && faction != null)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
			{
				int factionId = factionManager.GetFactionIndex(faction);
				m_Faction = faction;
				Rpc(SetFactionBroadCast, factionId);	
			}
		}
		
		if (companyIndex == m_iCompanyCallsign && m_iPlatoonCallsign == platoonIndex && m_iSquadCallsign == squadIndex && m_iCharacterCallsign == characterNumber && m_iRoleCallsign == role && m_bAloneInGroup == aloneInGroup) 
			return;
		
		//If group did not change
		if (companyIndex == m_iCompanyCallsign && m_iPlatoonCallsign == platoonIndex && m_iSquadCallsign == squadIndex && m_bAloneInGroup == aloneInGroup)
		{
			//If role did not change
			if (m_iRoleCallsign == role)
			{
				AssignCharacterCallsignBroadcast(characterNumber);
				Rpc(AssignCharacterCallsignBroadcast, characterNumber);	
			}
			//If role did change
			else
			{
				AssignCharacterAndRoleCallsignBroadcast(characterNumber, role);
				Rpc(AssignCharacterAndRoleCallsignBroadcast, characterNumber, role);	
			}
		}
		//If group did change but not role
		else if (m_iRoleCallsign == role)
		{
			AssignCallsignNoRoleBroadcast(companyIndex, platoonIndex, squadIndex, characterNumber, aloneInGroup);
			Rpc(AssignCallsignNoRoleBroadcast, companyIndex, platoonIndex, squadIndex, characterNumber, aloneInGroup);	
		}
		//If role and group changed
		else 
		{
			AssignCallsignBroadcast(companyIndex, platoonIndex, squadIndex, characterNumber, role, aloneInGroup);
			Rpc(AssignCallsignBroadcast, companyIndex, platoonIndex, squadIndex, characterNumber, role, aloneInGroup);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the assigned role
	//! \param[in] roleCallsignIndex role index
	void UpdateCharacterRoleCallsign(ERoleCallsign roleCallsignIndex)
	{
		if (!m_bIsServer)
			return;
		
		//If role did not change
		if (m_iRoleCallsign == roleCallsignIndex) 
			return;
		
		AssignRoleCallsignBroadcast(roleCallsignIndex);
		Rpc(AssignRoleCallsignBroadcast, roleCallsignIndex);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Updates the assigned callsign and role. Called when leader role is assigned as leader is always 1
	//! \param[in] characterCallsign character callsign
	//! \param[in] roleCallsignIndex role index
	void UpdateCharacterCallsignAndRole(int characterCallsign, ERoleCallsign roleCallsignIndex)
	{
		if (!m_bIsServer)
			return;
		
		//If callsigns did not change
		if (m_iCharacterCallsign == characterCallsign && m_iRoleCallsign == roleCallsignIndex) 
			return;
		
		AssignCharacterAndRoleCallsignBroadcast(characterCallsign, roleCallsignIndex);
		Rpc(AssignCharacterAndRoleCallsignBroadcast, characterCallsign, roleCallsignIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on server only before assigning new callsign to make sure everything is clear. Does not broadcast cleared values to players!
	override void ClearCallsigns()
	{
		super.ClearCallsigns();
		m_iCharacterCallsign = -1;
		m_iRoleCallsign = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on server only before assigning new role callsign to make sure everything is clear. It is not broadcast!
	void ClearCharacterRoleCallsign()
	{
		if (!m_bIsServer)
			return;
		
		m_iRoleCallsign = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCallsignBroadcast(int company, int platoon, int squad, int character, int role, bool aloneInGroup)
	{	
		m_iCompanyCallsign = company;
		m_iPlatoonCallsign = platoon;
		m_iSquadCallsign = squad;
		m_iCharacterCallsign = character;
		m_iRoleCallsign = role;
		m_bAloneInGroup = aloneInGroup;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCallsignNoRoleBroadcast(int company, int platoon, int squad, int character, bool aloneInGroup)
	{	
		m_iCompanyCallsign = company;
		m_iPlatoonCallsign = platoon;
		m_iSquadCallsign = squad;
		m_iCharacterCallsign = character;
		m_bAloneInGroup = aloneInGroup;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCharacterAndRoleCallsignBroadcast(int character, int role)
	{	
		m_iCharacterCallsign = character;
		m_iRoleCallsign = role;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignCharacterCallsignBroadcast(int character)
	{	
		m_iCharacterCallsign = character;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void AssignRoleCallsignBroadcast(int role)
	{	
		m_iRoleCallsign = role;
		
		Event_OnCallsignChanged.Invoke(m_iCompanyCallsign, m_iPlatoonCallsign, m_iSquadCallsign, m_iCharacterCallsign, m_iRoleCallsign);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetAloneInGroupBroadcast(bool isAloneInGroup)
	{	
		m_bAloneInGroup = isAloneInGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFactionBroadCast(int factionID)
	{	
		m_iFactioniD = factionID;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(factionID);
		
		if (faction != m_Faction && faction != null)
		{
			m_Faction = faction;
			m_CallsignInfo = null;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return is character alone in group. ALso returns true if not in a group at all
	bool GetIsCharacterAloneInGroup()
	{
		return m_bAloneInGroup;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the character index (ignoring the role)
	//! \return character group index
	int GetCharacterCallsignIndex()
	{
		return m_iCharacterCallsign;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns either the character index or, if a role is assigned, it will return the character role index instead
	//! \return int character group index or role index
	int GetCharacterOrRoleCallsignIndex()
	{
		if (m_iRoleCallsign >= 0)
			return m_iRoleCallsign;
		else
			return m_iCharacterCallsign;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns both character index and character role index
	//! \param[out] character group index
	//! \param[out] role index
	void GetCharacterAndRoleCallsign(out int character, out int role)
	{
		character = m_iCharacterCallsign;
		role = m_iRoleCallsign;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the character callsign name, this either the character group index or the specific role name
	//! \param[out] characterCallsignName the character Callsign name in string
	//! \return if name was succesfully found
	bool GetCharacterCallsignName(out string characterCallsignName)
	{
		if(m_iCharacterCallsign == -1)
			return false;

		if (m_iRoleCallsign >= 0)
			characterCallsignName = m_CallsignInfo.GetCharacterRoleCallsignName(m_iRoleCallsign);
		else 
			m_iCharacterCallsign.ToString();

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Server only. On Player callsign changed
	protected void OnPlayerCallsignChanged(int playerId, int companyIndex, int platoonIndex, int squadIndex, int characterNumber, ERoleCallsign characterRole)
	{
		//~ This player's callsign was not changed
		if (playerId != m_iPlayerId)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
		{
			AssignCharacterCallsign(null, -1, -1, -1, -1, -1, true);
			return;
		}
			
		//~ No group so remove callsign
		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(playerId);
		if (!playerGroup)
		{
			AssignCharacterCallsign(null, -1, -1, -1, -1, -1, true);
			return;
		}
		
		AssignCharacterCallsign(playerGroup.GetFaction(), companyIndex, platoonIndex, squadIndex, characterNumber, characterRole, playerGroup.GetPlayerAndAgentCount(true) <= 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by Callsign Manager. Makes sure that player characters listen to On Callsign Changed (Server Only)
	//! \param[in] playerId
	void InitPlayerOnServer(int playerId)
	{
		if (playerId <= 0 || m_iPlayerId > 0)
			return;
		
		m_iPlayerId = playerId;
		
		//~ Subscribe to player callsign changed
		m_CallsignManager.GetOnPlayerCallsignChanged().Insert(OnPlayerCallsignChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iCompanyCallsign);
		writer.WriteInt(m_iPlatoonCallsign);
		writer.WriteInt(m_iSquadCallsign);
		writer.WriteInt(m_iCharacterCallsign);
		writer.WriteInt(m_iRoleCallsign);
		writer.WriteBool(m_bAloneInGroup);
		writer.WriteInt(m_iFactioniD);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int company, platoon, squad, character, role, factionID;
		bool aloneInGroup;
		
		reader.ReadInt(company);
		reader.ReadInt(platoon);
		reader.ReadInt(squad);
		reader.ReadInt(character);
		reader.ReadInt(role);
		reader.ReadBool(aloneInGroup);
		reader.ReadInt(factionID);
		
		AssignCallsignBroadcast(company, platoon, squad, character, role, aloneInGroup);
		SetFactionBroadCast(factionID);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CallsignCharacterComponent()
	{
		if (!m_bIsServer || m_iPlayerId <= 0 || !m_CallsignManager)
			return;
		
		//~ Remove Player CallsignChanged
		m_CallsignManager.GetOnPlayerCallsignChanged().Remove(OnPlayerCallsignChanged);
	}
}
