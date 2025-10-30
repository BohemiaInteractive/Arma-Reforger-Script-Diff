[ComponentEditorProps(category: "GameScripted/Tasks", description: "Requested Task notification component")]
class SCR_RequestedTaskNotificationComponentClass : SCR_TaskNotificationComponentClass
{
	[Attribute(ENotification.GROUP_TASK_GROUP_ASSIGNED_TO_SAME_TASK.ToString(), uiwidget: UIWidgets.ComboBox, desc: "", params: "", enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eGroupAssignedToYourRequestNotification;

	[Attribute(ENotification.GROUP_TASK_YOUR_REQUEST_COMPLETED.ToString(), uiwidget: UIWidgets.ComboBox, desc: "", params: "", enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eYourRequestCompletedNotification;

	[Attribute(ENotification.GROUP_TASK_YOUR_REQUEST_FAILED.ToString(), uiwidget: UIWidgets.ComboBox, desc: "", params: "", enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eYourRequestFailedNotification;

	[Attribute(ENotification.GROUP_TASK_YOUR_REQUEST_CANCELED.ToString(), uiwidget: UIWidgets.ComboBox, desc: "", params: "", enumType: ENotification, category: "Minor notification")]
	protected ENotification m_eYourRequestCanceledNotification;

	//------------------------------------------------------------------------------------------------
	ENotification GetGroupAssignedToYourRequestNotification()
	{
		return m_eGroupAssignedToYourRequestNotification;
	}

	//------------------------------------------------------------------------------------------------
	ENotification GetYourRequestCompletedNotification()
	{
		return m_eYourRequestCompletedNotification;
	}

	//------------------------------------------------------------------------------------------------
	ENotification GetYourRequestFailedNotification()
	{
		return m_eYourRequestFailedNotification;
	}

	//------------------------------------------------------------------------------------------------
	ENotification GetYourRequestCanceledNotification()
	{
		return m_eYourRequestCanceledNotification;
	}
}

class SCR_RequestedTaskNotificationComponent : SCR_TaskNotificationComponent
{
	//------------------------------------------------------------------------------------------------
	override string GetCreationText()
	{
		SCR_TaskNotificationComponentClass prefabData = SCR_TaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return string.Empty;

		// requested task - uses group name in text
		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(GetOwner());
		if (requestedTask)
		{
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupsManager)
				return string.Empty;

			SCR_AIGroup aiGroup = groupsManager.FindGroup(requestedTask.GetRequesterId());
			if (!aiGroup)
				return string.Empty;

			return WidgetManager.Translate(prefabData.GetDataCreation().m_sText, SCR_GroupHelperUI.GetTranslatedGroupName(aiGroup));
		}

		return prefabData.GetDataCreation().m_sText;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTaskCreated(SCR_Task task)
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

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
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
				PlayVONotification(prefabData.GetDataYourGroupAssignedToTask().m_sSoundEventName, baseCallsign, requestedTask.GetRequesterId());
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
					PlayVONotification(prefabData.GetDataCreation().m_sSoundEventName, baseCallsign, requestedTask.GetRequesterId());
				}
			}
		}
		else if (m_TaskSystem.IsTaskVisibleFor(task, groupExecutor))
		{
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_CREATED, m_RplComponent.Id());
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != m_Task)
			return;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0 || requesterID == localPlayerId)
			return;

		SCR_RequestedTaskNotificationComponentClass prefabData = SCR_RequestedTaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		SCR_AIGroup group = m_GroupsManager.GetPlayerGroup(localPlayerId);
		if (!group)
			return;

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
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
					PlayVONotification(prefabData.GetDataYourGroupAssignedToTask().m_sSoundEventName, baseCallsign, requestedTask.GetRequesterId(), newAssignedGroupExecutor.GetGroupID());
				}
				else
				{
					SCR_NotificationsComponent.SendLocal(prefabData.GetGroupAssignedToSameTaskNotification(), newAssignedGroupExecutor.GetGroupID());
				}
			}
		}
		else
		{
			if (requestedTask.GetRequesterId() == group.GetGroupID())
				SCR_NotificationsComponent.SendLocal(prefabData.GetGroupAssignedToYourRequestNotification(), m_RplComponent.Id(), newAssignedGroupExecutor.GetGroupID());
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != m_Task)
			return;

		SCR_RequestedTaskNotificationComponentClass prefabData = SCR_RequestedTaskNotificationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerId == 0)
			return;

		SCR_AIGroup group = m_GroupsManager.GetPlayerGroup(localPlayerId);
		if (!group)
			return;

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		// To task is assigned new group, check if local group is assigned
		SCR_TaskExecutorGroup localGroupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));

		if (task.IsTaskAssignedTo(localGroupExecutor))
		{
			// assigned
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
			// not assigned
			if (requestedTask.GetRequesterId() == group.GetGroupID())
			{
				// group is requester
				if (newState == SCR_ETaskState.COMPLETED)
					SCR_NotificationsComponent.SendLocal(prefabData.GetYourRequestCompletedNotification(), m_RplComponent.Id());
				else if (newState == SCR_ETaskState.FAILED)
					SCR_NotificationsComponent.SendLocal(prefabData.GetYourRequestFailedNotification(), m_RplComponent.Id());
				else if (newState == SCR_ETaskState.CANCELLED)
					SCR_NotificationsComponent.SendLocal(prefabData.GetYourRequestCanceledNotification(), m_RplComponent.Id());
			}
			else if (group.IsPlayerLeader(localPlayerId) && m_TaskSystem.IsTaskVisibleFor(task, localGroupExecutor))
			{
				if (newState == SCR_ETaskState.COMPLETED)
					SCR_NotificationsComponent.SendLocal(prefabData.GetTaskCompletedNotification(), m_RplComponent.Id());
				else if (newState == SCR_ETaskState.FAILED)
					SCR_NotificationsComponent.SendLocal(prefabData.GetTaskFailedNotification(), m_RplComponent.Id());
				else if (newState == SCR_ETaskState.CANCELLED)
					SCR_NotificationsComponent.SendLocal(prefabData.GetTaskCanceledNotification(), m_RplComponent.Id());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRequestedGroupTaskSet(SCR_Task task, int byPlayerID, int requesterGroupId)
	{
		if (task != m_Task)
			return;

		OnTaskCreated(task);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (System.IsConsoleApp())
			return;

		if (m_GroupTaskManager && m_RplComponent.IsMaster())
			m_GroupTaskManager.GetOnRequestedGroupTaskSet().Insert(OnRequestedGroupTaskSet);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		if (m_GroupTaskManager)
			m_GroupTaskManager.GetOnRequestedGroupTaskSet().Remove(OnRequestedGroupTaskSet);
	}
}
