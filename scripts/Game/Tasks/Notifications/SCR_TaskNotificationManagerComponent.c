[ComponentEditorProps(category: "GameScripted/Tasks/Notifications", description: "Component that takes care of playing task notifications")]
class SCR_TaskNotificationManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_TaskNotificationManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc:"Task notifications root config")]
	protected ref SCR_TaskNotificationsRootConfig m_TaskNotificationsConfig;

	protected SimpleSoundComponent m_SimpleSoundComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_GameModeCampaign m_Campaign;
	protected SCR_FactionManager m_FactionManager;
	protected SCR_CallsignManagerComponent m_CallsignManager;
	protected ref map<SCR_ETaskNotification, WorldTimestamp> m_mNotificationIntervalMap = new map<SCR_ETaskNotification, WorldTimestamp>();

	protected static SCR_TaskNotificationManagerComponent s_Instance;

	protected const int POPUP_DURATION = 6;
	protected const bool SHOW_ENEMY_MAJOR_TEXT_NOTIFICATION = false;
	protected const bool SHOW_ENEMY_MINOR_TEXT_NOTIFICATION = false;
	protected const string GRID_FORMAT = "%1 %2";

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskNotification
	//! \param[in] taskNotificationMsg
	//! \return task notification data
	SCR_BaseTaskNotificationData GetTaskNotificationData(SCR_ETaskNotification taskNotification, SCR_ETaskNotificationMsg taskNotificationMsg)
	{
		SCR_TaskNotificationConfig config = GetTaskNotificationConfig(taskNotification);
		if (!config)
			return null;

		foreach (SCR_BaseTaskNotificationData data : config.m_aTaskNotificationDataList)
		{
			if (data && data.m_eTaskNotificationMsg == taskNotificationMsg)
				return data;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskNotification
	//! \return task notification config
	SCR_TaskNotificationConfig GetTaskNotificationConfig(SCR_ETaskNotification taskNotification)
	{
		if (!m_TaskNotificationsConfig || !m_TaskNotificationsConfig.m_aTaskNotificationList)
			return null;

		foreach (SCR_TaskNotificationConfig config : m_TaskNotificationsConfig.m_aTaskNotificationList)
		{
			if (config && config.m_eTaskNotification == taskNotification)
				return config;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskName
	//! \param[in] faction
	//! \param[in] base
	//! \return task name with formatted base name
	string GetTaskNameWithBase(string taskName, notnull Faction faction, notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return WidgetManager.Translate(taskName, base.GetFormattedBaseNameWithCallsign(faction));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskKey
	//! \param[in] grid
	//! \return task name with formatted grid coordinates
	string GetTaskNameWithGrid(LocalizedString taskKey, int grid)
	{
		int gridX = Math.Floor(grid * 0.001);
		int gridY = grid - (gridX * 1000);

		return WidgetManager.Translate(taskKey, string.Format(GRID_FORMAT, gridX, gridY));
	}

	//------------------------------------------------------------------------------------------------
	//! Play task notification message
	//! \param[in] taskNotification
	//! \param[in] msg
	//! \param[in] factionId
	//! \param[in] baseCallsign
	//! \param[in] callerGroupId
	//! \param[in] calledGroupId
	//! \param[in] grid
	//! \param[in] quality
	void PlayTaskNotificationMsg(SCR_ETaskNotification taskNotification, SCR_ETaskNotificationMsg msg, int factionId, int baseCallsign, int callerGroupId, int calledGroupId, int grid, float quality)
	{
		if (!m_Campaign || m_Campaign.IsTutorial() || !m_GroupsManager || !m_FactionManager)
			return;

		SCR_TaskNotificationConfig config = GetTaskNotificationConfig(taskNotification);
		if (!config)
			return;

		string taskName = config.m_sTaskName;

		SCR_BaseTaskNotificationData data = GetTaskNotificationData(taskNotification, msg);
		if (!data)
			return;

		// can skip notifications if using an interval
		if (config.m_bAllowedNotificationInterval)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			WorldTimestamp lastSavedTimestamp;
			if (m_mNotificationIntervalMap.Find(taskNotification, lastSavedTimestamp))
			{
				if (world.GetServerTimestamp().LessEqual(lastSavedTimestamp))
					return;
			}

			m_mNotificationIntervalMap.Set(taskNotification, world.GetServerTimestamp().PlusSeconds(config.m_fNotificationInterval));
		}

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(m_FactionManager.GetFactionByIndex(factionId));
		SCR_CampaignMilitaryBaseComponent base = m_Campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);
		SCR_AIGroup callerGroup = m_GroupsManager.FindGroup(callerGroupId);

		bool isFriendly = faction == m_FactionManager.GetLocalPlayerFaction();

		#ifdef TASK_NOTIFICATION_DEBUG
		Print("[SCR_CampaignFeedbackComponent.PlayTaskNotificationMsg] message:"+SCR_Enum.GetEnumName(SCR_ETaskNotificationMsg, msg)+" received", LogLevel.DEBUG);
		#endif

		SCR_MajorBaseTaskNotificationData majorBaseData = SCR_MajorBaseTaskNotificationData.Cast(data);
		if (majorBaseData && base)
		{
			switch (msg)
			{
				case SCR_ETaskNotificationMsg.MAJOR_CREATION:
					PopupMsg(majorBaseData.GetCreationText(faction, base), prio: SCR_EGroupTaskPopUpPriority.CREATED, sound: SCR_SoundEvent.TASK_CREATED, isFriendly: isFriendly);
					PlayVONotification(majorBaseData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;

				case SCR_ETaskNotificationMsg.MAJOR_GROUP_ASSIGNED_TO_TASK:
				case SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_SM:
				{
					PopupMsg(majorBaseData.m_sText, param1: GetTaskNameWithBase(taskName, faction, base), prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT, isFriendly: isFriendly);
					PlayVONotification(majorBaseData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_COMMANDER:
				{
					if (callerGroup)
					{
						PopupMsg(majorBaseData.m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(callerGroup), param2: GetTaskNameWithBase(taskName, faction, base), prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT, isFriendly: isFriendly);
						PlayVONotification(majorBaseData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					}

					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_COMPLETED:
				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_FAILED:
				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_CANCELED:
				{
					PopupMsg(majorBaseData.m_sText, param1: GetTaskNameWithBase(taskName, faction, base), prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED, isFriendly: isFriendly);
					PlayVONotification(majorBaseData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_FOR_COMMANDER_COMPLETED:
				{
					if (callerGroup)
					{
						PopupMsg(majorBaseData.m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(callerGroup), param2: GetTaskNameWithBase(taskName, faction, base), prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED, isFriendly:isFriendly);
						PlayVONotification(majorBaseData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					}

					break;
				}

				default:
					Print("[SCR_MajorTaskNotificationData.PlayTaskNotificationMsg] message type not found", LogLevel.WARNING);
					break;
			}

			return;
		}

		SCR_MajorRequestedTaskNotificationData majorRequestedData = SCR_MajorRequestedTaskNotificationData.Cast(data);
		if (majorRequestedData)
		{
			switch (msg)
			{
				case SCR_ETaskNotificationMsg.MAJOR_CREATION:
				case SCR_ETaskNotificationMsg.MAJOR_CREATION_BY_SL:
				{
					PopupMsg(majorRequestedData.GetCreationText(callerGroupId), param1: taskName, prio: SCR_EGroupTaskPopUpPriority.CREATED, sound: SCR_SoundEvent.TASK_CREATED, isFriendly: isFriendly);
					PlayVONotification(majorRequestedData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_GROUP_ASSIGNED_TO_TASK:
				case SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_SM:
				{
					PopupMsg(majorRequestedData.m_sText, param1: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT, isFriendly: isFriendly);
					PlayVONotification(majorRequestedData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_COMMANDER:
				{
					if (callerGroup)
					{
						PopupMsg(majorRequestedData.m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(callerGroup), param2: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT, isFriendly: isFriendly);
						PlayVONotification(majorRequestedData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					}

					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_COMPLETED:
				{
					PopupMsg(majorRequestedData.m_sText, param1: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED, isFriendly: isFriendly);
					PlayVONotification(majorRequestedData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_FAILED:
				{
					PopupMsg(majorRequestedData.m_sText, param1: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.FAILED, sound: SCR_SoundEvent.TASK_FAILED, isFriendly: isFriendly);
					PlayVONotification(majorRequestedData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_CANCELED:
				{
					PopupMsg(majorRequestedData.m_sText, param1: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.CANCELED, sound: SCR_SoundEvent.TASK_CANCELED, isFriendly: isFriendly);
					PlayVONotification(majorRequestedData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				default:
					Print("[SCR_MajorTaskNotificationData.PlayTaskNotificationMsg] message type not found", LogLevel.WARNING);
					break;
			}

			return;
		}

		SCR_MajorTaskNotificationData majorData = SCR_MajorTaskNotificationData.Cast(data);
		if (majorData)
		{
			switch (msg)
			{
				case SCR_ETaskNotificationMsg.MAJOR_CREATION:
					PopupMsg(majorData.GetCreationText(), prio: SCR_EGroupTaskPopUpPriority.CREATED, sound: SCR_SoundEvent.TASK_CREATED, isFriendly: isFriendly);
					PlayVONotification(majorData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;

				case SCR_ETaskNotificationMsg.MAJOR_GROUP_ASSIGNED_TO_TASK:
				case SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_SM:
				{
					PopupMsg(majorData.m_sText, param1: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT, isFriendly: isFriendly);
					PlayVONotification(majorData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_COMMANDER:
				{
					if (callerGroup)
					{
						PopupMsg(majorData.m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(callerGroup), param2:GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT, isFriendly: isFriendly);
						PlayVONotification(majorData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					}

					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_COMPLETED:
				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_FAILED:
				case SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_CANCELED:
				{
					PopupMsg(majorData.m_sText, param1: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED, isFriendly: isFriendly);
					PlayVONotification(majorData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					break;
				}

				case SCR_ETaskNotificationMsg.MAJOR_TASK_FOR_COMMANDER_COMPLETED:
				{
					if (callerGroup)
					{
						PopupMsg(majorData.m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(callerGroup), param2: GetTaskNameWithGrid(taskName, grid), prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED, isFriendly: isFriendly);
						PlayVONotification(majorData.m_sSoundEventName, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
					}

					break;
				}

				default:
					Print("[SCR_MajorTaskNotificationData.PlayTaskNotificationMsg] message type not found", LogLevel.WARNING);
					break;
			}

			return;
		}

		SCR_MinorTaskNotificationData minorData = SCR_MinorTaskNotificationData.Cast(data);
		if (minorData)
		{
			if (!isFriendly && !SHOW_ENEMY_MINOR_TEXT_NOTIFICATION)
				return;

			switch (msg)
			{
				case SCR_ETaskNotificationMsg.MINOR_CREATION:
					SCR_NotificationsComponent.SendLocal(minorData.m_eNotification, taskNotification, factionId, baseCallsign, callerGroupId);
					break;

				case SCR_ETaskNotificationMsg.MINOR_GROUP_ASSIGNED_TO_SAME_TASK:
					SCR_NotificationsComponent.SendLocal(minorData.m_eNotification, callerGroupId);
					break;

				case SCR_ETaskNotificationMsg.MINOR_TASK_COMPLETED:
				case SCR_ETaskNotificationMsg.MINOR_TASK_COMPLETED_TO_COMMANDER:
				case SCR_ETaskNotificationMsg.MINOR_TASK_FAILED:
				case SCR_ETaskNotificationMsg.MINOR_TASK_FAILED_TO_COMMANDER:
				case SCR_ETaskNotificationMsg.MINOR_TASK_CANCELED:
				case SCR_ETaskNotificationMsg.MINOR_TASK_CANCELED_TO_COMMANDER:
				case SCR_ETaskNotificationMsg.MINOR_YOUR_REQUEST_COMPLETED:
				case SCR_ETaskNotificationMsg.MINOR_YOUR_REQUEST_FAILED:
				case SCR_ETaskNotificationMsg.MINOR_YOUR_REQUEST_CANCELED:
					SCR_NotificationsComponent.SendLocal(minorData.m_eNotification, taskNotification, msg, factionId, baseCallsign, grid); // SCR_NotificationTaskName
					break;

				default:
					Print("[SCR_MajorTaskNotificationData.PlayTaskNotificationMsg] message type not found", LogLevel.WARNING);
					break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PopupMsg(string text, int prio = -1, string param1 = "", string param2 = "", string sound = "", bool isFriendly = true)
	{
		if (!isFriendly && !SHOW_ENEMY_MAJOR_TEXT_NOTIFICATION)
			return;

		SCR_PopUpNotification.GetInstance().PopupMsg(text, param1: param1, param2: param2, duration: POPUP_DURATION, prio: prio, sound: sound);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayVONotification(string soundEventName, int factionId, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int callerGroupId = -1, int calledGroupId = -1, int grid = -1, float quality = 0.5)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		IEntity player = playerController.GetMainEntity();
		if (!player)
			return;

		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_AudioSettings");
		bool announcerEnabled = true; // enabled so everything is in the default state if we don't find settings
		if (settings)
			settings.Get("m_bHQAnnouncer", announcerEnabled);

		if (!announcerEnabled)
			return;

		int gridX = Math.Floor(grid * 0.001);
		int gridY = grid - (gridX * 1000);

		PlayVOTaskNotification(soundEventName.Trim(), factionId, baseCallsign, callerGroupId, calledGroupId, gridX, gridY, quality);
	}

	//------------------------------------------------------------------------------------------------
	//! Play VO for task notification
	//! \param[in] soundEventName
	//! \param[in] factionId
	//! \param[in] baseCallsign
	//! \param[in] callerGroupId
	//! \param[in] calledGroupId
	//! \param[in] gridX
	//! \param[in] gridY
	//! \param[in] quality
	void PlayVOTaskNotification(string soundEventName, int factionId, int baseCallsign, int callerGroupId, int calledGroupId, int gridX, int gridY, float quality)
	{
		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;

		if (!m_GroupsManager)
			m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();

		SCR_HQRadioSoundEntity radioSoundEntity = SCR_HQRadioSoundEntity.GetInstance();
		if (!radioSoundEntity)
			return;

		if (!m_SimpleSoundComponent)
			m_SimpleSoundComponent = radioSoundEntity.GetSimpleSoundComponent();

		if (!m_FactionManager)
			m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!m_Campaign)
			m_Campaign = SCR_GameModeCampaign.GetInstance();

		if (!m_CallsignManager)
			m_CallsignManager = SCR_CallsignManagerComponent.Cast(m_Campaign.FindComponent(SCR_CallsignManagerComponent));

		if (!m_GroupsManager || !m_FactionManager || !m_CallsignManager || !m_SimpleSoundComponent || !m_Campaign)
			return;

		SCR_AIGroup callerGroup = m_GroupsManager.FindGroup(callerGroupId);

		int companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller;

		if (callerGroup)
			m_CallsignManager.GetEntityCallsignIndexes(callerGroup, companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller);

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0)
			return;

		SCR_AIGroup calledGroup = m_GroupsManager.FindGroup(calledGroupId);

		int companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled;

		if (calledGroup)
			m_CallsignManager.GetEntityCallsignIndexes(calledGroup, companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled);

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(m_FactionManager.GetFactionByIndex(factionId));
		SCR_CampaignMilitaryBaseComponent base = m_Campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);

		if (base)
		{
			SCR_MilitaryBaseCallsign callsignInfo;

			if (faction == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign());
			else
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign(), m_Campaign.GetCallsignOffset());

			if (callsignInfo)
			{
				int signalBase = m_SimpleSoundComponent.GetSignalIndex("Base");
				m_SimpleSoundComponent.SetSignalValue(signalBase, callsignInfo.GetSignalIndex());
			}
		}

		if (companyCallsignIndexCaller != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			int signalCompanyCaller = m_SimpleSoundComponent.GetSignalIndex("CompanyCaller");
			int signalPlatoonCaller = m_SimpleSoundComponent.GetSignalIndex("PlatoonCaller");
			int signalSquadCaller = m_SimpleSoundComponent.GetSignalIndex("SquadCaller");
			m_SimpleSoundComponent.SetSignalValue(signalCompanyCaller, companyCallsignIndexCaller);
			m_SimpleSoundComponent.SetSignalValue(signalPlatoonCaller, platoonCallsignIndexCaller);
			m_SimpleSoundComponent.SetSignalValue(signalSquadCaller, squadCallsignIndexCaller);
		}

		if (companyCallsignIndexCalled != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			int signalCompanyCalled = m_SimpleSoundComponent.GetSignalIndex("CompanyCalled");
			int signalPlatoonCalled = m_SimpleSoundComponent.GetSignalIndex("PlatoonCalled");
			int signalSquadCalled = m_SimpleSoundComponent.GetSignalIndex("SquadCalled");
			m_SimpleSoundComponent.SetSignalValue(signalCompanyCalled, companyCallsignIndexCalled);
			m_SimpleSoundComponent.SetSignalValue(signalPlatoonCalled, platoonCallsignIndexCalled);
			m_SimpleSoundComponent.SetSignalValue(signalSquadCalled, squadCallsignIndexCalled);
		}

		if (gridX >= 0 && gridY >= 0)
		{
			float gridValue = (gridX * 1000) + gridY;
			int signalGrid = m_SimpleSoundComponent.GetSignalIndex("Grid");
			m_SimpleSoundComponent.SetSignalValue(signalGrid, gridValue);
		}

		int signalIdentityVoice = m_SimpleSoundComponent.GetSignalIndex("IdentityVoice");
		m_SimpleSoundComponent.SetSignalValue(signalIdentityVoice, faction.GetIndentityVoiceSignal());

		int signalQuality = m_SimpleSoundComponent.GetSignalIndex("TransmissionQuality");
		m_SimpleSoundComponent.SetSignalValue(signalQuality, quality);

		radioSoundEntity.PlayRadioSound(soundEventName);
	}

	//------------------------------------------------------------------------------------------------
	//! \return instance of SCR_TaskNotificationManagerComponent
	static SCR_TaskNotificationManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Campaign = SCR_GameModeCampaign.GetInstance();
		if (!m_Campaign)
			return;

		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		m_CallsignManager = SCR_CallsignManagerComponent.Cast(m_Campaign.FindComponent(SCR_CallsignManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		// remove if unused
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_TaskNotificationManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
		else
			Print("[SCR_TaskNotificationManagerComponent instance is missing", LogLevel.WARNING);
	}
}
