class SCR_CampaignFaction : SCR_Faction
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Defenders group prefab", "et")]
	private ResourceName m_DefendersGroupPrefab;

	[Attribute("", params: "et")]
	protected ref array<ResourceName> m_aStartingVehicles;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	private ResourceName m_MobileHQPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "For radio operators", "et")]
	private ResourceName m_RadioPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "HQ composition in small bases", "et")]
	private ResourceName m_BaseBuildingHQ;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition", "et")]
	private ResourceName m_BaseBuildingSupplyDepot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Source Base Composition", "et")]
	private ResourceName m_sBaseBuildingSourceBase;

	[Attribute(desc: "Whitelist of allowed radio messages")]
	protected ref SCR_CampaignRadioMsgWhitelistConfig m_RadioMsgWhitelistConfig;

	protected SCR_CampaignMilitaryBaseComponent m_MainBase;
	
	protected SCR_CampaignMobileAssemblyStandaloneComponent m_MobileAssembly;
	
	protected WorldTimestamp m_fVictoryTimestamp;
	protected WorldTimestamp m_fPauseByBlockTimestamp;

	protected int m_iControlPointsHeld;
	
	protected const bool USE_GROUP_FREQUENCY = true;

	//------------------------------------------------------------------------------------------------
	void GetStartingVehiclePrefabs(out notnull array<ResourceName> prefabs)
	{
		prefabs.Copy(m_aStartingVehicles);
	}
	
	//------------------------------------------------------------------------------------------------
	void SendHQMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int calledID = SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM)
	{
		if (msgType == SCR_ERadioMsg.NONE)
			return;
		
		if (m_RadioMsgWhitelistConfig && !m_RadioMsgWhitelistConfig.CanSendRadioMessage(msgType))
			return;

		SCR_CampaignMilitaryBaseComponent HQ = GetMainBase();
		
		if (!HQ)
			return;
		
		BaseRadioComponent radio = BaseRadioComponent.Cast(HQ.GetOwner().FindComponent(BaseRadioComponent));
		
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex;
		
		if (called && !callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex))
	    	return;

		if (USE_GROUP_FREQUENCY)
		{
			// send a message from HQ trasmintter individually to each group
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupsManager)
				return;

			array<SCR_AIGroup> playableGroups = groupsManager.GetPlayableGroupsByFaction(this);
			if (!playableGroups)
				return;

			SCR_CampaignRadioMsg msg;

			foreach (SCR_AIGroup group : playableGroups)
			{
				if (!group)
					continue;

				msg = new SCR_CampaignRadioMsg();
				msg.SetRadioMsg(msgType);
				msg.SetFactionId(GetGame().GetFactionManager().GetFactionIndex(this));
				msg.SetBaseCallsign(baseCallsign);
				msg.SetCalledCallsign(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex);
				msg.SetIsPublic(public);
				msg.SetParam(param);
				msg.SetPlayerID(calledID);
				msg.SetEncryptionKey(radio.GetEncryptionKey());

				if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
					msg.SetMessageForCommander(true, this);

				transmitter.BeginTransmissionFreq(msg, group.GetRadioFrequency());
			}
		}
		else
		{
			// send a message only to HQ platoon frequency
			SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg();
			msg.SetRadioMsg(msgType);
			msg.SetFactionId(GetGame().GetFactionManager().GetFactionIndex(this));
			msg.SetBaseCallsign(baseCallsign);
			msg.SetCalledCallsign(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex);
			msg.SetIsPublic(public);
			msg.SetParam(param);
			msg.SetPlayerID(calledID);
			msg.SetEncryptionKey(radio.GetEncryptionKey());
			transmitter.BeginTransmission(msg);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControlPointsHeld(int count)
	{
		m_iControlPointsHeld = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetControlPointsHeld()
	{
		return m_iControlPointsHeld;
	}

	//------------------------------------------------------------------------------------------------
	void SetMainBase(SCR_CampaignMilitaryBaseComponent mainBase)
	{
		m_MainBase = mainBase;
	}

	//------------------------------------------------------------------------------------------------
	void SetMobileAssembly(SCR_CampaignMobileAssemblyStandaloneComponent mobileAssembly)
	{
		m_MobileAssembly = mobileAssembly;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRadioPrefab()
	{
		return m_RadioPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefendersGroupPrefab()
	{
		return m_DefendersGroupPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetMobileHQPrefab()
	{
		return m_MobileHQPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingPrefab(EEditableEntityLabel type)
	{
		switch (type)
		{
			case EEditableEntityLabel.SERVICE_HQ:
				return m_BaseBuildingHQ;

			case EEditableEntityLabel.SERVICE_SUPPLY_STORAGE:
				return m_BaseBuildingSupplyDepot;

			case EEditableEntityLabel.SERVICE_SOURCE_BASE:
				return m_sBaseBuildingSourceBase;
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFactionNameUpperCase()
	{
		SCR_FactionUIInfo UI = SCR_FactionUIInfo.Cast(GetUIInfo());
		
		if (UI)
			return UI.GetFactionNameUpperCase();
		else
			return "";
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetMainBase()
	{
		return m_MainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMobileAssemblyStandaloneComponent GetMobileAssembly()
	{
		return m_MobileAssembly;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVictoryTimestamp(WorldTimestamp timestamp)
	{
		m_fVictoryTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetVictoryTimestamp()
	{
		return m_fVictoryTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPauseByBlockTimestamp(WorldTimestamp timestamp)
	{
		m_fPauseByBlockTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetPauseByBlockTimestamp()
	{
		return m_fPauseByBlockTimestamp;
	}
};
