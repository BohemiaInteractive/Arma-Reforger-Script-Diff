[EnumBitFlag()]
enum SCR_ETaskState
{
	CREATED,
	ASSIGNED,
	PROGRESSED,
	COMPLETED,
	FAILED,
	CANCELLED
}

[EnumLinear()]
enum SCR_ETaskOwnership
{
	NONE,
	EXECUTOR,
	GROUP,
	FACTION,
	EVERYONE
}

[EnumLinear()]
enum SCR_ETaskVisibility
{
	NONE,
	EXECUTOR,
	ASSIGNEES,
	GROUP,
	FACTION,
	EVERYONE
}

[EnumLinear()]
enum SCR_ETaskUIVisibility
{
	NONE,
	MAP_ONLY,
	LIST_ONLY,
	ALL
}

void TaskStateInvokerDelegate(SCR_Task task, SCR_ETaskState newState);
typedef func TaskStateInvokerDelegate;
typedef ScriptInvokerBase<TaskStateInvokerDelegate> SCR_TaskStateInvoker;

void TaskOwnershipInvokerDelegate(SCR_Task task, SCR_ETaskOwnership newOwnership);
typedef func TaskOwnershipInvokerDelegate;
typedef ScriptInvokerBase<TaskOwnershipInvokerDelegate> SCR_TaskOwnershipInvoker;

void TaskVisibilityInvokerDelegate(SCR_Task task, SCR_ETaskVisibility newVisibility);
typedef func TaskVisibilityInvokerDelegate;
typedef ScriptInvokerBase<TaskVisibilityInvokerDelegate> SCR_TaskVisibilityInvoker;

void TaskUIVisibilityInvokerDelegate(SCR_Task task, SCR_ETaskUIVisibility newUIVisibility);
typedef func TaskUIVisibilityInvokerDelegate;
typedef ScriptInvokerBase<TaskUIVisibilityInvokerDelegate> SCR_TaskUIVisibilityInvoker;

void TaskExecutorInvokerDelegate(SCR_Task task, SCR_TaskExecutor executor);
typedef func TaskExecutorInvokerDelegate;
typedef ScriptInvokerBase<TaskExecutorInvokerDelegate> SCR_TaskExecutorInvoker;

void TaskExecutorRequesterInvokerDelegate(SCR_Task task, SCR_TaskExecutor executor, int requesterID);
typedef func TaskExecutorRequesterInvokerDelegate;
typedef ScriptInvokerBase<TaskExecutorRequesterInvokerDelegate> SCR_TaskExecutorRequesterInvoker;

void TaskFactionInvokerDelegate(SCR_Task task, string factionKey);
typedef func TaskFactionInvokerDelegate;
typedef ScriptInvokerBase<TaskFactionInvokerDelegate> SCR_TaskFactionInvoker;

void TaskGroupInvokerDelegate(SCR_Task task, int groupID);
typedef func TaskGroupInvokerDelegate;
typedef ScriptInvokerBase<TaskGroupInvokerDelegate> SCR_TaskGroupInvoker;

class SCR_TaskClass : GenericEntityClass
{
}

class SCR_Task : GenericEntity
{	
	[Attribute()]
	protected string m_sTaskID;
	
	[Attribute()]
	protected ref SCR_TaskUIInfo m_TaskUIInfo;
	
	[Attribute(defvalue: SCR_ETaskState.CREATED.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskState))]
	protected SCR_ETaskState m_eTaskState;
	
	[Attribute(defvalue: SCR_ETaskOwnership.EVERYONE.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskOwnership))]
	protected SCR_ETaskOwnership m_eTaskOwnership;
	
	[Attribute(defvalue: SCR_ETaskVisibility.EVERYONE.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskVisibility))]
	protected SCR_ETaskVisibility m_eTaskVisibility;
	
	[Attribute(defvalue: SCR_ETaskUIVisibility.ALL.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskUIVisibility))]
	protected SCR_ETaskUIVisibility m_eTaskUIVisibility;
	
	[Attribute()]
	protected ref array<string> m_aOwnerFactionKeys;
	
	[Attribute()]
	protected ref array<int> m_aOwnerGroupIDs;
	
	[Attribute()]
	protected ref array<ref SCR_TaskExecutor> m_aOwnerExecutors;
	
	[Attribute()]
	protected ref array<ref SCR_TaskExecutor> m_aAssignees;
	
	protected ref SCR_TaskData m_TaskData;
	
	protected SCR_TaskSystem m_TaskSystem;	
	protected SCR_TaskUIComponent m_UIComponent;
	protected SCR_EditableEntityComponent m_EditableEntityComponent;
	
	protected static ref SCR_TaskStateInvoker s_OnTaskStateChanged;
	protected static ref SCR_TaskOwnershipInvoker s_OnTaskOwnershipChanged;
	protected static ref SCR_TaskVisibilityInvoker s_OnTaskVisibilityChanged;
	protected static ref SCR_TaskUIVisibilityInvoker s_OnTaskUIVisibilityChanged;
	
	protected static ref SCR_TaskExecutorRequesterInvoker s_OnTaskAssigneeAdded;
	protected static ref SCR_TaskExecutorRequesterInvoker s_OnTaskAssigneeRemoved;
	
	protected static ref SCR_TaskFactionInvoker s_OnOwnerFactionAdded;
	protected static ref SCR_TaskFactionInvoker s_OnOwnerFactionRemoved;
	
	protected static ref SCR_TaskGroupInvoker s_OnOwnerGroupAdded;
	protected static ref SCR_TaskGroupInvoker s_OnOwnerGroupRemoved;
	
	protected static ref SCR_TaskExecutorInvoker s_OnOwnerExecutorAdded;
	protected static ref SCR_TaskExecutorInvoker s_OnOwnerExecutorRemoved;
	
	protected ref ScriptInvokerVoid m_OnDisplayDataChanged; // Invoked when task name, description or icon changes
	
#ifdef WORKBENCH
	protected ref DebugTextWorldSpace m_DebugText;
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Returns task data instance
	//! \return
	SCR_TaskData GetTaskData()
	{
		return m_TaskData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns task UI info
	//! \return
	SCR_TaskUIInfo GetTaskUIInfo()
	{
		if (!m_TaskData)
			return null;
		
		if (!m_TaskData.m_UIInfo)
			m_TaskData.m_UIInfo = new SCR_TaskUIInfo();
		
		return m_TaskData.m_UIInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns task ID
	//! \return
	string GetTaskID()
	{
		if (m_TaskData)
			return m_TaskData.m_sID;
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets ID of task
	//! \param[in] taskID
	void SetTaskID(string taskID)
	{
		Rpc_SetTaskID(taskID);
		Rpc(Rpc_SetTaskID, taskID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskID(string taskID)
	{
		if (m_TaskData)
			m_TaskData.m_sID = taskID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns task author ID
	//! \return
	int GetAuthorID()
	{
		if (m_TaskData)
			return m_TaskData.m_iAuthorId;
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets author id of task
	//! \param[in] taskID
	void SetAuthorID(int authorId)
	{
		Rpc_SetAuthorID(authorId);
		Rpc(Rpc_SetAuthorID, authorId);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetAuthorID(int authorId)
	{
		if (m_TaskData)
			m_TaskData.m_iAuthorId = authorId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns name of task
	//! \param[out] params
	//! \return
	LocalizedString GetTaskName(out array<LocalizedString> params = null)
	{
		if (m_TaskData && m_TaskData.m_UIInfo)
			return m_TaskData.m_UIInfo.GetUnformattedName(params);
		
		return LocalizedString.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets name of task
	//! \param[in] taskName
	//! \param[in] taskNameParams
	void SetTaskName(LocalizedString taskName, array<LocalizedString> taskNameParams = null)
	{
		Rpc_SetTaskName(taskName, taskNameParams);
		Rpc(Rpc_SetTaskName, taskName, taskNameParams);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskName(LocalizedString taskName, array<LocalizedString> taskNameParams)
	{
		if (!m_TaskData || !m_TaskData.m_UIInfo)
			return;
		
		m_TaskData.m_UIInfo.SetUnformattedName(taskName, taskNameParams);
		
		if (m_EditableEntityComponent)
			m_EditableEntityComponent.GetInfo(this).SetName(taskName);
		
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns description of task
	//! \param[out] params
	//! \return
	LocalizedString GetTaskDescription(out array<LocalizedString> params = null)
	{
		if (m_TaskData && m_TaskData.m_UIInfo)
			return m_TaskData.m_UIInfo.GetUnformattedDescription(params);
		
		return LocalizedString.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets description of task
	//! \param[in] taskDesc
	//! \param[in] taskDescParams
	void SetTaskDescription(LocalizedString taskDesc, array<LocalizedString> taskDescParams = null)
	{
		Rpc_SetTaskDescription(taskDesc, taskDescParams);
		Rpc(Rpc_SetTaskDescription, taskDesc, taskDescParams);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskDescription(LocalizedString taskDesc, array<LocalizedString> taskDescParams)
	{
		if (!m_TaskData || !m_TaskData.m_UIInfo)
			return;
		
		m_TaskData.m_UIInfo.SetUnformattedDescription(taskDesc, taskDescParams);
		
		if (m_EditableEntityComponent)
			m_EditableEntityComponent.GetInfo(this).SetDescription(taskDesc);
		
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns task icon path
	//! \return
	string GetTaskIconPath()
	{
		if (m_TaskData && m_TaskData.m_UIInfo)
			return m_TaskData.m_UIInfo.GetImageSetPath();
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets task icon path
	//! \param[in] iconPath
	void SetTaskIconPath(ResourceName iconPath)
	{
		Rpc_SetTaskIconPath(iconPath);
		Rpc(Rpc_SetTaskIconPath, iconPath);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskIconPath(ResourceName iconPath)
	{
		if (!m_TaskData || !m_TaskData.m_UIInfo)
			return;
		
		m_TaskData.m_UIInfo.SetIconPath(iconPath);
		
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns name of task icon set
	//! \return
	string GetTaskIconSetName()
	{
		if (m_TaskData && m_TaskData.m_UIInfo)
			return m_TaskData.m_UIInfo.GetIconSetName();
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets name of task icon set
	//! \param[in] iconSetName
	void SetTaskIconSetName(string iconSetName)
	{
		Rpc_SetTaskIconSetName(iconSetName);
		Rpc(Rpc_SetTaskIconSetName, iconSetName);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskIconSetName(string iconSetName)
	{
		if (!m_TaskData || !m_TaskData.m_UIInfo)
			return;
		
		m_TaskData.m_UIInfo.SetIconSetName(iconSetName);
		
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns number of assignees for this task
	//! \return
	int GetTaskAssigneeCount()
	{
		if (!m_TaskData)
			return 0;
		
		return m_TaskData.m_aAssignees.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns number of assigned players for this task; If recursive is true, it will check assigned groups as well
	//! \param[in] recursive
	//! \return
	int GetTaskAssigneePlayerCount(bool recursive = true)
	{
		if (!m_TaskData)
			return 0;
		
		if (!m_TaskData.m_aAssignees || m_TaskData.m_aAssignees.IsEmpty())
			return 0;
		
		SCR_GroupsManagerComponent groupsManagerComponent;
		if (recursive)
			groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		
		if (recursive && !groupsManagerComponent)
			return 0;
		
		int count, groupID;
		SCR_TaskExecutorGroup executorGroup;
		SCR_AIGroup group;
		foreach (SCR_TaskExecutor executor : m_TaskData.m_aAssignees)
		{
			if (SCR_TaskExecutorPlayer.Cast(executor))
			{
				count++;
			}
			else if (recursive)
			{		
				executorGroup = SCR_TaskExecutorGroup.Cast(executor);
				if (!executorGroup)
					continue;
				
				groupID = executorGroup.GetGroupID();				
				group = groupsManagerComponent.FindGroup(groupID);
				if (!group)
					continue;
					
				count += group.GetPlayerCount();
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns array of assignees for this task
	//! \return
	array<ref SCR_TaskExecutor> GetTaskAssignees()
	{
		if (!m_TaskData)
			return null;
		
		return m_TaskData.m_aAssignees;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns array of player assignees for this task
	//! \return
	array<int> GetTaskAssigneePlayerIDs()
	{
		array<int> assigneePlayerIDs = {};
		if (!m_TaskData || !m_TaskData.m_aAssignees)
			return assigneePlayerIDs;

		SCR_TaskExecutorPlayer playerAssignee;
		SCR_TaskExecutorGroup groupAssignee;
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		foreach (SCR_TaskExecutor assignee : m_TaskData.m_aAssignees)
		{
			playerAssignee = SCR_TaskExecutorPlayer.Cast(assignee);
			if (playerAssignee)
				assigneePlayerIDs.Insert(playerAssignee.GetPlayerID());

			groupAssignee = SCR_TaskExecutorGroup.Cast(assignee);
			if (groupAssignee)
			{
				SCR_AIGroup group = groupsManager.FindGroup(groupAssignee.GetGroupID());
				if (group)
				{
					assigneePlayerIDs.InsertAll(group.GetPlayerIDs());
				}
			}
		}

		return assigneePlayerIDs;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if specified executor is assigned to this task and provides matching executor
	//! \param[in] executor
	//! \param[out] match 
	//! \return
	bool IsTaskAssignedTo(SCR_TaskExecutor executor, out SCR_TaskExecutor match = null)
	{
		if (!m_TaskData || !executor)
			return false;
		
		if (!m_TaskData.m_aAssignees || m_TaskData.m_aAssignees.IsEmpty())
			return false;
		
		match = SCR_TaskExecutor.FindMatchingTaskExecutor(m_TaskData.m_aAssignees, executor, true);
		if (!match)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to add a new assignee to this task
	//! \param[in] executor
	//! \param[in] listenToInvokers
	//! \param[in] requesterID
	//! \return
	bool AddTaskAssignee(SCR_TaskExecutor executor, bool listenToInvokers = true, int requesterID = 0)
	{
		if (!m_TaskData || !executor)
			return false;
		
		if (!m_TaskData.m_aAssignees)
			m_TaskData.m_aAssignees = new array<ref SCR_TaskExecutor>();
		
		if (IsTaskAssignedTo(executor))
			return false;
		
		m_TaskData.m_aAssignees.Insert(executor);
		
		if (listenToInvokers)
		{
			SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
			if (executorPlayer)
			{				
				int playerID = executorPlayer.GetPlayerID();
				
				PlayerManager playerManager = GetGame().GetPlayerManager();
				if (!playerManager)
					return false;
				
				SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerID));
				if (!playerController)
					return false;
				
				SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
				if (groupComponent)
					groupComponent.GetOnGroupChanged().Insert(OnGroupChanged);
				
				SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
				if (!factionManager)
					return false;
				
				factionManager.GetOnPlayerFactionChanged_S().Insert(OnPlayerFactionChanged);
			}
			
			SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
			if (executorGroup)
			{
				int groupID = executorGroup.GetGroupID();
				
				SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
				if (!groupsManagerComponent)
					return false;
				
				groupsManagerComponent.GetOnPlayableGroupRemoved().Insert(OnGroupRemoved);
				
				SCR_AIGroup group = groupsManagerComponent.FindGroup(groupID);
				if (!group)
					return false;
				
				group.GetOnPlayerAdded().Insert(OnPlayerAddedToGroup);
				group.GetOnPlayerRemoved().Insert(OnPlayerRemovedFromGroup);
			}
			// TODO: Fix player/group handling when entity changes or player is removed from group
			/*else
			{
				IEntity ent = executor.GetEntity();
			
				SCR_FactionAffiliationComponent factionAffiliation = SCR_FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
				if (!factionAffiliation)
					return false;
				
				factionAffiliation.GetOnFactionChanged().Insert(OnFactionChanged);
			}*/
		}
		
		if (s_OnTaskAssigneeAdded)
			s_OnTaskAssigneeAdded.Invoke(this, executor, requesterID);
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);	
		if (executorPlayer)
		{
			Rpc(Rpc_AddTaskAssigneePlayer, executorPlayer, requesterID);
			return true;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{
			Rpc(Rpc_AddTaskAssigneeEntity, executorEntity, requesterID);
			return true;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
		{
			Rpc(Rpc_AddTaskAssigneeGroup, executorGroup, requesterID);
			return true;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddTaskAssignee_Proxy(SCR_TaskExecutor executor, int requesterID)
	{
		if (!m_TaskData || !executor)
			return;
		
		if (!m_TaskData.m_aAssignees)
			m_TaskData.m_aAssignees = new array<ref SCR_TaskExecutor>();
		
		if (IsTaskAssignedTo(executor))
			return;
		
		m_TaskData.m_aAssignees.Insert(executor);
			
		if (s_OnTaskAssigneeAdded)
			s_OnTaskAssigneeAdded.Invoke(this, executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddTaskAssigneePlayer(SCR_TaskExecutorPlayer executor, int requesterID)
	{
		AddTaskAssignee_Proxy(executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddTaskAssigneeEntity(SCR_TaskExecutorEntity executor, int requesterID)
	{
		AddTaskAssignee_Proxy(executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddTaskAssigneeGroup(SCR_TaskExecutorGroup executor, int requesterID)
	{
		AddTaskAssignee_Proxy(executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to remove an assignee from this task
	//! \param[in] executor
	//! \param[in] listenToInvokers
	//! \param[in] requesterID
	//! \return
	bool RemoveTaskAssignee(SCR_TaskExecutor executor, bool listenToInvokers = true, int requesterID = 0)
	{
		if (!m_TaskData || !executor)
			return false;
		
		if (!m_TaskData.m_aAssignees)
			return false;
		
		SCR_TaskExecutor assignee;
		if (!IsTaskAssignedTo(executor, assignee))
			return false;
		
		m_TaskData.m_aAssignees.RemoveItem(assignee);
		
		if (listenToInvokers)
		{
			SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
			if (executorPlayer)
			{	
				int playerID = executorPlayer.GetPlayerID();
				
				PlayerManager playerManager = GetGame().GetPlayerManager();
				if (!playerManager)
					return false;
				
				SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerID));
				if (!playerController)
					return false;
				
				SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
				if (groupComponent)
					groupComponent.GetOnGroupChanged().Remove(OnGroupChanged);
				
				SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
				if (!factionManager)
					return false;
				
				factionManager.GetOnPlayerFactionChanged_S().Remove(OnPlayerFactionChanged);
			}
			
			SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
			if (executorGroup)
			{
				int groupID = executorGroup.GetGroupID();
				
				SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
				if (!groupsManagerComponent)
					return false;
				
				groupsManagerComponent.GetOnPlayableGroupRemoved().Remove(OnGroupRemoved);
				
				SCR_AIGroup group = groupsManagerComponent.FindGroup(groupID);
				if (!group)
					return false;
				
				group.GetOnPlayerAdded().Remove(OnPlayerAddedToGroup);
				group.GetOnPlayerRemoved().Remove(OnPlayerRemovedFromGroup);
			}
			// TODO: Fix player/group handling when entity changes or player is removed from group
			/*else
			{
				IEntity ent = executor.GetEntity();
			
				SCR_FactionAffiliationComponent factionAffiliation = SCR_FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
				if (!factionAffiliation)
					return false;
				
				factionAffiliation.GetOnFactionChanged().Remove(OnFactionChanged);
			}*/
		}
		
		if (s_OnTaskAssigneeRemoved)
			s_OnTaskAssigneeRemoved.Invoke(this, assignee, requesterID);
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);	
		if (executorPlayer)
		{
			Rpc(Rpc_RemoveTaskAssigneePlayer, executorPlayer, requesterID);
			return true;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{
			Rpc(Rpc_RemoveTaskAssigneeEntity, executorEntity, requesterID);
			return true;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
		{
			Rpc(Rpc_RemoveTaskAssigneeGroup, executorGroup, requesterID);
			return true;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveTaskAssignee_Proxy(SCR_TaskExecutor executor, int requesterID)
	{
		if (!m_TaskData || !executor)
			return;
		
		if (!m_TaskData.m_aAssignees)
			return;
		
		SCR_TaskExecutor assignee;
		if (!IsTaskAssignedTo(executor, assignee))
			return;
		
		m_TaskData.m_aAssignees.RemoveItem(assignee);
			
		if (s_OnTaskAssigneeRemoved)
			s_OnTaskAssigneeRemoved.Invoke(this, assignee, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveTaskAssigneePlayer(SCR_TaskExecutorPlayer executor, int requesterID)
	{
		RemoveTaskAssignee_Proxy(executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveTaskAssigneeEntity(SCR_TaskExecutorEntity executor, int requesterID)
	{
		RemoveTaskAssignee_Proxy(executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveTaskAssigneeGroup(SCR_TaskExecutorGroup executor, int requesterID)
	{
		RemoveTaskAssignee_Proxy(executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_ETaskState GetTaskState()
	{
		if (m_TaskData)
			return m_TaskData.m_eState;
		
		return SCR_ETaskState.CREATED;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] state
	void SetTaskState(SCR_ETaskState state)
	{
		if (!m_TaskData)
			return;
		
		Rpc_SetTaskState(state);
		Rpc(Rpc_SetTaskState, state);
		
		if (GetTaskAssigneeCount() > 0 && (state == SCR_ETaskState.COMPLETED || state == SCR_ETaskState.FAILED || state == SCR_ETaskState.CANCELLED))
		{
			for (int i = m_TaskData.m_aAssignees.Count() - 1; i >= 0; --i)
			{
				RemoveTaskAssignee(m_TaskData.m_aAssignees[i]);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskState(SCR_ETaskState state)
	{	
		if (!m_TaskData)
			return;
		
		m_TaskData.m_eState = state;
		
		if (s_OnTaskStateChanged)
			s_OnTaskStateChanged.Invoke(this, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	//! \return
	SCR_ETaskOwnership GetTaskOwnership()
	{
		if (m_TaskData)
			return m_TaskData.m_eOwnership;
		
		return SCR_ETaskOwnership.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ownership
	void SetTaskOwnership(SCR_ETaskOwnership ownership)
	{
		Rpc_SetTaskOwnership(ownership);
		Rpc(Rpc_SetTaskOwnership, ownership);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskOwnership(SCR_ETaskOwnership ownership)
	{
		if (!m_TaskData)
			return;
		
		m_TaskData.m_eOwnership = ownership;
		
		if (s_OnTaskOwnershipChanged)
			s_OnTaskOwnershipChanged.Invoke(this, ownership);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_ETaskVisibility GetTaskVisibility()
	{
		if (m_TaskData)
			return m_TaskData.m_eVisibility;
		
		return SCR_ETaskVisibility.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] visibility
	void SetTaskVisibility(SCR_ETaskVisibility visibility)
	{
		Rpc_SetTaskVisibility(visibility);
		Rpc(Rpc_SetTaskVisibility, visibility);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskVisibility(SCR_ETaskVisibility visibility)
	{
		if (!m_TaskData)
			return;
		
		m_TaskData.m_eVisibility = visibility;
		
		if (s_OnTaskVisibilityChanged)
			s_OnTaskVisibilityChanged.Invoke(this, visibility);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_ETaskUIVisibility GetTaskUIVisibility()
	{
		if (m_TaskData)
			return m_TaskData.m_eUIVisibility;
		
		return SCR_ETaskUIVisibility.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] visibility
	void SetTaskUIVisibility(SCR_ETaskUIVisibility visibility)
	{
		Rpc_SetTaskUIVisibility(visibility);
		Rpc(Rpc_SetTaskUIVisibility, visibility);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskUIVisibility(SCR_ETaskUIVisibility visibility)
	{
		if (!m_TaskData)
			return;
		
		m_TaskData.m_eUIVisibility = visibility;
		
		if (s_OnTaskUIVisibilityChanged)
			s_OnTaskUIVisibilityChanged.Invoke(this, visibility);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int GetOwnerFactionCount()
	{
		if (!m_TaskData)
			return 0;
		
		return m_TaskData.m_aOwnerFactionKeys.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	array<string> GetOwnerFactionKeys()
	{
		if (m_TaskData)
			return m_TaskData.m_aOwnerFactionKeys;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] factionKey
	void AddOwnerFactionKey(FactionKey factionKey)
	{
		Rpc_AddOwnerFactionKey(factionKey);
		Rpc(Rpc_AddOwnerFactionKey, factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddOwnerFactionKey(FactionKey factionKey)
	{
		if (!m_TaskData || factionKey.IsEmpty())
			return;
		
		if (!m_TaskData.m_aOwnerFactionKeys)
			m_TaskData.m_aOwnerFactionKeys = new array<string>();
		
		if (m_TaskData.m_aOwnerFactionKeys.Contains(factionKey))
			return;
		
		m_TaskData.m_aOwnerFactionKeys.Insert(factionKey);
		
		if (s_OnOwnerFactionAdded)
			s_OnOwnerFactionAdded.Invoke(this, factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] factionKey
	void RemoveOwnerFactionKey(FactionKey factionKey)
	{
		Rpc_RemoveOwnerFactionKey(factionKey);
		Rpc(Rpc_RemoveOwnerFactionKey, factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveOwnerFactionKey(FactionKey factionKey)
	{
		if (!m_TaskData || factionKey.IsEmpty())
			return;
		
		if (!m_TaskData.m_aOwnerFactionKeys || !m_TaskData.m_aOwnerFactionKeys.Contains(factionKey))
			return;
		
		m_TaskData.m_aOwnerFactionKeys.RemoveItem(factionKey);
		
		if (s_OnOwnerFactionRemoved)
			s_OnOwnerFactionRemoved.Invoke(this, factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int GetOwnerGroupCount()
	{
		if (!m_TaskData)
			return 0;
		
		return m_TaskData.m_aOwnerGroupIDs.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	array<int> GetOwnerGroupIDs()
	{
		if (m_TaskData)
			return m_TaskData.m_aOwnerGroupIDs;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	void AddOwnerGroupID(int groupID)
	{
		Rpc_AddOwnerGroupID(groupID);
		Rpc(Rpc_AddOwnerGroupID, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddOwnerGroupID(int groupID)
	{
		if (!m_TaskData || groupID <= -1)
			return;
		
		if (!m_TaskData.m_aOwnerGroupIDs)
			m_TaskData.m_aOwnerGroupIDs = new array<int>();
		
		if (m_TaskData.m_aOwnerGroupIDs.Contains(groupID))
			return;
		
		m_TaskData.m_aOwnerGroupIDs.Insert(groupID);
		
		if (s_OnOwnerGroupAdded)
			s_OnOwnerGroupAdded.Invoke(this, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	void RemoveOwnerGroupID(int groupID)
	{
		Rpc_RemoveOwnerGroupID(groupID);
		Rpc(Rpc_RemoveOwnerGroupID, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveOwnerGroupID(int groupID)
	{
		if (!m_TaskData || groupID <= -1)
			return;
		
		if (!m_TaskData.m_aOwnerGroupIDs || !m_TaskData.m_aOwnerGroupIDs.Contains(groupID))
			return;
		
		m_TaskData.m_aOwnerGroupIDs.RemoveItem(groupID);
		
		if (s_OnOwnerGroupRemoved)
			s_OnOwnerGroupRemoved.Invoke(this, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int GetOwnerExecutorCount()
	{
		if (!m_TaskData)
			return 0;
		
		return m_TaskData.m_aOwnerExecutors.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	array<ref SCR_TaskExecutor> GetOwnerExecutors()
	{
		if (!m_TaskData)
			return null;
		
		return m_TaskData.m_aOwnerExecutors;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to add an owner executor to this task
	//! \param[in] executor
	//! \return
	bool AddOwnerExecutor(SCR_TaskExecutor executor)
	{
		if (!m_TaskData || !executor)
			return false;
		
		if (!m_TaskData.m_aOwnerExecutors)
			m_TaskData.m_aOwnerExecutors = new array<ref SCR_TaskExecutor>();
		
		SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(m_TaskData.m_aOwnerExecutors, executor);
		if (match)
			return false;
		
		m_TaskData.m_aOwnerExecutors.Insert(executor);
		
		if (s_OnOwnerExecutorAdded)
			s_OnOwnerExecutorAdded.Invoke(this, executor);
				
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);	
		if (executorPlayer)
		{
			Rpc(Rpc_AddOwnerExecutorPlayer, executorPlayer);
			return true;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{
			Rpc(Rpc_AddOwnerExecutorEntity, executorEntity);
			return true;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
		{
			Rpc(Rpc_AddOwnerExecutorGroup, executorGroup);
			return true;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddOwnerExecutor_Proxy(SCR_TaskExecutor executor)
	{
		if (!m_TaskData || !executor)
			return;
		
		if (!m_TaskData.m_aOwnerExecutors)
			m_TaskData.m_aOwnerExecutors = new array<ref SCR_TaskExecutor>();
		
		SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(m_TaskData.m_aOwnerExecutors, executor);
		if (match)
			return;
		
		m_TaskData.m_aOwnerExecutors.Insert(executor);
			
		if (s_OnOwnerExecutorAdded)
			s_OnOwnerExecutorAdded.Invoke(this, executor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddOwnerExecutorPlayer(SCR_TaskExecutorPlayer executor)
	{
		AddOwnerExecutor_Proxy(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddOwnerExecutorEntity(SCR_TaskExecutorEntity executor)
	{
		AddOwnerExecutor_Proxy(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddOwnerExecutorGroup(SCR_TaskExecutorGroup executor)
	{
		AddOwnerExecutor_Proxy(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to remove an owner executor from this task
	//! \param[in] executor
	//! \return
	bool RemoveOwnerExecutor(SCR_TaskExecutor executor)
	{
		if (!m_TaskData || !executor)
			return false;
		
		if (!m_TaskData.m_aOwnerExecutors)
			return false;
		
		SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(m_TaskData.m_aOwnerExecutors, executor);
		if (!match)
			return false;
		
		m_TaskData.m_aOwnerExecutors.RemoveItem(match);
		
		if (s_OnOwnerExecutorRemoved)
			s_OnOwnerExecutorRemoved.Invoke(this, match);
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);	
		if (executorPlayer)
		{
			Rpc(Rpc_RemoveOwnerExecutorPlayer, executorPlayer);
			return true;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{
			Rpc(Rpc_RemoveOwnerExecutorEntity, executorEntity);
			return true;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
		{
			Rpc(Rpc_RemoveOwnerExecutorGroup, executorGroup);
			return true;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveOwnerExecutor_Proxy(SCR_TaskExecutor executor)
	{
		if (!m_TaskData || !executor)
			return;
		
		if (!m_TaskData.m_aOwnerExecutors)
			return;
		
		SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(m_TaskData.m_aOwnerExecutors, executor);
		if (!match)
			return;
		
		m_TaskData.m_aOwnerExecutors.RemoveItem(match);
			
		if (s_OnOwnerExecutorRemoved)
			s_OnOwnerExecutorRemoved.Invoke(this, match);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveOwnerExecutorPlayer(SCR_TaskExecutorPlayer executor)
	{
		RemoveOwnerExecutor_Proxy(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveOwnerExecutorEntity(SCR_TaskExecutorEntity executor)
	{
		RemoveOwnerExecutor_Proxy(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveOwnerExecutorGroup(SCR_TaskExecutorGroup executor)
	{
		RemoveOwnerExecutor_Proxy(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns position of task
	//! \return
	vector GetTaskPosition()
	{
		if (m_TaskData)
		{
			if (m_TaskData.m_vPosition != GetOrigin())
				m_TaskData.m_vPosition = GetOrigin();
			
			return m_TaskData.m_vPosition;
		}
		
		return GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets position of task
	//! \param[in] position
	void SetTaskPosition(vector position)
	{
		Rpc_SetTaskPosition(position);
		Rpc(Rpc_SetTaskPosition, position);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskPosition(vector position)
	{
		if (m_TaskData)
			m_TaskData.m_vPosition = position;
		
		SetOrigin(position);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_TaskUIComponent GetUIComponent()
	{
		return m_UIComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskStateInvoker GetOnTaskStateChanged()
	{
		if (!s_OnTaskStateChanged)
			s_OnTaskStateChanged = new SCR_TaskStateInvoker();
		
		return s_OnTaskStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskOwnershipInvoker GetOnTaskOwnershipChanged()
	{
		if (!s_OnTaskOwnershipChanged)
			s_OnTaskOwnershipChanged = new SCR_TaskOwnershipInvoker();
		
		return s_OnTaskOwnershipChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskVisibilityInvoker GetOnTaskVisibilityChanged()
	{
		if (!s_OnTaskVisibilityChanged)
			s_OnTaskVisibilityChanged = new SCR_TaskVisibilityInvoker();
		
		return s_OnTaskVisibilityChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskUIVisibilityInvoker GetOnTaskUIVisibilityChanged()
	{
		if (!s_OnTaskUIVisibilityChanged)
			s_OnTaskUIVisibilityChanged = new SCR_TaskUIVisibilityInvoker();
		
		return s_OnTaskUIVisibilityChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskExecutorRequesterInvoker GetOnTaskAssigneeAdded()
	{
		if (!s_OnTaskAssigneeAdded)
			s_OnTaskAssigneeAdded = new SCR_TaskExecutorRequesterInvoker();
		
		return s_OnTaskAssigneeAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskExecutorRequesterInvoker GetOnTaskAssigneeRemoved()
	{
		if (!s_OnTaskAssigneeRemoved)
			s_OnTaskAssigneeRemoved = new SCR_TaskExecutorRequesterInvoker();
		
		return s_OnTaskAssigneeRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskFactionInvoker GetOnOwnerFactionAdded()
	{
		if (!s_OnOwnerFactionAdded)
			s_OnOwnerFactionAdded = new SCR_TaskFactionInvoker();
		
		return s_OnOwnerFactionAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskFactionInvoker GetOnOwnerFactionRemoved()
	{
		if (!s_OnOwnerFactionRemoved)
			s_OnOwnerFactionRemoved = new SCR_TaskFactionInvoker();
		
		return s_OnOwnerFactionRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskGroupInvoker GetOnOwnerGroupAdded()
	{
		if (!s_OnOwnerGroupAdded)
			s_OnOwnerGroupAdded = new SCR_TaskGroupInvoker();
		
		return s_OnOwnerGroupAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskGroupInvoker GetOnOwnerGroupRemoved()
	{
		if (!s_OnOwnerGroupRemoved)
			s_OnOwnerGroupRemoved = new SCR_TaskGroupInvoker();
		
		return s_OnOwnerGroupRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskExecutorInvoker GetOnOwnerExecutorAdded()
	{
		if (!s_OnOwnerExecutorAdded)
			s_OnOwnerExecutorAdded = new SCR_TaskExecutorInvoker();
		
		return s_OnOwnerExecutorAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_TaskExecutorInvoker GetOnOwnerExecutorRemoved()
	{
		if (!s_OnOwnerExecutorRemoved)
			s_OnOwnerExecutorRemoved = new SCR_TaskExecutorInvoker();
		
		return s_OnOwnerExecutorRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	ScriptInvokerVoid GetOnDisplayDataChanged()
	{
		if (!m_OnDisplayDataChanged)
			m_OnDisplayDataChanged = new ScriptInvokerVoid();
		
		return m_OnDisplayDataChanged;
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		vector textPosition = GetOrigin() + vector.Up * 0.5;
		m_DebugText = DebugTextWorldSpace.Create(GetWorld(), "Task [" + m_sTaskID + "]", DebugTextFlags.CENTER, textPosition[0], textPosition[1], textPosition[2], 15.0, Color.CYAN, Color.BLUE);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteBool(m_TaskData != null);
		if (!m_TaskData)
			return true;
		
		writer.WriteString(m_TaskData.m_sID);
		WriteUIInfo(writer, m_TaskData.m_UIInfo);
		writer.WriteVector(m_TaskData.m_vPosition);
		writer.WriteInt(m_TaskData.m_iAuthorId);
		
		writer.WriteInt(m_TaskData.m_eState);
		writer.WriteInt(m_TaskData.m_eOwnership);
		writer.WriteInt(m_TaskData.m_eVisibility);
		writer.WriteInt(m_TaskData.m_eUIVisibility);
		
		int assigneeCount = 0;
		if (m_TaskData.m_aAssignees)
			assigneeCount = m_TaskData.m_aAssignees.Count();
		
		writer.WriteInt(assigneeCount);
		if (assigneeCount > 0)
		{
			foreach (SCR_TaskExecutor assignee : m_TaskData.m_aAssignees)
			{
				WriteExecutor(writer, assignee);
			}
		}
		
		int ownerFactionCount = 0;
		if (m_TaskData.m_aOwnerFactionKeys)
			ownerFactionCount = m_TaskData.m_aOwnerFactionKeys.Count();
		
		writer.WriteInt(ownerFactionCount);
		if (ownerFactionCount > 0)
		{
			foreach (string factionKey : m_TaskData.m_aOwnerFactionKeys)
			{
				writer.WriteString(factionKey);
			}
		}
		
		int ownerGroupCount = 0;
		if (m_TaskData.m_aOwnerGroupIDs)
			ownerGroupCount = m_TaskData.m_aOwnerGroupIDs.Count();
		
		writer.WriteInt(ownerGroupCount);
		if (ownerGroupCount > 0)
		{
			foreach (int groupID : m_TaskData.m_aOwnerGroupIDs)
			{
				writer.WriteInt(groupID);
			}
		}
		
		int ownerExecutorCount = 0;
		if (m_TaskData.m_aOwnerExecutors)
			ownerExecutorCount = m_TaskData.m_aOwnerExecutors.Count();
		
		writer.WriteInt(ownerExecutorCount);
		if (ownerExecutorCount > 0)
		{
			foreach (SCR_TaskExecutor executor : m_TaskData.m_aOwnerExecutors)
			{
				WriteExecutor(writer, executor);
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void WriteUIInfo(ScriptBitWriter writer, SCR_TaskUIInfo info)
	{
		array<LocalizedString> nameParams;
		array<LocalizedString> descParams;
		writer.WriteString(info.GetUnformattedName(nameParams));
		writer.WriteString(info.GetUnformattedDescription(descParams));
		writer.WriteString(info.GetImageSetPath());
		writer.WriteString(info.GetIconSetName());
		
		if (!nameParams || nameParams.IsEmpty())
		{
			writer.WriteInt(0);
		}
		else
		{
			writer.WriteInt(nameParams.Count());
			foreach (string param : nameParams)
			{
				writer.WriteString(param);
			}
		}
		
		if (!descParams || descParams.IsEmpty())
		{
			writer.WriteInt(0);
		}
		else
		{
			writer.WriteInt(descParams.Count());
			foreach (string param : descParams)
			{
				writer.WriteString(param);
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void WriteExecutor(ScriptBitWriter writer, SCR_TaskExecutor executor)
	{
		writer.WriteString(executor.Type().ToString());
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
		if (executorPlayer)
		{
			writer.WriteInt(executorPlayer.GetPlayerID());
			return;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{	
			IEntity entity = executorEntity.GetEntity();
			if (!entity)
				return;
				
			RplComponent rpl = RplComponent.Cast(entity.FindComponent(RplComponent));
			if (!rpl)
				return;
			
			writer.WriteRplId(rpl.Id());
			return;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
		{
			writer.WriteInt(executorGroup.GetGroupID());
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		bool hasTaskData;
		reader.ReadBool(hasTaskData);
		if (!hasTaskData)
		{
			m_TaskData = null;
			return true;
		}

		if (!m_TaskData)
			m_TaskData = new SCR_TaskData();
		
		reader.ReadString(m_TaskData.m_sID);
		ReadUIInfo(reader, m_TaskData.m_UIInfo);
		reader.ReadVector(m_TaskData.m_vPosition);
		reader.ReadInt(m_TaskData.m_iAuthorId);
		
		reader.ReadInt(m_TaskData.m_eState);
		reader.ReadInt(m_TaskData.m_eOwnership);
		reader.ReadInt(m_TaskData.m_eVisibility);
		reader.ReadInt(m_TaskData.m_eUIVisibility);

		int assigneeCount;
		reader.ReadInt(assigneeCount);	
		if (assigneeCount > 0)
		{
			m_TaskData.m_aAssignees = {};
			
			for (int i = 0; i < assigneeCount; i++)
			{
				SCR_TaskExecutor assignee;
				ReadExecutor(reader, assignee);
				m_TaskData.m_aAssignees.Insert(assignee);
			}
		}
		
		int ownerFactionCount;
		reader.ReadInt(ownerFactionCount);
		if (ownerFactionCount > 0)
		{
			m_TaskData.m_aOwnerFactionKeys = {};
			
			for (int i = 0; i < ownerFactionCount; i++)
			{
				string factionKey;
				reader.ReadString(factionKey);
				m_TaskData.m_aOwnerFactionKeys.Insert(factionKey);
			}
		}
		
		int ownerGroupCount;
		reader.ReadInt(ownerGroupCount);
		if (ownerGroupCount > 0)
		{
			m_TaskData.m_aOwnerGroupIDs = {};
			
			for (int i = 0; i < ownerGroupCount; i++)
			{
				int groupID;
				reader.ReadInt(groupID);
				m_TaskData.m_aOwnerGroupIDs.Insert(groupID);
			}
		}
		
		int ownerExecutorCount;
		reader.ReadInt(ownerExecutorCount);
		if (ownerExecutorCount > 0)
		{
			m_TaskData.m_aOwnerExecutors = {};
			
			for (int i = 0; i < ownerExecutorCount; i++)
			{
				SCR_TaskExecutor executor;
				ReadExecutor(reader, executor);
				m_TaskData.m_aOwnerExecutors.Insert(executor);
			}
		}
		
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ReadUIInfo(ScriptBitReader reader, out SCR_TaskUIInfo info)
	{
		string name, desc, iconPath, iconSetName;
		reader.ReadString(name);
		reader.ReadString(desc);
		reader.ReadString(iconPath);
		reader.ReadString(iconSetName);
		
		array<LocalizedString> nameParams;
		array<LocalizedString> descParams;
		
		int nameParamCount;
		reader.ReadInt(nameParamCount);
		if (nameParamCount > 0)
		{
			nameParams = {};
			for (int i = 0; i < nameParamCount; i++)
			{
				string param;
				reader.ReadString(param);
				nameParams.Insert(param);
			}
		}
		
		int descParamCount;
		reader.ReadInt(descParamCount);
		if (descParamCount > 0)
		{
			descParams = {};
			for (int i = 0; i < descParamCount; i++)
			{
				string param;
				reader.ReadString(param);
				descParams.Insert(param);
			}
		}
		
		info = SCR_TaskUIInfo.CreateInfo(name, nameParams, desc, descParams, iconPath, iconSetName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ReadExecutor(ScriptBitReader reader, out SCR_TaskExecutor executor)
	{
		string type;
		reader.ReadString(type);
		
		if (type.ToType() == SCR_TaskExecutorPlayer)
		{
			int playerID;
			reader.ReadInt(playerID);
				
			executor = SCR_TaskExecutor.FromPlayerID(playerID);
			return;
		}
		
		if (type.ToType() == SCR_TaskExecutorEntity)
		{
			RplId entityRplID;
			reader.ReadRplId(entityRplID);
				
			if (!entityRplID.IsValid())
				return;
				
			RplComponent rpl = RplComponent.Cast(Replication.FindItem(entityRplID));
			if (!rpl)
				return;
				
			executor = SCR_TaskExecutor.FromEntity(rpl.GetEntity());
			return;
		}
		
		if (type.ToType() == SCR_TaskExecutorGroup)
		{
			int groupID;
			reader.ReadInt(groupID);
				
			executor = SCR_TaskExecutor.FromGroup(groupID);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Go through assignee list and remove assignees who are not allowed to be assigned to this task
	protected void RemoveUnauthorizedAssignees()
	{
		array<ref SCR_TaskExecutor> assignees = GetTaskAssignees();
		foreach (SCR_TaskExecutor assignee : assignees)
		{
			if (!m_TaskSystem.CanTaskBeAssignedTo(this, assignee))
				RemoveTaskAssignee(assignee);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! If faction changed for an assignee, go through assignee list and remove assignees who are no longer allowed to be assigned to this task
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		RemoveUnauthorizedAssignees();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionChanged(int playerId, SCR_PlayerFactionAffiliationComponent playerFactionAffiliationComponent, Faction faction)
	{	
		RemoveUnauthorizedAssignees();
	}
	
	//------------------------------------------------------------------------------------------------
	//! If group changed for an assignee, go through assignee list and remove assignees who are no longer allowed to be assigned to this task
	protected void OnGroupChanged(int groupID)
	{
		RemoveUnauthorizedAssignees();
	}
	
	//------------------------------------------------------------------------------------------------
	//! When group is deleted, check if group matches group in task data and reset values
	protected void OnGroupRemoved(SCR_AIGroup group)
	{
		int groupID = group.GetGroupID();
		SCR_TaskExecutor executor = SCR_TaskExecutor.FromGroup(groupID);
		
		if (IsTaskAssignedTo(executor))
			RemoveTaskAssignee(executor);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invoke m_OnDisplayDataChanged to update UI when player joins group
	protected void OnPlayerAddedToGroup(SCR_AIGroup group, int playerID)
	{
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invoke m_OnDisplayDataChanged to update UI when player leaves group
	protected void OnPlayerRemovedFromGroup(SCR_AIGroup group, int playerID)
	{
		int groupID = group.GetGroupID();
		SCR_TaskExecutor executor = SCR_TaskExecutor.FromGroup(groupID);
		
		if (IsTaskAssignedTo(executor) && group.GetTotalPlayerCount() <= 0)
			RemoveTaskAssignee(executor);
		
		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitializeData()
	{
		if (!m_TaskData)
			m_TaskData = new SCR_TaskData();
		
		m_TaskData.m_sID = m_sTaskID;
		
		if (m_TaskUIInfo)
			m_TaskData.m_UIInfo = m_TaskUIInfo;
		else
			m_TaskData.m_UIInfo = new SCR_TaskUIInfo();
		
		m_TaskData.m_vPosition = GetOrigin();
		
		m_TaskData.m_aAssignees = {};
		
		m_TaskData.m_aOwnerFactionKeys = {};
		if (m_aOwnerFactionKeys && !m_aOwnerFactionKeys.IsEmpty())
			m_TaskData.m_aOwnerFactionKeys.Copy(m_aOwnerFactionKeys);
		
		m_TaskData.m_aOwnerGroupIDs = {};
		if (m_aOwnerGroupIDs && !m_aOwnerGroupIDs.IsEmpty())
			m_TaskData.m_aOwnerGroupIDs.Copy(m_aOwnerGroupIDs);
		
		m_TaskData.m_aOwnerExecutors = {};
		if (m_aOwnerExecutors && !m_aOwnerExecutors.IsEmpty())
			SCR_ArrayHelperRefT<ref SCR_TaskExecutor>.CopyReferencesFromTo(m_aOwnerExecutors, m_TaskData.m_aOwnerExecutors);
		
		m_TaskData.m_aAssignees = {};
		if (m_aAssignees && !m_aAssignees.IsEmpty())
			SCR_ArrayHelperRefT<ref SCR_TaskExecutor>.CopyReferencesFromTo(m_aAssignees, m_TaskData.m_aAssignees);
		
		m_TaskData.m_eState = m_eTaskState;
		m_TaskData.m_eOwnership = m_eTaskOwnership;
		m_TaskData.m_eVisibility = m_eTaskVisibility;
		m_TaskData.m_eUIVisibility = m_eTaskUIVisibility;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToTaskSystem()
	{
		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem)
			return;
		
		m_TaskSystem.RegisterTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromTaskSystem()
	{
		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem)
			return;
		
		m_TaskSystem.UnregisterTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		InitializeData();
		ConnectToTaskSystem();
		
		m_UIComponent = SCR_TaskUIComponent.Cast(FindComponent(SCR_TaskUIComponent));
		
		m_EditableEntityComponent = SCR_EditableEntityComponent.Cast(FindComponent(SCR_EditableEntityComponent));
		if (m_EditableEntityComponent)
		{
			m_EditableEntityComponent.GetInfo(this).SetName(m_TaskData.m_UIInfo.GetName());
			m_EditableEntityComponent.GetInfo(this).SetDescription(m_TaskData.m_UIInfo.GetName());
		}
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_Task(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_Task()
	{		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			groupsManager.GetOnPlayableGroupRemoved().Remove(OnGroupRemoved);
		
		DisconnectFromTaskSystem();
	}
}
