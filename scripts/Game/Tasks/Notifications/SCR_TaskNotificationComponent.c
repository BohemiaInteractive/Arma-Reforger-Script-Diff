[ComponentEditorProps(category: "GameScripted/Tasks", description: "Task notification component")]
class SCR_TaskNotificationComponentClass : ScriptComponentClass
{
	[Attribute(ENotification.GROUP_TASK_GROUP_ASSIGNED_TO_SAME_TASK.ToString(), uiwidget: UIWidgets.ComboBox, enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eGroupAssignedToSameTaskNotification;

	[Attribute(ENotification.GROUP_TASK_COMPLETED.ToString(), uiwidget: UIWidgets.ComboBox, enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eTaskCompletedNotification;

	[Attribute(ENotification.GROUP_TASK_FAILED.ToString(), uiwidget: UIWidgets.ComboBox, enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eTaskFailedNotification;

	[Attribute(ENotification.GROUP_TASK_CANCELED.ToString(), uiwidget: UIWidgets.ComboBox, enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eTaskCanceledNotification;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_Creation;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_YourGroupAssignedToTask;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_TaskAssignedBySquadLeader;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_TaskAssignedBySquadLeaderToHisMembers;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_TaskCompletedForCommander;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_TaskAssignedToYourGroupCompleted;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_TaskAssignedToYourGroupFailed;

	[Attribute(category: "Major notification")]
	protected ref SCR_TaskNotificationData m_TaskAssignedToYourGroupCanceled;

	//------------------------------------------------------------------------------------------------
	ENotification GetGroupAssignedToSameTaskNotification()
	{
		return m_eGroupAssignedToSameTaskNotification;
	}

	//------------------------------------------------------------------------------------------------
	ENotification GetTaskCompletedNotification()
	{
		return m_eTaskCompletedNotification;
	}

	//------------------------------------------------------------------------------------------------
	ENotification GetTaskFailedNotification()
	{
		return m_eTaskFailedNotification;
	}

	//------------------------------------------------------------------------------------------------
	ENotification GetTaskCanceledNotification()
	{
		return m_eTaskCanceledNotification;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataCreation()
	{
		return m_Creation;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataYourGroupAssignedToTask()
	{
		return m_YourGroupAssignedToTask;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataTaskAssignedBySquadLeader()
	{
		return m_TaskAssignedBySquadLeader;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataTaskAssignedBySquadLeaderToHisMembers()
	{
		return m_TaskAssignedBySquadLeaderToHisMembers;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataTaskCompletedForCommander()
	{
		return m_TaskCompletedForCommander;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataTaskAssignedToYourGroupCompleted()
	{
		return m_TaskAssignedToYourGroupCompleted;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataTaskAssignedToYourGroupFailed()
	{
		return m_TaskAssignedToYourGroupFailed;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskNotificationData GetDataTaskAssignedToYourGroupcCanceled()
	{
		return m_TaskAssignedToYourGroupCanceled;
	}
}

class SCR_TaskNotificationComponent : ScriptComponent
{
	protected const int POPUP_DURATION = 6;

	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected RplComponent m_RplComponent;
	protected SCR_Task m_Task;
	protected SCR_GroupTaskManagerComponent m_GroupTaskManager;

	protected int m_iGridX;
	protected int m_iGridY;

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
	string GetCreationText()
	{
		SCR_TaskNotificationComponentClass prefabData = SCR_TaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return string.Empty;

		// campaign military base task - uses base name in text
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask)
		{
			Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
			if (!faction)
				return string.Empty;

			SCR_CampaignMilitaryBaseComponent base = baseTask.GetMilitaryBase();
			if (!base)
				return string.Empty;

			return WidgetManager.Translate(prefabData.GetDataCreation().m_sText, base.GetFormattedBaseNameWithCallsign(faction));
		}

		return prefabData.GetDataCreation().m_sText;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayVONotification(string soundEventName, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int callerGroupId = 0, int calledGroupId = 0)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_TaskNotificationManagerEntity taskNotifManager = SCR_TaskNotificationManagerEntity.GetInstance();
		if (!taskNotifManager)
			return;

		taskNotifManager.PlayVONotification(soundEventName.Trim(), baseCallsign, callerGroupId, calledGroupId, m_iGridX, m_iGridY);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerCommander(int playerID)
	{
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		return faction && faction.IsPlayerCommander(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetGrid()
	{
		m_iGridX = 0;
		m_iGridY = 0;
		SCR_MapEntity.GetGridPos(m_Task.GetTaskPosition(), m_iGridX, m_iGridY);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskCreated(SCR_Task task)
	{
		if (task != m_Task)
			return;

		SCR_TaskNotificationComponentClass prefabData = SCR_TaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0)
			return;

		SCR_AIGroup group = m_GroupsManager.GetPlayerGroup(localPlayerId);
		if (!group)
			return;

		SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
		bool hasGroupTask = HasGroupTask(groupExecutor);

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		// task is created and data loaded, check if local group is assigned to the new task
		if (m_TaskSystem.CanTaskBeAssignedTo(task, groupExecutor))
		{
			if (task.IsTaskAssignedTo(groupExecutor))
			{
				SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataYourGroupAssignedToTask().m_sText, param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.CREATED, sound: SCR_SoundEvent.TASK_CREATED);
				PlayVONotification(prefabData.GetDataYourGroupAssignedToTask().m_sSoundEventName, baseCallsign);
			}
			else
			{
				if (hasGroupTask)
				{
					SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_CREATED, m_RplComponent.Id());
				}
				else
				{
					SCR_PopUpNotification.GetInstance().PopupMsg(GetCreationText(), param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.CREATED, sound: SCR_SoundEvent.TASK_CREATED);
					PlayVONotification(prefabData.GetDataCreation().m_sSoundEventName, baseCallsign);
				}
			}
		}
		else if (m_TaskSystem.IsTaskVisibleFor(task, groupExecutor))
		{
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_CREATED, m_RplComponent.Id());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != m_Task)
			return;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0 || requesterID == localPlayerId)
			return;

		SCR_TaskNotificationComponentClass prefabData = SCR_TaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		SCR_AIGroup group = m_GroupsManager.GetPlayerGroup(localPlayerId);
		if (!group)
			return;

		SCR_TaskExecutorGroup localGroupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
		if (!m_TaskSystem.IsTaskVisibleFor(task, localGroupExecutor))
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		SCR_TaskExecutorGroup newAssignedGroupExecutor = SCR_TaskExecutorGroup.Cast(executor);

		if (IsPlayerCommander(localPlayerId))
		{
			// player is commander and not requester, play group leader voice

			SCR_AIGroup newAssignedGroup = m_GroupsManager.FindGroup(newAssignedGroupExecutor.GetGroupID());
			if (!newAssignedGroup)
				return;

			SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataTaskAssignedBySquadLeader().m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(newAssignedGroup), param2: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT);
			PlayVONotification(prefabData.GetDataTaskAssignedBySquadLeader().m_sSoundEventName, baseCallsign, newAssignedGroupExecutor.GetGroupID());
			return;
		}

		if (m_TaskSystem.CanTaskBeAssignedTo(task, localGroupExecutor))
		{
			if (task.IsTaskAssignedTo(localGroupExecutor))
			{
				if (group.IsPlayerLeader(requesterID))
				{
					// assigned by leader, play group leader voice to his members
					SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataTaskAssignedBySquadLeaderToHisMembers().m_sText, param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT);
					PlayVONotification(prefabData.GetDataTaskAssignedBySquadLeaderToHisMembers().m_sSoundEventName, baseCallsign, 0, newAssignedGroupExecutor.GetGroupID());
				}
				else if (newAssignedGroupExecutor.GetGroupID() == localGroupExecutor.GetGroupID())
				{
					// assigned by commander, play commander voice
					SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataYourGroupAssignedToTask().m_sText, param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.ASSIGNED, sound: SCR_SoundEvent.TASK_ACCEPT);
					PlayVONotification(prefabData.GetDataYourGroupAssignedToTask().m_sSoundEventName, baseCallsign, 0, newAssignedGroupExecutor.GetGroupID());
				}
				else
				{
					SCR_NotificationsComponent.SendLocal(prefabData.GetGroupAssignedToSameTaskNotification(), newAssignedGroupExecutor.GetGroupID());
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != m_Task)
			return;

		SCR_TaskNotificationComponentClass prefabData = SCR_TaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0)
			return;

		SCR_AIGroup group = m_GroupsManager.GetPlayerGroup(localPlayerId);
		if (!group)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		// To task is assigned new group, check if local group is assigned
		SCR_TaskExecutorGroup localGroupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));

		if (task.IsTaskAssignedTo(localGroupExecutor))
		{
			if (newState == SCR_ETaskState.COMPLETED)
			{
				SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataTaskAssignedToYourGroupCompleted().m_sText, param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED);
				PlayVONotification(prefabData.GetDataTaskAssignedToYourGroupCompleted().m_sSoundEventName, baseCallsign);
			}
			else if (newState == SCR_ETaskState.FAILED)
			{
				SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataTaskAssignedToYourGroupFailed().m_sText, param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.FAILED, sound: SCR_SoundEvent.TASK_FAILED);
				PlayVONotification(prefabData.GetDataTaskAssignedToYourGroupFailed().m_sSoundEventName, baseCallsign);
			}
			else if (newState == SCR_ETaskState.CANCELLED)
			{
				SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataTaskAssignedToYourGroupcCanceled().m_sText, param1: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.CANCELED, sound: SCR_SoundEvent.TASK_CANCELED);
				PlayVONotification(prefabData.GetDataTaskAssignedToYourGroupcCanceled().m_sSoundEventName, baseCallsign);
			}
		}
		else
		{
			if (group.IsPlayerLeader(localPlayerId) && m_TaskSystem.IsTaskVisibleFor(task, localGroupExecutor))
			{
				if (newState == SCR_ETaskState.COMPLETED)
				{
					if (IsPlayerCommander(localPlayerId))
					{
						SCR_TaskExecutorGroup assignedGroupExecutor;
						array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();
						if (assignees && !assignees.IsEmpty())
						{
							foreach (SCR_TaskExecutor executor : assignees)
							{
								assignedGroupExecutor = SCR_TaskExecutorGroup.Cast(executor);
								if (assignedGroupExecutor)
									break;
							}
						}

						if (!assignedGroupExecutor)
							return;

						SCR_AIGroup assignedGroup = m_GroupsManager.FindGroup(assignedGroupExecutor.GetGroupID());
						if (!assignedGroup)
							return;

						// play only to commander
						SCR_PopUpNotification.GetInstance().PopupMsg(prefabData.GetDataTaskCompletedForCommander().m_sText, param1: SCR_GroupHelperUI.GetTranslatedGroupName(assignedGroup), param2: GetTaskName(task), duration: POPUP_DURATION, prio: SCR_EGroupTaskPopUpPriority.COMPLETED, sound: SCR_SoundEvent.TASK_SUCCEED);
						PlayVONotification(prefabData.GetDataTaskCompletedForCommander().m_sSoundEventName, baseCallsign, assignedGroupExecutor.GetGroupID());
					}
					else
					{
						SCR_NotificationsComponent.SendLocal(prefabData.GetTaskCompletedNotification(), m_RplComponent.Id());
					}
				}
				else if (newState == SCR_ETaskState.FAILED)
				{
					SCR_NotificationsComponent.SendLocal(prefabData.GetTaskFailedNotification(), m_RplComponent.Id());
				}
				else if (newState == SCR_ETaskState.CANCELLED)
				{
					SCR_NotificationsComponent.SendLocal(prefabData.GetTaskCanceledNotification(), m_RplComponent.Id());
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupTaskSet(SCR_Task task, int byPlayerID)
	{
		if (task != m_Task)
			return;

		OnTaskCreated(task);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		SCR_Task task = SCR_Task.Cast(GetOwner());
		if (!task)
			return true;

		SetGrid();

		OnTaskCreated(task);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (System.IsConsoleApp())
			return;

		m_Task = SCR_Task.Cast(owner);

		SCR_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
		SCR_Task.GetOnTaskStateChanged().Insert(OnTaskStateChanged);

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		m_GroupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();

		SetGrid();

		if (m_GroupTaskManager && m_RplComponent.IsMaster())
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

[BaseContainerProps()]
class SCR_TaskNotificationData
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	LocalizedString m_sText;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	string m_sSoundEventName;
}
