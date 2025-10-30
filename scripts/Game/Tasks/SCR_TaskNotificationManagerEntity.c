class SCR_TaskNotificationManagerEntityClass : GenericEntityClass
{
}

class SCR_TaskNotificationManagerEntity : GenericEntity
{
	protected SoundComponent m_SoundComponent;
	protected SignalsManagerComponent m_SignalComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;

	protected static SCR_TaskNotificationManagerEntity s_Instance = null;

	//------------------------------------------------------------------------------------------------
	static SCR_TaskNotificationManagerEntity GetInstance()
	{
		if (!s_Instance)
		{
			BaseWorld world = GetGame().GetWorld();

			if (world)
			{
				Resource resource = Resource.Load("{8CE56D5578694CB1}Prefabs/MP/Campaign/SCR_TaskNotificationManagerEntity.et");
				if (!resource || !resource.IsValid())
					return null;

				s_Instance = SCR_TaskNotificationManagerEntity.Cast(GetGame().SpawnEntityPrefab(resource, world));
			}
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] soundEventName
	//! \param[in] baseCallsign
	//! \param[in] callerGroupId
	//! \param[in] calledGroupId
	//! \param[in] gridX
	//! \param[in] gridY
	void PlayVONotification(string soundEventName, int baseCallsign, int callerGroupId, int calledGroupId, int gridX, int gridY)
	{
		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;

		if (!m_GroupsManager)
			m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!m_SoundComponent)
			m_SoundComponent = SoundComponent.Cast(FindComponent(SoundComponent));

		if (!m_SignalComponent)
			m_SignalComponent = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));

		if (!m_SoundComponent || !m_SignalComponent || !m_GroupsManager)
			return;

		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!fManager)
			return;

		SCR_PlayerController m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!m_PlayerController)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		if (!callsignManager)
			return;

		SCR_AIGroup callerGroup = m_GroupsManager.FindGroup(callerGroupId);

		int companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller;

		if (callerGroup)
			callsignManager.GetEntityCallsignIndexes(callerGroup, companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller);

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0)
			return;

		SCR_AIGroup calledGroup = m_GroupsManager.FindGroup(calledGroupId);
		if (!calledGroup)
			return;

		int factionId = fManager.GetFactionIndex(SCR_FactionManager.SGetLocalPlayerFaction());
		int companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled;

		if (calledGroup)
			callsignManager.GetEntityCallsignIndexes(calledGroup, companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled);

		int signalBase = m_SignalComponent.AddOrFindSignal("Base");
		int signalCompanyCaller = m_SignalComponent.AddOrFindSignal("CompanyCaller");
		int signalCompanyCalled = m_SignalComponent.AddOrFindSignal("CompanyCalled");
		int signalPlatoonCaller = m_SignalComponent.AddOrFindSignal("PlatoonCaller");
		int signalPlatoonCalled = m_SignalComponent.AddOrFindSignal("PlatoonCalled");
		int signalSquadCaller = m_SignalComponent.AddOrFindSignal("SquadCaller");
		int signalSquadCalled = m_SignalComponent.AddOrFindSignal("SquadCalled");
		int signalGrid = m_SignalComponent.AddOrFindSignal("Grid");

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(factionId));
		SCR_CampaignMilitaryBaseComponent base = campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);

		if (base)
		{
			SCR_MilitaryBaseCallsign callsignInfo;

			if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign());
			else
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign(), campaign.GetCallsignOffset());

			if (callsignInfo)
				m_SignalComponent.SetSignalValue(signalBase, callsignInfo.GetSignalIndex());
		}

		if (companyCallsignIndexCaller != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			m_SignalComponent.SetSignalValue(signalCompanyCaller, companyCallsignIndexCaller);
			m_SignalComponent.SetSignalValue(signalPlatoonCaller, platoonCallsignIndexCaller);
			m_SignalComponent.SetSignalValue(signalSquadCaller, squadCallsignIndexCaller);
		}

		if (companyCallsignIndexCalled != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			m_SignalComponent.SetSignalValue(signalCompanyCalled, companyCallsignIndexCalled);
			m_SignalComponent.SetSignalValue(signalPlatoonCalled, platoonCallsignIndexCalled);
			m_SignalComponent.SetSignalValue(signalSquadCalled, squadCallsignIndexCalled);
		}

		float gridValue = (gridX * 1000) + gridY;
		m_SignalComponent.SetSignalValue(signalGrid, gridValue);

		int signalIdentityVoice = m_SignalComponent.AddOrFindSignal("IdentityVoice");
		m_SignalComponent.SetSignalValue(signalIdentityVoice, faction.GetIndentityVoiceSignal());

		AudioSystem.TerminateSound(m_PlayedRadio);
		m_PlayedRadio = m_SoundComponent.SoundEvent(soundEventName);
	}
}
