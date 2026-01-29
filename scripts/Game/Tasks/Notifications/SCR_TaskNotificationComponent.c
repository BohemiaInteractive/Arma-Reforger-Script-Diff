[ComponentEditorProps(category: "GameScripted/Tasks", description: "Task notification component")]
class SCR_TaskNotificationComponentClass : ScriptComponentClass
{
}

class SCR_TaskNotificationComponent : ScriptComponent
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enumType:SCR_ETaskNotification)]
	protected SCR_ETaskNotification m_eTaskNotification;

	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected RplComponent m_RplComponent;
	protected SCR_Task m_Task;
	protected SCR_GroupTaskManagerComponent m_GroupTaskManager;
	protected SCR_TaskNotificationManagerComponent m_TaskNotificationManager;
	protected SCR_FactionManager m_FactionManager;
	protected SCR_GameModeCampaign m_Campaign
	protected int m_iGridX;
	protected int m_iGridY;

	protected const bool ALLOW_SEND_TO_EMPTY_GROUP = false;

	//------------------------------------------------------------------------------------------------
	//! \return campaign faction
	SCR_CampaignFaction GetFaction()
	{
		if (!m_Task || !m_FactionManager)
			return null;

		SCR_CampaignFaction campaignFaction;
		foreach (string factionKey : m_Task.GetTaskData().m_aOwnerFactionKeys)
		{
			if (factionKey.IsEmpty())
				continue;

			campaignFaction = SCR_CampaignFaction.Cast(m_FactionManager.GetFactionByKey(factionKey));
			if (campaignFaction)
				return campaignFaction;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasGroupTask(SCR_TaskExecutor executor)
	{
		SCR_Task task = m_TaskSystem.GetTaskAssignedTo(executor);
		return task != null;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTaskName(SCR_Task task)
	{
		if (!task || !task.GetTaskData() || !task.GetTaskData().m_UIInfo)
		{
			Print("Task name cannot be fetched", LogLevel.WARNING);
			return string.Empty;
		}

		return task.GetTaskData().m_UIInfo.GetTranslatedName();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerGroupLeader(int playerID, notnull array<SCR_AIGroup> playableGroups)
	{
		foreach (SCR_AIGroup group : playableGroups)
		{
			if (group.IsPlayerLeader(playerID))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetGrid()
	{
		m_iGridX = 0;
		m_iGridY = 0;
		SCR_MapEntity.GetGridPos(m_Task.GetTaskPosition(), m_iGridX, m_iGridY);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskCreated(SCR_Task task, int byPlayerID)
	{
		if (task != m_Task || !m_GroupsManager)
			return;

		SCR_CampaignFaction faction = GetFaction();
		if (!faction)
			return;

		array<SCR_AIGroup> playableGroups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return;

		BaseTransceiver hqTransceiver = GetHQTransceiver(faction);
		if (!hqTransceiver)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		SCR_TaskExecutorGroup groupExecutor;
		foreach (SCR_AIGroup group : playableGroups)
		{
			if (!ALLOW_SEND_TO_EMPTY_GROUP && group.GetPlayerCount() == 0)
				continue;

			groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
			if (!groupExecutor)
				continue;

			if (m_TaskSystem.CanTaskBeAssignedTo(task, groupExecutor))
			{
				if (!task.IsTaskAssignedTo(groupExecutor))
				{
					if (HasGroupTask(groupExecutor))
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_CREATION, 0, hqTransceiver, group, baseCallsign);
					else
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_CREATION, 0, hqTransceiver, group, baseCallsign, 0, 0, m_iGridX, m_iGridY);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != m_Task || !m_GroupsManager)
			return;

		SCR_CampaignFaction faction = GetFaction();
		if (!faction)
			return;

		array<SCR_AIGroup> playableGroups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return;

		SCR_TaskExecutorGroup newAssignedGroupExecutor = SCR_TaskExecutorGroup.Cast(executor);
		if (!newAssignedGroupExecutor)
			return;

		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(requesterID);
		if (!playerGroupComponent)
			return;

		int requesterGroupId = playerGroupComponent.GetGroupID();

		SCR_AIGroup newAssignedGroup = m_GroupsManager.FindGroup(newAssignedGroupExecutor.GetGroupID());
		if (!newAssignedGroup)
			return;

		BaseTransceiver playerTransceiver = GetPlayerTransceiver(requesterID);
		if (!playerTransceiver)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		if (faction.IsPlayerCommander(requesterID))
		{
			// commander send msg to SL and SM
			SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_GROUP_ASSIGNED_TO_TASK, requesterID, playerTransceiver, newAssignedGroup, baseCallsign, requesterGroupId, newAssignedGroup.GetGroupID(), m_iGridX, m_iGridY);
		}

		SCR_TaskExecutorGroup groupExecutor;
		foreach (SCR_AIGroup group : playableGroups)
		{
			if (!ALLOW_SEND_TO_EMPTY_GROUP && group.GetPlayerCount() == 0)
				continue;

			groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
			if (!groupExecutor)
				continue;

			if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
			{
				SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_COMMANDER, requesterID, playerTransceiver, group, baseCallsign, requesterGroupId, newAssignedGroup.GetGroupID(), m_iGridX, m_iGridY);
			}
			else if (task.IsTaskAssignedTo(groupExecutor))
			{
				if (group.IsPlayerLeader(requesterID))
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_SM, requesterID, playerTransceiver, group, baseCallsign, requesterGroupId, newAssignedGroup.GetGroupID(), m_iGridX, m_iGridY);
				}
				else if (newAssignedGroupExecutor.GetGroupID() != groupExecutor.GetGroupID())
				{
					// skip a current assigned group
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_GROUP_ASSIGNED_TO_SAME_TASK, requesterID, playerTransceiver, group, baseCallsign, newAssignedGroup.GetGroupID(), groupExecutor.GetGroupID(), m_iGridX, m_iGridY);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != m_Task || !m_GroupsManager)
			return;

		SCR_CampaignFaction faction = GetFaction();
		if (!faction)
			return;

		array<SCR_AIGroup> playableGroups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return;

		BaseTransceiver hqTransceiver = GetHQTransceiver(faction);
		if (!hqTransceiver)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		SCR_TaskExecutorGroup groupExecutor;
		foreach (SCR_AIGroup group : playableGroups)
		{
			if (!ALLOW_SEND_TO_EMPTY_GROUP && group.GetPlayerCount() == 0)
				continue;

			groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
			if (!groupExecutor)
				continue;

			if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER && newState == SCR_ETaskState.COMPLETED)
			{
				SCR_AIGroup firstAssignedGroup = GetFirstAssignedGroup(m_Task);
				if (firstAssignedGroup)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_FOR_COMMANDER_COMPLETED, 0, hqTransceiver, group, baseCallsign, firstAssignedGroup.GetGroupID(), 0, m_iGridX, m_iGridY);
			}
			else if (task.IsTaskAssignedTo(groupExecutor))
			{
				if (newState == SCR_ETaskState.COMPLETED)
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_COMPLETED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
				else if (newState == SCR_ETaskState.FAILED)
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_FAILED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
				else if (newState == SCR_ETaskState.CANCELLED)
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_CANCELED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
			}
			else if (m_TaskSystem.IsTaskVisibleFor(task, groupExecutor))
			{
				if (newState == SCR_ETaskState.COMPLETED)
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_COMPLETED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
				else if (newState == SCR_ETaskState.FAILED)
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_FAILED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
				else if (newState == SCR_ETaskState.CANCELLED)
				{
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_CANCELED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupTaskSet(SCR_Task task, int byPlayerID)
	{
		if (task != m_Task)
			return;

		OnTaskCreated(task, byPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	protected BaseTransceiver GetHQTransceiver(notnull SCR_CampaignFaction faction)
	{
		SCR_CampaignMilitaryBaseComponent HQ = faction.GetMainBase();
		if (!HQ)
			return null;

		BaseRadioComponent radio = BaseRadioComponent.Cast(HQ.GetOwner().FindComponent(BaseRadioComponent));
		if (!radio || !radio.IsPowered())
			return null;

		return radio.GetTransceiver(0);
	}

	//------------------------------------------------------------------------------------------------
	protected BaseTransceiver GetPlayerTransceiver(int playerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!playerController)
			return null;

		IEntity player = playerController.GetMainEntity();
		if (!player)
			return null;

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(player.FindComponent(SCR_GadgetManagerComponent));
		if (!gadgetManager)
			return null;

		IEntity radioEnt = gadgetManager.GetGadgetByType(EGadgetType.RADIO);
		if (!radioEnt)
			return null;

		BaseRadioComponent radio = BaseRadioComponent.Cast(radioEnt.FindComponent(BaseRadioComponent));
		if (!radio || !radio.IsPowered())
			return null;

		return radio.GetTransceiver(0);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AIGroup GetFirstAssignedGroup(notnull SCR_Task task)
	{
		array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();

		if (!assignees.IsIndexValid(0))
			return null;

		SCR_TaskExecutorGroup assignedGroupExecutor = SCR_TaskExecutorGroup.Cast(assignees[0]);
		if (!assignedGroupExecutor)
			return null;

		return m_GroupsManager.FindGroup(assignedGroupExecutor.GetGroupID());
	}

	//------------------------------------------------------------------------------------------------
	protected void SendMessageToGroup(SCR_ETaskNotification notificationType, SCR_ETaskNotificationMsg msgType, int callerPlayerId, BaseTransceiver transmitter, SCR_AIGroup receiverGroup, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int callerGroupId = -1, int calledGroupId = -1, int gridX = -1, int gridY = -1)
	{
		if (!transmitter || !receiverGroup || !m_TaskNotificationManager || !m_FactionManager)
			return;

		SCR_CampaignFaction faction = GetFaction();
		if (!faction)
			return;

		SCR_BaseTaskNotificationData notificationData = SCR_BaseTaskNotificationData.Cast(m_TaskNotificationManager.GetTaskNotificationData(notificationType, msgType));
		if (!notificationData || !notificationData.CanSendNotification(receiverGroup))
			return;

		int factionId = m_FactionManager.GetFactionIndex(faction);

		SCR_TaskRadioMsg msg = new SCR_TaskRadioMsg();
		msg.SetTaskNotification(notificationType, msgType);
		msg.SetTaskNotificationData(notificationData);
		msg.SetBaseCallsign(baseCallsign);
		msg.SetFactionId(factionId);
		msg.SetFaction(faction);
		msg.SetGrid((gridX * 1000) + gridY);
		msg.SetCallerGroupId(callerGroupId);
		msg.SetCalledGroupId(calledGroupId);
		msg.SetCallerPlayerId(callerPlayerId);
		msg.SetEncryptionKey(transmitter.GetRadio().GetEncryptionKey());

		if (m_Task)
			msg.SetTaskPosition(m_Task.GetTaskPosition());

		#ifdef TASK_NOTIFICATION_DEBUG
		Print("[SCR_TaskNotificationComponent.SendMessageToGroup] notifType:"+SCR_Enum.GetEnumName(SCR_ETaskNotification, notificationType)+" msg:"+SCR_Enum.GetEnumName(SCR_ETaskNotificationMsg, msgType)+" to group:"+SCR_GroupHelperUI.GetTranslatedGroupNameAndRoleName(receiverGroup), LogLevel.DEBUG);
		#endif

		transmitter.BeginTransmissionFreq(msg, receiverGroup.GetRadioFrequency());
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		m_GroupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		m_TaskNotificationManager = SCR_TaskNotificationManagerComponent.GetInstance();
		m_Task = SCR_Task.Cast(owner);
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		m_Campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());

		if (!m_Campaign)
			return;

		SCR_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
		SCR_Task.GetOnTaskStateChanged().Insert(OnTaskStateChanged);

		m_TaskSystem = SCR_TaskSystem.GetInstance();

		SetGrid();

		if (m_GroupTaskManager)
			m_GroupTaskManager.GetOnGroupTaskSet().Insert(OnGroupTaskSet);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		SCR_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
		SCR_Task.GetOnTaskStateChanged().Remove(OnTaskStateChanged);

		if (m_GroupTaskManager)
			m_GroupTaskManager.GetOnGroupTaskSet().Remove(OnGroupTaskSet);
	}
}
