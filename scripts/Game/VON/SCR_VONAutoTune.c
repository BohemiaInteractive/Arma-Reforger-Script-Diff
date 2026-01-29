[BaseContainerProps()]
class SCR_VONAutoTune
{
	[Attribute("1")]
	private bool m_bEnabled;

	[Attribute()]
	protected ref SCR_VONAutoTuneConfig m_VONAutoTuneConfig;

	protected const float UPDATE_DELAY = 0.5; //! seconds

	protected SCR_GroupTaskManagerComponent m_GroupTaskManagerComponent;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupComponent;
	protected SCR_PlayerController m_PlayerController;
	protected SCR_VONController m_VONController;
	protected SCR_TaskSystem m_TaskSystem;
	protected RplComponent m_RplComponent;
	protected SCR_Task m_LastAddedTask;

	protected bool m_bMustUpdate;
	protected float m_fUpdateDelay;

	//------------------------------------------------------------------------------------------------
	//! Check if the auto tune is enabled.
	//! \return True when enabled
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void SetRadios(IEntity entity, SCR_EVONAutoTuneFrequency tuneSpecificFrequency = SCR_EVONAutoTuneFrequency.NONE)
	{
		if (!entity || !m_VONAutoTuneConfig || !m_VONController)
			return;

		#ifdef VON_AUTO_TUNE_DEBUG
		Print("VONAutoTune", LogLevel.DEBUG);
		#endif

		int playerID = SCR_PlayerController.GetLocalPlayerId();

		SCR_Faction faction = SCR_Faction.Cast(SCR_Faction.GetEntityFaction(entity));
		if (!faction)
			return;

		int groupId = m_PlayerGroupComponent.GetGroupID();
		SCR_AIGroup group = m_GroupsManagerComponent.FindGroup(groupId);
		if (!group)
			return;

		// set character role
		SCR_EVONAutoTuneRole characterRadioRole = SCR_EVONAutoTuneRole.ANY;
		if (faction.IsPlayerCommander(playerID))
			characterRadioRole = SCR_EVONAutoTuneRole.COMMANDER;
		else if (group.IsPlayerLeader(playerID))
			characterRadioRole = SCR_EVONAutoTuneRole.SQUAD_LEADER;
		else
			characterRadioRole = SCR_EVONAutoTuneRole.SQUAD_MEMBER;

		BaseRadioComponent foundRadio;
		foreach (SCR_VONAutoTuneIdentityConfig identity : m_VONAutoTuneConfig.m_aAutoTuneList)
		{
			if (identity.m_sFactionKey != faction.GetFactionKey())
				continue;

			foundRadio = FindFirstRadio(m_VONController, identity.m_eRadioType, faction.GetFactionRadioEncryptionKey());
			if (!foundRadio)
				continue;

			foreach (SCR_VONAutoTuneRoleConfig role : identity.m_aRoles)
			{
				if (role.m_eRole != SCR_EVONAutoTuneRole.ANY && role.m_eRole != characterRadioRole)
					continue;

				SetChannels(faction, foundRadio, role, groupId, tuneSpecificFrequency);
			}
		}
	}

	//--------------------------------- ---------------------------------------------------------------
	protected void SetChannels(notnull SCR_Faction faction, notnull BaseRadioComponent radio, notnull SCR_VONAutoTuneRoleConfig roleConfig, int groupId, SCR_EVONAutoTuneFrequency tuneSpecificFrequency = SCR_EVONAutoTuneFrequency.NONE)
	{
		int transceiversCount = radio.TransceiversCount();

		foreach (SCR_VONAutoTuneChannelConfig channel : roleConfig.m_aChannels)
		{
			if (channel.m_iChannel >= transceiversCount)
				continue;
			
			// if set tuneSpecificFrequency to NONE, tune according to all frequency types from the config
			if (tuneSpecificFrequency != SCR_EVONAutoTuneFrequency.NONE && channel.m_eAutoTuneFrequency != tuneSpecificFrequency)
				continue;

			BaseTransceiver transceiver = radio.GetTransceiver(channel.m_iChannel);
			int frequency = GetFrequency(faction, groupId, channel.m_eAutoTuneFrequency);
			if (frequency > 0)
			{
				transceiver.SetFrequency(frequency);

				#ifdef VON_AUTO_TUNE_DEBUG
				Print("VONAutoTune Set channel:"+channel.m_iChannel+" frequency:"+frequency+" freqType:"+SCR_Enum.GetEnumName(SCR_EVONAutoTuneFrequency, channel.m_eAutoTuneFrequency), LogLevel.DEBUG);
				#endif
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int GetFrequency(notnull SCR_Faction faction, int groupId, SCR_EVONAutoTuneFrequency autoTuneFrequencyType)
	{
		if (autoTuneFrequencyType == SCR_EVONAutoTuneFrequency.SQUAD)
			return m_PlayerGroupComponent.GetActualGroupFrequency();

		if (autoTuneFrequencyType == SCR_EVONAutoTuneFrequency.PLATOON)
			return GetPlatoonFrequency(faction);

		if (autoTuneFrequencyType == SCR_EVONAutoTuneFrequency.OBJECTIVE_PLATOON)
			return GetObjectiveOrPlatoonFrequency(faction, groupId);

		if (autoTuneFrequencyType == SCR_EVONAutoTuneFrequency.COMMANDER)
			return GetCommanderFrequency(faction);

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlatoonFrequency(notnull SCR_Faction faction)
	{
		return faction.GetFactionRadioFrequency();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetObjectiveOrPlatoonFrequency(notnull SCR_Faction faction, int groupId)
	{
		if (groupId < 0)
			return 0;

		int frequency = 0;

		// finding out if the group has a task
		SCR_Task task = m_TaskSystem.GetTaskAssignedTo(SCR_TaskExecutorGroup.FromGroup(groupId));
		if (task && m_GroupTaskManagerComponent.CanAssignFrequencyToTask(task))
			frequency = m_GroupTaskManagerComponent.GetTaskFrequency(task);

		if (frequency <= 0)
			frequency = GetPlatoonFrequency(faction);

		return frequency;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetCommanderFrequency(notnull Faction faction)
	{
		if (!m_GroupsManagerComponent)
			return 0;

		array<SCR_AIGroup> playableGroups = m_GroupsManagerComponent.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return 0;

		foreach (SCR_AIGroup group : playableGroups)
		{
			if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
				return group.GetRadioFrequency();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Find first equipped radio by type and encryptionKey
	protected BaseRadioComponent FindFirstRadio(SCR_VONController vonController, ERadioType radioType, string encryptionKey)
	{
		array<ref SCR_VONEntry> entries = {};
		vonController.GetVONEntries(entries);

		SCR_VONEntryRadio radioEntry;
		BaseRadioComponent baseRadio;
		foreach (SCR_VONEntry entry : entries)
		{
			radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (!radioEntry)
				continue;

			baseRadio = radioEntry.GetTransceiver().GetRadio();
			if (baseRadio.GetEncryptionKey() != encryptionKey)
				continue;

			SCR_RadioComponent radioComponent = SCR_RadioComponent.Cast(baseRadio.GetOwner().FindComponent(SCR_RadioComponent));
			if (!radioComponent)
				continue;

			if (radioComponent.GetRadioType() == radioType)
				return baseRadio;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		SetUpdateRadios();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpdateRadios()
	{
		m_bMustUpdate = true;
		m_fUpdateDelay = UPDATE_DELAY;
	}

	//------------------------------------------------------------------------------------------------
	//! Frame update
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		if (!m_bMustUpdate || !IsEnabled())
			return;

		if (m_fUpdateDelay > 0)
		{
			m_fUpdateDelay -= timeSlice;
			return;
		}

		if (m_PlayerController)
			SetRadios(m_PlayerController.GetLocalMainEntity());

		m_bMustUpdate = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Init VON Auto tune
	//! \param[in] owner
	//! \param[in] vonController
	void Init(notnull IEntity owner, notnull SCR_VONController vonController)
	{
		if (!m_VONAutoTuneConfig || !IsEnabled())
			return;

		m_VONController = vonController;

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupsManagerComponent)
			return;

		m_GroupTaskManagerComponent = SCR_GroupTaskManagerComponent.GetInstance();
		if (!m_GroupTaskManagerComponent)
			return;

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem)
			return;

		m_PlayerController = SCR_PlayerController.Cast(owner);
		if (!m_PlayerController)
			return;

		m_PlayerGroupComponent = SCR_PlayerControllerGroupComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!m_PlayerGroupComponent)
			return;

		vonController.GetOnEntriesChangedInvoker().Insert(OnEntriesChanged);
		m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Deinit VON Auto tune
	void Deinit()
	{
		if (m_PlayerController)
			m_PlayerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);

		if (m_VONController)
			m_VONController.GetOnEntriesChangedInvoker().Remove(OnEntriesChanged);

	}
}
