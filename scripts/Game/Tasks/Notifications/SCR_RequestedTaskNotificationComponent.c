[ComponentEditorProps(category: "GameScripted/Tasks", description: "Requested Task notification component")]
class SCR_RequestedTaskNotificationComponentClass : SCR_TaskNotificationComponentClass{
}

class SCR_RequestedTaskNotificationComponent : SCR_TaskNotificationComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void OnTaskCreated(SCR_Task task, int byPlayerID)
	{
		if (task != m_Task || !m_GroupsManager)
			return;

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
			return;

		SCR_CampaignFaction faction = GetFaction();
		if (!faction)
			return;

		BaseTransceiver playerTransceiver = GetPlayerTransceiver(byPlayerID);
		if (!playerTransceiver)
			return;

		array<SCR_AIGroup> playableGroups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		bool isCreatedByCommander = faction.IsPlayerCommander(byPlayerID);
		bool isCreatedByGroupLeader = IsPlayerGroupLeader(byPlayerID, playableGroups);

		SCR_TaskExecutorGroup groupExecutor;
		foreach (SCR_AIGroup group : playableGroups)
		{
			if (!ALLOW_SEND_TO_EMPTY_GROUP && group.GetPlayerCount() == 0)
				continue;

			groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
			if (!groupExecutor)
				continue;

			// doesn't send notification to commander group and spam yourself and group leaders
			if (isCreatedByCommander && group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
				continue;

			// is created by squad leader
			if (isCreatedByGroupLeader && group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
			{
				SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_CREATION_BY_SL, byPlayerID, playerTransceiver, group, baseCallsign, requestedTask.GetRequesterId(), 0, m_iGridX, m_iGridY);
			}
			else if (m_TaskSystem.CanTaskBeAssignedTo(task, groupExecutor))
			{
				if (!task.IsTaskAssignedTo(groupExecutor))
				{
					if (HasGroupTask(groupExecutor))
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_CREATION, byPlayerID, playerTransceiver, group, baseCallsign, requestedTask.GetRequesterId());
					else
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_CREATION, byPlayerID, playerTransceiver, group, baseCallsign, requestedTask.GetRequesterId(), 0, m_iGridX, m_iGridY);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != m_Task || !m_GroupsManager)
			return;

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
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

		BaseTransceiver hqTransceiver = GetHQTransceiver(faction);
		if (!hqTransceiver)
			return;

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		SCR_CampaignMilitaryBaseTaskEntity baseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(GetOwner());
		if (baseTask && baseTask.GetMilitaryBase())
			baseCallsign = baseTask.GetMilitaryBase().GetCallsign();

		if (faction.IsPlayerCommander(requesterID))
		{
			// commander send msg to SL and SM
			SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_GROUP_ASSIGNED_TO_TASK, requesterID, hqTransceiver, newAssignedGroup, baseCallsign, requestedTask.GetRequesterId(), newAssignedGroup.GetGroupID(), m_iGridX, m_iGridY);
		}

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
				if (task.IsTaskAssignedTo(groupExecutor))
				{
					if (group.IsPlayerLeader(requesterID))
					{
						BaseTransceiver playerTransceiver = GetPlayerTransceiver(requesterID);
						if (playerTransceiver)
							SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_SM, requesterID, playerTransceiver, newAssignedGroup, baseCallsign, -1, requestedTask.GetRequesterId(), m_iGridX, m_iGridY);
					}
					else if (newAssignedGroupExecutor.GetGroupID() != groupExecutor.GetGroupID())
					{
						// skip a current assigned group
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_GROUP_ASSIGNED_TO_SAME_TASK, requesterID, hqTransceiver, group, baseCallsign, newAssignedGroupExecutor.GetGroupID(), groupExecutor.GetGroupID(), m_iGridX, m_iGridY);
					}
				}
			}
			else if (m_TaskSystem.IsTaskVisibleFor(task, groupExecutor))
			{
				if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
				{
					BaseTransceiver playerTransceiver = GetPlayerTransceiver(requesterID);
					if (playerTransceiver)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_ASSIGNED_BY_SL_TO_COMMANDER, requesterID, playerTransceiver, group, baseCallsign, requesterGroupId, newAssignedGroup.GetGroupID(), m_iGridX, m_iGridY);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != m_Task || !m_GroupsManager)
			return;

		SCR_CampaignFaction faction = GetFaction();
		if (!faction)
			return;

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
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

			if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
			{
				if (newState == SCR_ETaskState.COMPLETED)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_COMPLETED_TO_COMMANDER, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				else if (newState == SCR_ETaskState.FAILED)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_FAILED_TO_COMMANDER, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				else if (newState == SCR_ETaskState.CANCELLED)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_CANCELED_TO_COMMANDER, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
			}
			else if (task.IsTaskAssignedTo(groupExecutor))
			{
				if (newState == SCR_ETaskState.COMPLETED)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_COMPLETED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				else if (newState == SCR_ETaskState.FAILED)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_FAILED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				else if (newState == SCR_ETaskState.CANCELLED)
					SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MAJOR_TASK_ASSIGNED_TO_YOUR_GROUP_CANCELED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
			}
			else
			{
				// your request
				if (requestedTask.GetRequesterId() == group.GetGroupID())
				{
					// group is requester
					if (newState == SCR_ETaskState.COMPLETED)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_YOUR_REQUEST_COMPLETED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
					else if (newState == SCR_ETaskState.FAILED)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_YOUR_REQUEST_FAILED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
					else if (newState == SCR_ETaskState.CANCELLED)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_YOUR_REQUEST_CANCELED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
				else if (m_TaskSystem.IsTaskVisibleFor(task, groupExecutor))
				{
					if (newState == SCR_ETaskState.COMPLETED)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_COMPLETED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
					else if (newState == SCR_ETaskState.FAILED)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_FAILED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
					else if (newState == SCR_ETaskState.CANCELLED)
						SendMessageToGroup(m_eTaskNotification, SCR_ETaskNotificationMsg.MINOR_TASK_CANCELED, 0, hqTransceiver, group, baseCallsign, group.GetGroupID(), 0, m_iGridX, m_iGridY);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRequestedGroupTaskSet(SCR_Task task, int byPlayerID, int requesterGroupId)
	{
		if (task != m_Task)
			return;

		OnTaskCreated(task, byPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (!m_Campaign)
			return;

		if (m_GroupTaskManager)
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
