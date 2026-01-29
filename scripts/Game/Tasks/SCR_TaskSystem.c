void TaskInvokerDelegate(SCR_Task task);
typedef func TaskInvokerDelegate;
typedef ScriptInvokerBase<TaskInvokerDelegate> SCR_TaskInvoker;

class SCR_TaskSystem : GameSystem
{	
	protected RplComponent m_Rpl;
	
	protected static ref SCR_TaskInvoker s_OnTaskAdded;
	protected static ref SCR_TaskInvoker s_OnTaskRemoved;
	
	protected static ref array<SCR_Task> s_aTasks = {};
	
	protected const ResourceName BASE_TASK_RESOURCE = "{1D0F815858EE24AD}Prefabs/Tasks/BaseTask.et";
	protected const ResourceName EXTENDED_TASK_RESOURCE = "{CF6A2A13DF7B1018}Prefabs/Tasks/ExtendedTask.et";
	
	// Diag
	private bool diag_taskListMenu;
	private int diag_taskListMenuSelection;
	private string diag_taskListMenuExecutorName;
	private ref SCR_TaskExecutor diag_selectedExecutor;
	
	private const int DIAG_MAX_NAME_LENGTH = 30;
	private const int DIAG_MAX_NAME_LENGTH_LIST = 20;
	private const int DIAG_MAX_DESCRIPTION_LENGTH = 23;
	
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo
			.SetAbstract(false)
			.SetLocation(ESystemLocation.Both)
			.AddPoint(ESystemPoint.Frame);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_TASKS, "Task System", "Systems");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_TASKS_LIST, "", "Task List", "Task System");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_TASKS_CREATE, "", "Create Task", "Task System");
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns instance of SCR_TaskSystem
	//! \return
	static SCR_TaskSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return SCR_TaskSystem.Cast(world.FindSystem(SCR_TaskSystem));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns OnTaskAdded invoker
	//! \return
	static SCR_TaskInvoker GetOnTaskAdded()
	{
		if (!s_OnTaskAdded)
			s_OnTaskAdded = new SCR_TaskInvoker();
		
		return s_OnTaskAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns OnTaskRemoved invoker
	//! \return
	static SCR_TaskInvoker GetOnTaskRemoved()
	{
		if (!s_OnTaskRemoved)
			s_OnTaskRemoved = new SCR_TaskInvoker();
		
		return s_OnTaskRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outputs array of assignee entities for task
	//! \param[in] task
	//! \param[out] assignees
	void GetAssigneesForTask(notnull SCR_Task task, out array<ref SCR_TaskExecutor> assignees)
	{
		assignees = task.GetTaskAssignees();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if task is visible in task list
	//! \param[in] task
	//! \return
	bool IsTaskVisibleInTaskList(notnull SCR_Task task)
	{
		return task.GetTaskUIVisibility() == SCR_ETaskUIVisibility.ALL || task.GetTaskUIVisibility() == SCR_ETaskUIVisibility.LIST_ONLY;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if task is visible on map
	//! \param[in] task
	//! \return
	bool IsTaskVisibleOnMap(notnull SCR_Task task)
	{
		return task.GetTaskUIVisibility() == SCR_ETaskUIVisibility.ALL || task.GetTaskUIVisibility() == SCR_ETaskUIVisibility.MAP_ONLY;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if task is visible to executor
	//! \param[in] task
	//! \param[in] executor
	//! \return
	bool IsTaskVisibleFor(notnull SCR_Task task, notnull SCR_TaskExecutor executor)
	{
		SCR_ETaskVisibility visibility = task.GetTaskVisibility();
		
		if (visibility == SCR_ETaskVisibility.NONE)
		{
			return false;
		}	
		else if (visibility == SCR_ETaskVisibility.EXECUTOR)
		{
			array<ref SCR_TaskExecutor> executors = task.GetOwnerExecutors();
			if (!executors)
				return false;
			
			SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(executors, executor, true);		
			if (!match)
				return false;
		}				
		else if (visibility == SCR_ETaskVisibility.ASSIGNEES)
		{
			array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();
			if (!assignees)
				return false;
			
			SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(assignees, executor, true);		
			if (!match)
				return false;
		}		
		else if (visibility == SCR_ETaskVisibility.GROUP)
		{			
			int groupID = executor.GetGroupID();
			if (groupID <= -1)
				return false;
			
			array<int> groupIDs = task.GetOwnerGroupIDs();
			if (!groupIDs)
				return false;
			
			if (!groupIDs.Contains(groupID))
				return false;
		}		
		else if (visibility == SCR_ETaskVisibility.FACTION)
		{
			FactionKey factionKey = executor.GetFactionKey();
			if (factionKey == FactionKey.Empty)
				return false;
			
			array<string> factionKeys = task.GetOwnerFactionKeys();
			if (!factionKeys)
				return false;
			
			if (!factionKeys.Contains(factionKey))
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if task is assignable to executor; Uses task ownership for comparison
	//! \param[in] task
	//! \param[in] executor
	//! \return
	bool CanTaskBeAssignedTo(notnull SCR_Task task, notnull SCR_TaskExecutor executor)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && !extendedTask.CanBeAssigned())
			return false;
		
		SCR_ETaskState state = task.GetTaskState();
		if (state == SCR_ETaskState.COMPLETED || state == SCR_ETaskState.FAILED || state == SCR_ETaskState.CANCELLED)
			return false;
		
		SCR_ETaskOwnership ownership = task.GetTaskOwnership();
		
		if (ownership == SCR_ETaskOwnership.NONE)
		{
			return false;
		}		
		else if (ownership == SCR_ETaskOwnership.EXECUTOR)
		{
			array<ref SCR_TaskExecutor> executors = task.GetOwnerExecutors();
			if (!executors)
				return false;
			
			SCR_TaskExecutor match = SCR_TaskExecutor.FindMatchingTaskExecutor(executors, executor);		
			if (!match)
			{
				array<int> groupIDs = {};			
				foreach (SCR_TaskExecutor e : executors)
				{
					SCR_TaskExecutorGroup eGroup = SCR_TaskExecutorGroup.Cast(e);
					if (eGroup)
						groupIDs.Insert(eGroup.GetGroupID());
				}
				
				SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
				if (!executorPlayer)
					return false;
				
				if (!IsGroupLeader(groupIDs, executorPlayer.GetPlayerID()))
					return false;
			}
		}		
		else if (ownership == SCR_ETaskOwnership.GROUP)
		{
			int groupID = executor.GetGroupID();
			if (groupID <= -1)
				return false;
			
			array<int> groupIDs = task.GetOwnerGroupIDs();
			if (!groupIDs)
				return false;
			
			if (!groupIDs.Contains(groupID))
				return false;
		}	
		else if (ownership == SCR_ETaskOwnership.FACTION)
		{
			FactionKey factionKey = executor.GetFactionKey();
			if (factionKey == FactionKey.Empty)
				return false;
			
			array<string> factionKeys = task.GetOwnerFactionKeys();
			if (!factionKeys)
				return false;
			
			if (!factionKeys.Contains(factionKey))
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if specified player is leader of any group in the array
	//! \param[in] groupIDs
	//! \param[in] playerID
	bool IsGroupLeader(notnull array<int> groupIDs, int playerID)
	{
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManagerComponent)
			return false;
		
		if (!groupIDs || groupIDs.IsEmpty())
			return false;
		
		SCR_AIGroup group;
		foreach (int groupID : groupIDs)
		{		
			group = groupsManagerComponent.FindGroup(groupID);
			if (!group)
				continue;
			
			int groupLeaderID = group.GetLeaderID();
			if (groupLeaderID == playerID)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds and returns group which specified player is the leader of
	//! \param[in] groupIDs
	//! \param[in] leaderID
	//! \return
	int FindGroupByLeader(notnull array<int> groupIDs, int leaderID)
	{
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManagerComponent)
			return -1;
		
		if (!groupIDs || groupIDs.IsEmpty())
			return -1;
		
		foreach (int groupID : groupIDs)
		{		
			SCR_AIGroup group = groupsManagerComponent.FindGroup(groupID);
			if (!group)
				continue;
			
			if (group.IsPlayerLeader(leaderID))
				return groupID;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] outTasks
	//! \return
	int GetTasks(out notnull array<SCR_Task> outTasks)
	{
		outTasks.Copy(s_aTasks);
		return outTasks.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns task assigned to provided executor; Returns null if executor is not assigned to any task
	//! \param[in] executor
	//! \return
	SCR_Task GetTaskAssignedTo(notnull SCR_TaskExecutor executor)
	{
		foreach (SCR_Task task : s_aTasks)
		{
			if (!task)
				continue;
			
			if (task.IsTaskAssignedTo(executor))
				return task;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outputs array of tasks that match the state; Tasks can be filtered further through faction and group
	//! \param[out] outTasks
	//! \param[in] state
	//! \param[in] filterFaction
	//! \param[in] filterGroup
	void GetTasksByState(out notnull array<SCR_Task> outTasks, SCR_ETaskState state, FactionKey filterFaction = FactionKey.Empty, int filterGroup = -1)
	{
		outTasks = {};

		array<string> factionKeys;
		array<int> groupIDs;
		
		foreach (SCR_Task task : s_aTasks)
		{	
			if (!task)
				continue;
			
			if ((state & task.GetTaskState()) == 0)
				continue;
			
			if (filterFaction != FactionKey.Empty)
			{
				factionKeys = task.GetOwnerFactionKeys();
				if (!factionKeys || factionKeys.IsEmpty())
					continue;
				
				if (!factionKeys.Contains(filterFaction))
					continue;
			}
			
			if (filterGroup > -1)
			{
				groupIDs = task.GetOwnerGroupIDs();
				if (!groupIDs || groupIDs.IsEmpty())
					continue;
				
				if (!groupIDs.Contains(filterGroup))
					continue;
			}
			
			outTasks.Insert(task);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outputs array of tasks that match the state; Tasks can be filtered further through faction, group and class
	//! \param[out] outTasks
	//! \param[in] state
	//! \param[in] filterFaction
	//! \param[in] filterGroup
	//! \param[in] filterClass
	//! \param[in] allowInherited
	int GetTasksByStateFiltered(out notnull array<SCR_Task> outTasks, SCR_ETaskState state, FactionKey filterFaction = FactionKey.Empty, int filterGroup = -1, typename filterClass = typename.Empty, bool allowInherited = false)
	{
		SCR_TaskSystem.GetInstance().GetTasksByState(outTasks, state, filterFaction, filterGroup);

		if (filterClass == filterClass.Empty)
			return outTasks.Count();

		for (int i = outTasks.Count() - 1; i >= 0; i--)
		{
			if (allowInherited)
			{
				if (!outTasks[i].IsInherited(filterClass))
					outTasks.Remove(i);
			}
			else
			{
				if (outTasks[i].Type() != filterClass)
					outTasks.Remove(i);
			}
		}

		return outTasks.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! 
	//! \param[out] outTasks
	//! \param[in] executor
	//! \return
	int GetTasksVisibleFor(out notnull array<SCR_Task> outTasks, notnull SCR_TaskExecutor executor)
	{
		outTasks = {};
		
		array<SCR_Task> tasks = {};
		GetTasks(tasks);
		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;
			
			if (!IsTaskVisibleFor(task, executor))
				continue;
			
			outTasks.Insert(task);
		}
		
		return outTasks.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers task into task system
	//! \param[in] task
	void RegisterTask(notnull SCR_Task task)
	{	
		s_aTasks.Insert(task);
		
		if (s_OnTaskAdded)
			s_OnTaskAdded.Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregisters task from task system
	//! \param[in] task
	void UnregisterTask(notnull SCR_Task task)
	{
		s_aTasks.RemoveItem(task);
		
		if (s_OnTaskRemoved)
			s_OnTaskRemoved.Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to assign task to executor;
	//! If force is true, executor will be automatically unassigned from other tasks and assigned to this one
	//! \param[in] task
	//! \param[in] executor
	//! \param[in] force
	//! \param[in] requesterID
	void AssignTask(notnull SCR_Task task, notnull SCR_TaskExecutor executor, bool force = false, int requesterID = 0)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		if (task.IsTaskAssignedTo(executor))
			return; // Already assigned

		SCR_TaskExecutor assignee;
		if (executor)
			assignee = executor;
		
		if (task.GetTaskOwnership() == SCR_ETaskOwnership.EXECUTOR)
		{
			array<ref SCR_TaskExecutor> ownerExecutors = task.GetOwnerExecutors();
			if (ownerExecutors && !ownerExecutors.IsEmpty())
			{
				array<int> groupIDs = {};
				foreach (SCR_TaskExecutor ownerExecutor : ownerExecutors)
				{
					SCR_TaskExecutorGroup ownerExecutorGroup = SCR_TaskExecutorGroup.Cast(ownerExecutor);
					if (ownerExecutorGroup)
						groupIDs.Insert(ownerExecutorGroup.GetGroupID());
				}
				
				SCR_TaskExecutorPlayer assigneePlayer = SCR_TaskExecutorPlayer.Cast(assignee);
				if (assigneePlayer)
				{	
					int groupID = FindGroupByLeader(groupIDs, assigneePlayer.GetPlayerID());
					if (groupID > -1)
						assignee = TaskExecutorFromGroup(groupID);
				}
			}
		}
		
		string name, suffix;
		if (assignee)
		{
			SCR_TaskExecutorPlayer assigneePlayer = SCR_TaskExecutorPlayer.Cast(assignee);
			if (assigneePlayer)
			{	
				PlayerManager playerManager = GetGame().GetPlayerManager();
				if (!playerManager)
					return;
				
				int playerID = assigneePlayer.GetPlayerID();
				name = playerManager.GetPlayerName(playerID);
				suffix = playerID.ToString();
			}
			
			SCR_TaskExecutorEntity assigneeEntity = SCR_TaskExecutorEntity.Cast(assignee);
			if (assigneeEntity)
			{
				IEntity entity = assigneeEntity.GetEntity();
				if (!entity)
					return;
				
				name = entity.GetName();		
				suffix = entity.Type().ToString();
			}
			
			SCR_TaskExecutorGroup assigneeGroup = SCR_TaskExecutorGroup.Cast(assignee);
			if (assigneeGroup)
			{
				SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
				if (!groupsManagerComponent)
					return;
			
				SCR_AIGroup group = groupsManagerComponent.FindGroup(assigneeGroup.GetGroupID());
				if (!group)
					return;
					
				name = "Group";
				suffix = group.GetGroupID().ToString();
			}
		}
		
		if (!CanTaskBeAssignedTo(task, assignee))
		{
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " cannot be assigned to " + name + "(" + suffix + ")", LogLevel.WARNING);
			return;
		}
		
		foreach (SCR_Task t : s_aTasks)
		{
			if (!t)
				continue;
			
			if (t.IsTaskAssignedTo(assignee))
			{
				if (t == task)
				{
					Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " is already assigned to " + name + "(" + suffix + ")", LogLevel.WARNING);
					return;
				}
				
				if (!force)
				{
					Print("SCR_TaskSystem: " + name + "(" + suffix + ") can only have 1 task assigned to it at a time!", LogLevel.WARNING);
					return;
				}
				
				t.RemoveTaskAssignee(assignee, true, requesterID);
			}
		}
		
		if (!task.AddTaskAssignee(assignee, true, requesterID))
		{
			Print("SCR_TaskSystem: Could not assign " + name + "(" + suffix + ") to task with ID: " + task.GetTaskID(), LogLevel.ERROR);
			return;
		}
		
		if (task.GetTaskState() != SCR_ETaskState.COMPLETED && task.GetTaskState() != SCR_ETaskState.FAILED && task.GetTaskState() != SCR_ETaskState.CANCELLED && task.GetTaskAssignees().Count() == 1)
			task.SetTaskState(SCR_ETaskState.ASSIGNED);
		
		if (force)
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " force-assigned to " + name + "(" + suffix + ")", LogLevel.DEBUG);
		else
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " assigned to " + name + "(" + suffix + ")", LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to unassign task from executor
	//! \param[in] task
	//! \param[in] executor
	//! \param[in] requesterID
	void UnassignTask(notnull SCR_Task task, notnull SCR_TaskExecutor executor, int requesterID = 0)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_TaskExecutor assignee;
		if (executor)
			assignee = executor;
		
		if (task.GetTaskOwnership() == SCR_ETaskOwnership.EXECUTOR)
		{
			array<ref SCR_TaskExecutor> ownerExecutors = task.GetOwnerExecutors();
			if (ownerExecutors && !ownerExecutors.IsEmpty())
			{
				array<int> groupIDs = {};
				foreach (SCR_TaskExecutor ownerExecutor : ownerExecutors)
				{
					SCR_TaskExecutorGroup ownerExecutorGroup = SCR_TaskExecutorGroup.Cast(ownerExecutor);
					if (ownerExecutorGroup)
						groupIDs.Insert(ownerExecutorGroup.GetGroupID());
				}
				
				SCR_TaskExecutorPlayer assigneePlayer = SCR_TaskExecutorPlayer.Cast(assignee);
				if (assigneePlayer)
				{	
					int groupID = FindGroupByLeader(groupIDs, assigneePlayer.GetPlayerID());
					if (groupID > -1)
						assignee = TaskExecutorFromGroup(groupID);
				}
			}
		}
		
		string name, suffix;
		if (assignee)
		{
			SCR_TaskExecutorPlayer assigneePlayer = SCR_TaskExecutorPlayer.Cast(assignee);
			if (assigneePlayer)
			{	
				PlayerManager playerManager = GetGame().GetPlayerManager();
				if (!playerManager)
					return;
				
				int playerID = assigneePlayer.GetPlayerID();
				name = playerManager.GetPlayerName(playerID);
				suffix = playerID.ToString();
			}
			
			SCR_TaskExecutorEntity assigneeEntity = SCR_TaskExecutorEntity.Cast(assignee);
			if (assigneeEntity)
			{
				IEntity entity = assigneeEntity.GetEntity();
				if (!entity)
					return;
				
				name = entity.GetName();		
				suffix = entity.Type().ToString();
			}
			
			SCR_TaskExecutorGroup assigneeGroup = SCR_TaskExecutorGroup.Cast(assignee);
			if (assigneeGroup)
			{
				SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
				if (!groupsManagerComponent)
					return;
			
				SCR_AIGroup group = groupsManagerComponent.FindGroup(assigneeGroup.GetGroupID());
				if (!group)
					return;
					
				name = "Group";
				suffix = group.GetGroupID().ToString();
			}
		}
		
		if (!task.RemoveTaskAssignee(assignee, true, requesterID))
		{
			Print("SCR_TaskSystem: Could not unassign " + name + "(" + suffix + ") from task with ID: " + task.GetTaskID(), LogLevel.ERROR);
			return;
		}
		
		if (task.GetTaskState() != SCR_ETaskState.COMPLETED && task.GetTaskState() != SCR_ETaskState.FAILED && task.GetTaskAssignees().IsEmpty())
			task.SetTaskState(SCR_ETaskState.CREATED);
		
		Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " unassigned from " + name + "(" + suffix + ")", LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if progress for task can be shown
	//! \param[in] task
	//! \return
	bool CanProgressBeShownForTask(notnull SCR_Task task)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
			return false;
		
		return extendedTask.CanTaskProgressBeShown();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggles progress bar for task
	//! \param[in] task
	//! \param[in] enable
	//! \return
	void ShowProgressForTask(notnull SCR_Task task, bool enable)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
		{
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " is not an extended task!", LogLevel.WARNING);
			return;
		}
		
		extendedTask.ShowTaskProgress(enable);
		
		if (enable)
			Print("SCR_TaskSystem: Enabled progress bar for task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
		else
			Print("SCR_TaskSystem: Disabled progress bar for task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns progress of task as float; Value ranges from 0-100
	//! \param[in] task
	//! \return
	float GetTaskProgress(notnull SCR_Task task)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
			return -1;
		
		return extendedTask.GetTaskProgress();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets progress of task as float; Value ranges from 0-100
	//! \param[in] task
	//! \param[in] percentage
	//! \return
	void SetTaskProgress(notnull SCR_Task task, float percentage)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
		{
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " is not an extended task!", LogLevel.WARNING);
			return;
		}
		
		extendedTask.SetTaskProgress(percentage);
		Print("SCR_TaskSystem: Changed progress of task with ID: " + task.GetTaskID() + " to " + percentage + "%", LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to add progress to task
	//! \param[in] task
	//! \param[in] percentage
	//! \return
	void AddTaskProgress(notnull SCR_Task task, float percentage)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
		{
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " is not an extended task!", LogLevel.WARNING);
			return;
		}
		
		extendedTask.AddTaskProgress(percentage);
		Print("SCR_TaskSystem: Added " + percentage + "% progress to task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to remove progress from task
	//! \param[in] task
	//! \param[in] percentage
	//! \return
	void RemoveTaskProgress(notnull SCR_Task task, float percentage)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
		{
			Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " is not an extended task!", LogLevel.WARNING);
			return;
		}
		
		extendedTask.RemoveTaskProgress(percentage);
		Print("SCR_TaskSystem: Removed " + percentage + "% progress from task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns state of task
	//! \param[in] task
	//! \return
	SCR_ETaskState GetTaskState(notnull SCR_Task task)
	{
		if (!task)
			return -1;
		
		return task.GetTaskState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the state of task to the new state
	//! \param[in] task
	//! \param[in] state
	void SetTaskState(notnull SCR_Task task, SCR_ETaskState state)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		task.SetTaskState(state);
		Print("SCR_TaskSystem: Changed state for task with ID: " + task.GetTaskID() + " to " + SCR_Enum.GetEnumName(SCR_ETaskState, state), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns ownership of task
	//! \param[in] task
	//! \return
	SCR_ETaskOwnership GetTaskOwnership(notnull SCR_Task task)
	{
		if (!task)
			return -1;
		
		return task.GetTaskOwnership();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to change ownership of task to new ownership
	//! \param[in] task
	//! \param[in] ownership
	void SetTaskOwnership(notnull SCR_Task task, SCR_ETaskOwnership ownership)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (ownership > parentTask.GetTaskOwnership())
			{
				Print("SCR_TaskSystem: Cannot change ownership for task with ID: " + task.GetTaskID() + " as ownership level cannot be higher than parent!", LogLevel.WARNING);
				return;
			}
		}
		
		task.SetTaskOwnership(ownership);
		
		array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();
		if (assignees && !assignees.IsEmpty())
		{
			foreach (SCR_TaskExecutor assignee : assignees)
			{
				if (!CanTaskBeAssignedTo(task, assignee))
					task.RemoveTaskAssignee(assignee);
			}
		}
		
		Print("SCR_TaskSystem: Changed ownership for task with ID: " + task.GetTaskID() + " to " + SCR_Enum.GetEnumName(SCR_ETaskOwnership, ownership), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns visibility of task
	//! \param[in] task
	//! \return
	SCR_ETaskVisibility GetTaskVisibility(notnull SCR_Task task)
	{
		if (!task)
			return -1;
		
		return task.GetTaskVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to change visibility of task to new visibility
	//! \param[in] task
	//! \param[in] visibility
	void SetTaskVisibility(notnull SCR_Task task, SCR_ETaskVisibility visibility)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (visibility > parentTask.GetTaskVisibility())
			{
				Print("SCR_TaskSystem: Cannot change visibility for task with ID: " + task.GetTaskID() + " as visibility level cannot be higher than parent!", LogLevel.WARNING);
				return;
			}
		}
		
		task.SetTaskVisibility(visibility);
		Print("SCR_TaskSystem: Changed visibility for task with ID: " + task.GetTaskID() + " to " + SCR_Enum.GetEnumName(SCR_ETaskVisibility, visibility), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns visibility of task in UI
	//! \param[in] task
	//! \return
	SCR_ETaskUIVisibility GetTaskUIVisibility(notnull SCR_Task task)
	{
		if (!task)
			return -1;
		
		return task.GetTaskUIVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to change visibility of task to new visibility in UI
	//! \param[in] task
	//! \param[in] visibility
	void SetTaskUIVisibility(notnull SCR_Task task, SCR_ETaskUIVisibility visibility)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		task.SetTaskUIVisibility(visibility);
		Print("SCR_TaskSystem: Changed UI visibility for task with ID: " + task.GetTaskID() + " to " + SCR_Enum.GetEnumName(SCR_ETaskUIVisibility, visibility), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns factions of task
	//! \param[in] task
	//! \return
	array<string> GetTaskFactions(notnull SCR_Task task)
	{
		if (!task)
			return null;
		
		return task.GetOwnerFactionKeys();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to add faction to task
	//! \param[in] task
	//! \param[in] factionKey
	void AddTaskFaction(notnull SCR_Task task, FactionKey factionKey)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (parentTask.GetTaskOwnership() <= SCR_ETaskOwnership.FACTION || parentTask.GetTaskVisibility() <= SCR_ETaskVisibility.FACTION)
			{
				Print("SCR_TaskSystem: Cannot add faction to task with ID: " + task.GetTaskID() + " as parent task does not allow it!", LogLevel.WARNING);
				return;
			}
		}
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
			
		if (!factionManager.GetFactionByKey(factionKey))
		{
			Print("SCR_TaskSystem: Faction with key: " + factionKey + " does not exist!", LogLevel.WARNING);
			return;
		}
		
		task.AddOwnerFactionKey(factionKey);
		Print("SCR_TaskSystem: Added faction with key: " + factionKey + " to task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to remove faction from task
	//! \param[in] task
	//! \param[in] factionKey
	void RemoveTaskFaction(notnull SCR_Task task, FactionKey factionKey)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (parentTask.GetTaskOwnership() <= SCR_ETaskOwnership.FACTION || parentTask.GetTaskVisibility() <= SCR_ETaskVisibility.FACTION)
			{
				Print("SCR_TaskSystem: Cannot remove faction from task with ID: " + task.GetTaskID() + " as parent task does not allow it!", LogLevel.WARNING);
				return;
			}
		}
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
			
		if (!factionManager.GetFactionByKey(factionKey))
		{
			Print("SCR_TaskSystem: Faction with key: " + factionKey + " does not exist!", LogLevel.WARNING);
			return;
		}
		
		task.RemoveOwnerFactionKey(factionKey);
		
		array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();
		if (assignees && !assignees.IsEmpty())
		{
			foreach (SCR_TaskExecutor assignee : assignees)
			{
				if (!CanTaskBeAssignedTo(task, assignee))
					task.RemoveTaskAssignee(assignee);
			}
		}
		
		Print("SCR_TaskSystem: Removed faction with key: " + factionKey + " from task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns group IDs of task
	//! \param[in] task
	//! \return
	array<int> GetTaskGroups(notnull SCR_Task task)
	{
		if (!task)
			return null;
		
		return task.GetOwnerGroupIDs();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds group ID to task
	//! \param[in] task
	//! \param[in] groupID
	void AddTaskGroup(notnull SCR_Task task, int groupID)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
			
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
		{
			Print("SCR_TaskSystem: Group with ID: " + groupID + " does not exist!", LogLevel.WARNING);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (parentTask.GetTaskOwnership() <= SCR_ETaskOwnership.GROUP || parentTask.GetTaskVisibility() <= SCR_ETaskVisibility.GROUP)
			{
				Print("SCR_TaskSystem: Cannot add group with ID: " + groupID + " to task with ID: " + task.GetTaskID() + " as parent task does not allow it!", LogLevel.WARNING);
				return;
			}
				
			array<string> parentTaskFactions = parentTask.GetOwnerFactionKeys();
			if ((!parentTaskFactions || !parentTaskFactions.Contains(group.GetFactionName())) && parentTask.GetTaskOwnership() == SCR_ETaskOwnership.FACTION)
			{
				Print("SCR_TaskSystem: Cannot add group with ID: " + groupID + " to task with ID: " + task.GetTaskID() + " as group is from another faction!", LogLevel.WARNING);
				return;
			}
		}
		
		task.AddOwnerGroupID(groupID);
		Print("SCR_TaskSystem: Added group with ID: " + groupID + " to task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes group ID from task
	//! \param[in] task
	//! \param[in] groupID
	void RemoveTaskGroup(notnull SCR_Task task, int groupID)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
			
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
		{
			Print("SCR_TaskSystem: Group with ID: " + groupID + " does not exist!", LogLevel.WARNING);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (parentTask.GetTaskOwnership() <= SCR_ETaskOwnership.GROUP || parentTask.GetTaskVisibility() <= SCR_ETaskVisibility.GROUP)
			{
				Print("SCR_TaskSystem: Cannot remove group with ID: " + groupID + " from task with ID: " + task.GetTaskID() + " as parent task does not allow it!", LogLevel.WARNING);
				return;
			}
		}
		
		task.RemoveOwnerGroupID(groupID);
		
		array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();
		if (assignees && !assignees.IsEmpty())
		{
			foreach (SCR_TaskExecutor assignee : assignees)
			{
				if (!CanTaskBeAssignedTo(task, assignee))
					task.RemoveTaskAssignee(assignee);
			}
		}
		
		Print("SCR_TaskSystem: Removed group with ID: " + groupID + " from task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns owner executors of task
	//! \param[in] task
	//! \return
	array<ref SCR_TaskExecutor> GetTaskExecutors(notnull SCR_Task task)
	{
		if (!task)
			return null;
		
		return task.GetOwnerExecutors();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds given executor to task as an owner/viewer depending on task ownership and visibility
	//! \param[in] task
	//! \param[in] executor
	void AddTaskExecutor(notnull SCR_Task task, notnull SCR_TaskExecutor executor)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		string name, suffix;
		if (executor)
		{
			SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
			if (executorPlayer)
			{	
				PlayerManager playerManager = GetGame().GetPlayerManager();
				if (!playerManager)
					return;
				
				int playerID = executorPlayer.GetPlayerID();
				name = playerManager.GetPlayerName(playerID);
				suffix = playerID.ToString();
			}
			
			SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
			if (executorEntity)
			{
				IEntity entity = executorEntity.GetEntity();
				if (!entity)
					return;
				
				name = entity.GetName();		
				suffix = entity.Type().ToString();
			}
			
			SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
			if (executorGroup)
			{
				SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
				if (!groupsManagerComponent)
					return;
			
				SCR_AIGroup group = groupsManagerComponent.FindGroup(executorGroup.GetGroupID());
				if (!group)
					return;
					
				name = "Group";
				suffix = group.GetGroupID().ToString();
			}
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (parentTask.GetTaskOwnership() <= SCR_ETaskOwnership.EXECUTOR || parentTask.GetTaskVisibility() <= SCR_ETaskVisibility.EXECUTOR)
			{
				Print("SCR_TaskSystem: Cannot add " + name + "(" + suffix + ") to task with ID: " + task.GetTaskID() + " as parent task does not allow it!", LogLevel.WARNING);
				return;
			}
			
			if (parentTask.GetTaskOwnership() == SCR_ETaskOwnership.FACTION || parentTask.GetTaskVisibility() == SCR_ETaskVisibility.FACTION)
			{			
				string factionKey = executor.GetFactionKey();
				
				array<string> parentTaskFactions = parentTask.GetOwnerFactionKeys();
				if (!parentTaskFactions || !parentTaskFactions.Contains(factionKey))
				{
					Print("SCR_TaskSystem: Cannot add " + name + "(" + suffix + ") to task with ID: " + task.GetTaskID() + " as entity is from another faction!", LogLevel.WARNING);
					return;
				}
			}
			
			if (parentTask.GetTaskOwnership() == SCR_ETaskOwnership.GROUP || parentTask.GetTaskVisibility() == SCR_ETaskVisibility.GROUP)
			{
				int groupID = executor.GetGroupID();
			
				array<int> parentTaskGroupIDs = parentTask.GetOwnerGroupIDs();
				if (!parentTaskGroupIDs)
					return;
			
				if (!parentTaskGroupIDs.Contains(groupID))
				{
					Print("SCR_TaskSystem: Cannot add " + name + "(" + suffix + ") to task with ID: " + task.GetTaskID() + " as entity is from another group!", LogLevel.WARNING);
					return;
				}
			}
		}
		
		if (!task.AddOwnerExecutor(executor))
		{
			Print("SCR_TaskSystem: Could not add " + name + "(" + suffix + ") to task with ID: " + task.GetTaskID(), LogLevel.ERROR);
			return;
		}
		
		Print("SCR_TaskSystem: Added " + name + "(" + suffix + ") to task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes given executor from task as an owner/viewer depending on task ownership and visibility
	//! \param[in] task
	//! \param[in] executor
	void RemoveTaskExecutor(notnull SCR_Task task, notnull SCR_TaskExecutor executor)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		string name, suffix;
		if (executor)
		{
			SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
			if (executorPlayer)
			{	
				PlayerManager playerManager = GetGame().GetPlayerManager();
				if (!playerManager)
					return;
				
				int playerID = executorPlayer.GetPlayerID();
				name = playerManager.GetPlayerName(playerID);
				suffix = playerID.ToString();
			}
			
			SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
			if (executorEntity)
			{
				IEntity entity = executorEntity.GetEntity();
				if (!entity)
					return;
				
				name = entity.GetName();		
				suffix = entity.Type().ToString();
			}
			
			SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
			if (executorGroup)
			{
				SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
				if (!groupsManagerComponent)
					return;
			
				SCR_AIGroup group = groupsManagerComponent.FindGroup(executorGroup.GetGroupID());
				if (!group)
					return;
					
				name = "Group";
				suffix = group.GetGroupID().ToString();
			}
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask && extendedTask.IsChildTask())
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
				return;
				
			if (parentTask.GetTaskOwnership() <= SCR_ETaskOwnership.EXECUTOR || parentTask.GetTaskVisibility() <= SCR_ETaskVisibility.EXECUTOR)
			{
				Print("SCR_TaskSystem: Cannot remove " + name + "(" + suffix + ") from task with ID: " + task.GetTaskID() + " as parent task does not allow it!", LogLevel.WARNING);
				return;
			}
		}
		
		if (!task.RemoveOwnerExecutor(executor))
		{
			Print("SCR_TaskSystem: Could not remove " + name + "(" + suffix + ") from task with ID: " + task.GetTaskID(), LogLevel.ERROR);
			return;
		}
		
		array<ref SCR_TaskExecutor> assignees = task.GetTaskAssignees();
		if (assignees && !assignees.IsEmpty())
		{
			foreach (SCR_TaskExecutor assignee : assignees)
			{
				if (!CanTaskBeAssignedTo(task, assignee))
					task.RemoveTaskAssignee(assignee);
			}
		}
		
		Print("SCR_TaskSystem: Removed " + name + "(" + suffix + ") from task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns location of task as vector
	//! \param[in] task
	//! \return
	vector GetTaskLocation(notnull SCR_Task task)
	{
		if (!task)
			return vector.Zero;
		
		return task.GetTaskPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the location of task to a new location
	//! \param[in] task
	//! \param[in] destination
	void MoveTask(notnull SCR_Task task, vector destination)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		task.SetTaskPosition(destination);
		Print("SCR_TaskSystem: Task with ID: " + task.GetTaskID() + " moved to " + destination, LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outputs parent task of provided child
	//! \param[in] task
	//! \param[out] parentTask
	void GetParentTasksFor(notnull SCR_Task task, out SCR_Task parentTask)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
			return;
		
		parentTask = extendedTask.GetParentTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Outputs array of tasks parented to task
	//! \param[in] task
	//! \param[out] childTasks
	void GetChildTasksFor(notnull SCR_Task task, out array<SCR_Task> childTasks)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
			return;
		
		childTasks = extendedTask.GetChildTasks();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to parent childTask to task; Tasks already parented to another cannot have their own child tasks as maximum allowed depth is 1
	//! \param[in] task
	//! \param[in] childTask
	void AddChildTaskTo(notnull SCR_Task task, notnull SCR_Task childTask)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedParentTask = SCR_ExtendedTask.Cast(task);
		SCR_ExtendedTask extendedChildTask = SCR_ExtendedTask.Cast(childTask);
		if (!extendedParentTask || !extendedChildTask)
		{
			Print("SCR_TaskSystem: Both parent and child tasks need to be extended tasks!", LogLevel.WARNING);
			return;
		}
		
		if (extendedParentTask.GetNodeDepth() > 0)
		{
			Print("SCR_TaskSystem: Tasks cannot be nested beyond depth 1!", LogLevel.WARNING);
			return;
		}
		
		if (!extendedParentTask.AddChildTask(childTask))
		{
			Print("SCR_TaskSystem: Could not parent task with ID: " + childTask.GetTaskID() + " to task with ID: " + task.GetTaskID(), LogLevel.ERROR);
			return;
		}
		
		Print("SCR_TaskSystem: Task with ID: " + childTask.GetTaskID() + " parented to task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to unparent childTask from task
	//! \param[in] task
	//! \param[in] childTask
	void RemoveChildTaskFrom(notnull SCR_Task task, notnull SCR_Task childTask)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
		{
			Print("SCR_TaskSystem: Parent task needs to be an extended task!", LogLevel.ERROR);
			return;
		}
		
		if (!extendedTask.RemoveChildTask(childTask))
		{
			Print("SCR_TaskSystem: Could not unparent task with ID: " + childTask.GetTaskID() + " from task with ID: " + task.GetTaskID(), LogLevel.ERROR);
			return;
		}
		
		Print("SCR_TaskSystem: Task with ID: " + childTask.GetTaskID() + " unparented from task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a new task and registers it into SCR_TaskSystem
	//! \param[in] taskResource
	//! \param[in] taskID
	//! \param[in] name
	//! \param[in] desc
	//! \param[in] position
	//! \return
	SCR_Task CreateTask(ResourceName taskResourceName, string taskID, string name, string desc, vector position = vector.Zero, int playerId = -1)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return null;
		}
		
		if (GetTaskFromTaskID(taskID, false))
		{
			Print("SCR_TaskSystem: Task with ID: " + taskID + " already exists!", LogLevel.WARNING);
			return null;
		}
		
		if (!taskResourceName || taskResourceName.IsEmpty())
		{			
			Print("SCR_TaskSystem: Task resource name is empty! Using default task resource", LogLevel.WARNING);
			taskResourceName = BASE_TASK_RESOURCE;
		}
		
		Resource taskResource = Resource.Load(taskResourceName);
		if (!taskResource || !taskResource.IsValid())
		{
			Print("SCR_TaskSystem: Task resource is invalid!", LogLevel.WARNING);
			return null;
		}

		IEntitySource entitySource = taskResource.GetResource().ToEntitySource();
		if (!entitySource)
			return null;

		typename taskType = entitySource.GetClassName().ToType();
		if (!taskType.IsInherited(SCR_Task))
			return null;
		
		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		transform[3] = position;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform = transform;
		
		SCR_Task task = SCR_Task.Cast(GetGame().SpawnEntityPrefab(taskResource, GetGame().GetWorld(), spawnParams));
		if (!task || !task.FindComponent(RplComponent))
		{
			Print("SCR_TaskSystem: Failed to create task with ID: " + taskID, LogLevel.WARNING);
			return null;
		}
		
		task.SetTaskID(taskID);
		task.SetAuthorID(playerId);
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(name))
			task.SetTaskName(name);
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(desc))
			task.SetTaskDescription(desc);

		Print("SCR_TaskSystem: Created task with ID: " + taskID, LogLevel.DEBUG);
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deletes task from SCR_TaskSystem
	//! \param[in] task
	void DeleteTask(notnull SCR_Task task)
	{
		if (Replication.IsClient())
		{
			Print("SCR_TaskSystem: Trying to invoke server-only function on client", LogLevel.ERROR);
			return;
		}
		
		RplComponent taskRpl = RplComponent.Cast(task.FindComponent(RplComponent));		
		if (!taskRpl)
		{
			Print("SCR_TaskSystem: Missing Rpl Component; Cannot delete task with ID: " + task.GetTaskID(), LogLevel.WARNING);
			return;
		}
		
		Print("SCR_TaskSystem: Deleted task with ID: " + task.GetTaskID(), LogLevel.DEBUG);
		taskRpl.DeleteRplEntity(task, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds task by taskID and returns it; Set notify to false to suppress warning message in case task could not be found
	//! \param[in] taskID
	//! \param[in] notify
	//! \return
	static SCR_Task GetTaskFromTaskID(string taskID, bool notify = true)
	{
		foreach (SCR_Task task : s_aTasks)
		{
			if (!task)
				continue;
			
			string ID = task.GetTaskID();			
			if (!ID || ID.IsEmpty())
				continue;
			
			if (taskID == ID)
				return task;
		}
		
		if (notify)
			Print("SCR_TaskSystem: Task with ID: " + taskID + " does not exist!", LogLevel.WARNING);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates task executor from player ID and returns it
	//! \param[in] playerID
	//! \return
	static SCR_TaskExecutor TaskExecutorFromPlayerID(int playerID)
	{
		return SCR_TaskExecutor.FromPlayerID(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates task executor from entity and returns it
	//! \param[in] ent
	//! \return
	static SCR_TaskExecutor TaskExecutorFromEntity(IEntity ent)
	{
		return SCR_TaskExecutor.FromEntity(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates task executor from player group and returns it
	//! \param[in] groupID
	//! \return
	static SCR_TaskExecutor TaskExecutorFromGroup(int groupID)
	{
		return SCR_TaskExecutor.FromGroup(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		// Debug task system
#ifdef ENABLE_DIAG
		if (args.GetPoint() != WorldSystemPoint.Frame)
			return;
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_TASKS_CREATE))
			Diag_CreateTask();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_TASKS_LIST))
			Diag_TaskList();		
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_CreateTask()
	{
		DbgUI.Begin("Create Task");
			
		string taskID;
		DbgUI.InputText("ID: ", taskID);
			
		string taskName;
		DbgUI.InputText("Name: ", taskName);
			
		string taskDesc;
		DbgUI.InputText("Description: ", taskDesc);
			
		if (DbgUI.Button("Create Base Task") &&
			!SCR_StringHelper.IsEmptyOrWhiteSpace(taskID) &&
			!SCR_StringHelper.IsEmptyOrWhiteSpace(taskName))
			CreateTask(BASE_TASK_RESOURCE, taskID, taskName, taskDesc);
		
		if (DbgUI.Button("Create Extended Task") &&
			!SCR_StringHelper.IsEmptyOrWhiteSpace(taskID) &&
			!SCR_StringHelper.IsEmptyOrWhiteSpace(taskName))
			CreateTask(EXTENDED_TASK_RESOURCE, taskID, taskName, taskDesc);
			
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_TaskList()
	{	
		array<SCR_Task> tasks = {};
		int taskCount;
		string header;
		
		if (diag_taskListMenu && diag_selectedExecutor)
		{
			taskCount = GetTasksVisibleFor(tasks, diag_selectedExecutor);
			
			string executorName;
			
			SCR_TaskExecutorPlayer playerExecutor = SCR_TaskExecutorPlayer.Cast(diag_selectedExecutor);
			if (playerExecutor)
				executorName = GetGame().GetPlayerManager().GetPlayerName(playerExecutor.GetPlayerID());
			
			SCR_TaskExecutorEntity entityExecutor = SCR_TaskExecutorEntity.Cast(diag_selectedExecutor);
			if (entityExecutor)
				executorName = entityExecutor.GetEntity().GetName();
			
			SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(diag_selectedExecutor);
			if (groupExecutor)
				executorName = groupExecutor.GetGroupID().ToString();
			
			header = string.Format("Task List for %1 (%2)", executorName, taskCount);
		}
		else
		{
			taskCount = GetTasks(tasks);
			header = string.Format("Task List (%1)", taskCount);
		}
		
		DbgUI.Begin(header);
		
		bool checked = diag_taskListMenu;
		DbgUI.Check("Show Task List for: ", checked);
		diag_taskListMenu = checked;
		
		if (diag_taskListMenu)
			Diag_ShowTaskListFor();
		
		DbgUI.Text("________________________________________________");
		
		int currentTask;	
		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;
			
			SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
			if (extendedTask && extendedTask.IsChildTask())
				continue;
			
			Diag_Task(task, currentTask);
			currentTask++;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_Task(notnull SCR_Task task, int index, int subIndex = -1)
	{
		SCR_TaskData data = task.GetTaskData();
		if (!data)
			return;
			
		string taskName = task.GetTaskName();
		if (taskName.Length() > DIAG_MAX_NAME_LENGTH_LIST)
			taskName = taskName.Substring(0, DIAG_MAX_NAME_LENGTH_LIST) + "...";
		
		string taskText;
		if (subIndex > -1)
			taskText = string.Format("%1.%2 - id:%3 | name:%4", index, subIndex, task.GetTaskID(), taskName); 
		else
			taskText = string.Format("%1 - id:%2 | name:%3", index, task.GetTaskID(), taskName);
		
		bool checked = data.diag_isChecked;		
		DbgUI.Check(taskText + " " , checked);
		data.diag_isChecked = checked;

		if (data.diag_isChecked)
			Diag_TaskWindow(task, data);
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask || !extendedTask.HasChildTasks())
			return;
		
		array<SCR_Task> childTasks = extendedTask.GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		int currentChildTask;
		foreach (SCR_Task childTask : childTasks)
		{
			if (!childTask)
				continue;
			
			Diag_Task(childTask, index, currentChildTask);
			currentChildTask++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_ShowTaskListFor()
	{
		DbgUI.Begin("Show task list for individual executor");
		
		array<string> executorTypes = {"PLAYER", "ENTITY", "GROUP"};
		DbgUI.Combo("Executor Type: ", diag_taskListMenuSelection, executorTypes);
		
		DbgUI.InputText("Executor Name/ID: ", diag_taskListMenuExecutorName);
		
		DbgUI.End();
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(diag_taskListMenuExecutorName))
		{
			switch (diag_taskListMenuSelection)
			{
				// Player Executor
				case 0:
					int playerID = diag_taskListMenuExecutorName.ToInt();
					if (playerID > -1)
						diag_selectedExecutor = SCR_TaskExecutor.FromPlayerID(playerID);
				
					break;
				// Entity Executor
				case 1:
					IEntity entity = GetGame().FindEntity(diag_taskListMenuExecutorName);
					if (entity)
						diag_selectedExecutor = SCR_TaskExecutor.FromEntity(entity);
				
					break;
				// Group Executor
				case 2:
					int groupID = diag_taskListMenuExecutorName.ToInt();
					if (groupID > -1)
						diag_selectedExecutor = SCR_TaskExecutor.FromGroup(groupID);
				
					break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_TaskWindow(notnull SCR_Task task, notnull SCR_TaskData data)
	{
		DbgUI.Begin(string.Format("Task: %1 (%2)", task.GetTaskID(), task.Type()));
		
		bool checked;
				
		// Name
		string taskName = task.GetTaskName();
		if (taskName.Length() > DIAG_MAX_NAME_LENGTH)
			taskName = taskName.Substring(0, DIAG_MAX_NAME_LENGTH) + "...";
		
		checked = data.diag_checkedSettings[0];
		DbgUI.Check(string.Format("Name: %1", taskName), checked);
		data.diag_checkedSettings[0] = checked;
		
		if (data.diag_checkedSettings[0])
			Diag_EditStringContextMenu(task, "Name", 0);
		
		// Description
		string taskDesc = task.GetTaskDescription();
		if (taskDesc.Length() > DIAG_MAX_DESCRIPTION_LENGTH)
			taskDesc = taskDesc.Substring(0, DIAG_MAX_DESCRIPTION_LENGTH) + "...";
		
		checked = data.diag_checkedSettings[1];
		DbgUI.Check(string.Format("Description: %1", taskDesc), checked);
		data.diag_checkedSettings[1] = checked;
		
		if (data.diag_checkedSettings[1])
			Diag_EditStringContextMenu(task, "Description", 1);
		
		// State
		checked = data.diag_checkedSettings[2];
		DbgUI.Check(string.Format("State: %1", SCR_Enum.GetEnumName(SCR_ETaskState, task.GetTaskState())), checked);
		data.diag_checkedSettings[2] = checked;
		
		if (data.diag_checkedSettings[2])
			Diag_EditEnumContextMenu(task, SCR_ETaskState, 2);
		
		// Ownership
		checked = data.diag_checkedSettings[3];
		DbgUI.Check(string.Format("Ownership: %1", SCR_Enum.GetEnumName(SCR_ETaskOwnership, task.GetTaskOwnership())), checked);
		data.diag_checkedSettings[3] = checked;
		
		if (data.diag_checkedSettings[3])
			Diag_EditEnumContextMenu(task, SCR_ETaskOwnership, 3);
		
		// Visibility
		checked = data.diag_checkedSettings[4];
		DbgUI.Check(string.Format("Visibility: %1", SCR_Enum.GetEnumName(SCR_ETaskVisibility, task.GetTaskVisibility())), checked);
		data.diag_checkedSettings[4] = checked;
		
		if (data.diag_checkedSettings[4])
			Diag_EditEnumContextMenu(task, SCR_ETaskVisibility, 4);
		
		// UI Visibility
		checked = data.diag_checkedSettings[5];
		DbgUI.Check(string.Format("UI Visibility: %1", SCR_Enum.GetEnumName(SCR_ETaskUIVisibility, task.GetTaskUIVisibility())), checked);
		data.diag_checkedSettings[5] = checked;
		
		if (data.diag_checkedSettings[5])
			Diag_EditEnumContextMenu(task, SCR_ETaskUIVisibility, 5);
		
		// Assignees
		checked = data.diag_checkedSettings[6];
		DbgUI.Check(string.Format("Assignees: %1", task.GetTaskAssigneeCount()), checked);
		data.diag_checkedSettings[6] = checked;
		
		if (data.diag_checkedSettings[6])
			Diag_EditExecutorContextMenu(task, "Assignee", 6);
		
		// Owner Executors
		checked = data.diag_checkedSettings[7];
		DbgUI.Check(string.Format("Executors: %1", task.GetOwnerExecutorCount()), checked);
		data.diag_checkedSettings[7] = checked;
		
		if (data.diag_checkedSettings[7])
			Diag_EditExecutorContextMenu(task, "Executor", 7);
		
		// Owner Factions
		checked = data.diag_checkedSettings[8];
		DbgUI.Check(string.Format("Factions: %1", task.GetOwnerFactionCount()), checked);
		data.diag_checkedSettings[8] = checked;
		
		if (data.diag_checkedSettings[8])
			Diag_EditFactionContextMenu(task, 8);
		
		// Owner Groups
		checked = data.diag_checkedSettings[9];
		DbgUI.Check(string.Format("Groups: %1", task.GetOwnerGroupCount()), checked);
		data.diag_checkedSettings[9] = checked;
		
		if (data.diag_checkedSettings[9])
			Diag_EditGroupContextMenu(task, 9);
		
		// Position
		checked = data.diag_checkedSettings[10];
		DbgUI.Check(string.Format("Position: %1", task.GetTaskPosition()), checked);
		data.diag_checkedSettings[10] = checked;
		
		if (data.diag_checkedSettings[10])
			Diag_MoveTaskContextMenu(task, 10);
		
		// Extended Task Settings
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(data);	
		if (extendedTask && extendedData)
		{
			// Child Tasks
			checked = extendedData.diag_checkedExtendedSettings[0];
			DbgUI.Check(string.Format("Child Tasks: %1", extendedTask.GetChildTaskCount()), checked);
			extendedData.diag_checkedExtendedSettings[0] = checked;
		
			if (extendedData.diag_checkedExtendedSettings[0])
				Diag_EditChildTasksContextMenu(extendedTask, 0);
			
			// Progress
			checked = extendedData.diag_checkedExtendedSettings[1];
			DbgUI.Check(string.Format("Progress: %1%2", extendedTask.GetTaskProgress(), "%"), checked);
			extendedData.diag_checkedExtendedSettings[1] = checked;
		
			if (extendedData.diag_checkedExtendedSettings[1])
				Diag_EditProgressContextMenu(extendedTask, 1);
		}
		
		// Shortcuts
		DbgUI.Text("_________________________________________");
		DbgUI.Text("Shortcuts:");
		
		PlayerController localPlayerController;
		int localPlayerID;
		SCR_TaskExecutor localPlayerExecutor;
		SCR_TaskSystemNetworkComponent networkComponent;
		
		if (DbgUI.Button("Assign Self"))
		{
			localPlayerController = GetGame().GetPlayerController();
			localPlayerID = localPlayerController.GetPlayerId();
			localPlayerExecutor = SCR_TaskExecutor.FromPlayerID(localPlayerID);
			
			networkComponent = SCR_TaskSystemNetworkComponent.Cast(localPlayerController.FindComponent(SCR_TaskSystemNetworkComponent));
			if (networkComponent)
				networkComponent.AssignTask(task, localPlayerExecutor, true);
		}
		
		if (DbgUI.Button("Unassign Self"))
		{
			localPlayerController = GetGame().GetPlayerController();
			localPlayerID = localPlayerController.GetPlayerId();
			localPlayerExecutor = SCR_TaskExecutor.FromPlayerID(localPlayerID);
			
			networkComponent = SCR_TaskSystemNetworkComponent.Cast(localPlayerController.FindComponent(SCR_TaskSystemNetworkComponent));
			if (networkComponent)
				networkComponent.UnassignTask(task, localPlayerExecutor);
		}
		
		if (DbgUI.Button("Complete Task"))
			SetTaskState(task, SCR_ETaskState.COMPLETED);
		
		if (DbgUI.Button("Fail Task"))
			SetTaskState(task, SCR_ETaskState.FAILED);
		
		// Delete
		if (DbgUI.Button("Delete Task"))
			DeleteTask(task);
						
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditStringContextMenu(notnull SCR_Task task, string name, int settingIndex)
	{
		string input;
		switch (name)
		{
			case "Name":
				input = task.GetTaskName();
				break;
			case "Description":
				input = task.GetTaskDescription();
				break;
		}
		
		DbgUI.Begin(string.Format("Edit %1 for task: %2", name, task.GetTaskID()));
		
		DbgUI.InputText(string.Format("%1:", name), input);
		
		if (DbgUI.Button(string.Format("Set %1", name)) && !SCR_StringHelper.IsEmptyOrWhiteSpace(input))
		{
			switch (name)
			{
				case "Name":
					task.SetTaskName(input);
					break;
				case "Description":
					task.SetTaskDescription(input);
					break;
			}
			
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditEnumContextMenu(notnull SCR_Task task, typename enumType, int settingIndex)
	{
		string enumName;
		int selection;
		switch (enumType)
		{
			case SCR_ETaskState:
				enumName = "State";
				selection = Math.Log2(task.GetTaskState());
				break;
			case SCR_ETaskOwnership:
				enumName = "Ownership";
				selection = task.GetTaskOwnership();
				break;
			case SCR_ETaskVisibility:
				enumName = "Visibility";
				selection = task.GetTaskVisibility();
				break;
			case SCR_ETaskUIVisibility:
				enumName = "UI Visibility";
				selection = task.GetTaskUIVisibility();
				break;
		}
		
		DbgUI.Begin(string.Format("Edit %1 for task: %2", enumName, task.GetTaskID()));
		
		array<string> enumNames = {};
		SCR_Enum.GetEnumNames(enumType, enumNames);
		
		DbgUI.Combo(enumName, selection, enumNames);
		
		if (DbgUI.Button(string.Format("Set %1", enumName)))
		{
			switch (enumType)
			{
				case SCR_ETaskState:
					SetTaskState(task, 1 << selection);
					break;
				case SCR_ETaskOwnership:
					SetTaskOwnership(task, selection);
					break;
				case SCR_ETaskVisibility:
					SetTaskVisibility(task, selection);
					break;
				case SCR_ETaskUIVisibility:
					SetTaskUIVisibility(task, selection);
					break;
			}
			
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditExecutorContextMenu(notnull SCR_Task task, string name, int settingIndex)
	{
		DbgUI.Begin(string.Format("Add/Remove %1 for task: %2", name, task.GetTaskID()));
		
		array<ref SCR_TaskExecutor> executors;
		switch (name)
		{
			case "Assignee":
				executors = task.GetTaskAssignees();
				break;
			case "Executor":
				executors = task.GetOwnerExecutors();
				break;
		}
		
		int currentExecutor;
		if (executors && !executors.IsEmpty())
		{
			foreach (SCR_TaskExecutor executor : executors)
			{			
				string executorName, executorType;
						
				SCR_TaskExecutorPlayer playerExecutor = SCR_TaskExecutorPlayer.Cast(executor);
				if (playerExecutor)
				{
					executorName = GetGame().GetPlayerManager().GetPlayerName(playerExecutor.GetPlayerID());
					executorType = "PLAYER";
				}
			
				SCR_TaskExecutorEntity entityExecutor = SCR_TaskExecutorEntity.Cast(executor);
				if (entityExecutor)
				{
					executorName = entityExecutor.GetEntity().GetName();
					executorType = "ENTITY";
				}
				
				SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(executor);
				if (groupExecutor)
				{
					executorName = groupExecutor.GetGroupID().ToString();
					executorType = "GROUP";
				}
			
				DbgUI.Text(string.Format("%1 - name/id:%2 | type:%3", currentExecutor, executorName, executorType));			
				++currentExecutor;
			}
		}
		
		if (currentExecutor > 0)
			DbgUI.Text("________________________________________________");
		
		array<string> executorTypes = {"PLAYER", "ENTITY", "GROUP"};
		int selection;
		DbgUI.Combo("Executor Type: ", selection, executorTypes);
		
		string input;
		DbgUI.InputText("Executor Name/ID: ", input);	
		
		SCR_TaskExecutor executor;
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(input))
		{
			switch (selection)
			{
				// Player Executor
				case 0:
					int playerID = input.ToInt();
					if (playerID > -1)
						executor = SCR_TaskExecutor.FromPlayerID(playerID);
				
					break;
				// Entity Executor
				case 1:
					IEntity entity = GetGame().FindEntity(input);
					if (entity)
						executor = SCR_TaskExecutor.FromEntity(entity);
				
					break;
				// Group Executor
				case 2:
					int groupID = input.ToInt();
					if (groupID > -1)
						executor = SCR_TaskExecutor.FromGroup(groupID);
				
					break;
			}
		}
		
		PlayerController localPlayerController;
		SCR_TaskSystemNetworkComponent networkComponent;
		
		if (DbgUI.Button(string.Format("Add %1", name)) && executor)
		{
			localPlayerController = GetGame().GetPlayerController();
			networkComponent = SCR_TaskSystemNetworkComponent.Cast(localPlayerController.FindComponent(SCR_TaskSystemNetworkComponent));
			
			switch (name)
			{
				case "Assignee":
					networkComponent.AssignTask(task, executor, true);
					break;
				case "Executor":
					AddTaskExecutor(task, executor);
					break;
			}
			
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		if (DbgUI.Button(string.Format("Remove %1", name)) && executor)
		{
			localPlayerController = GetGame().GetPlayerController();
			networkComponent = SCR_TaskSystemNetworkComponent.Cast(localPlayerController.FindComponent(SCR_TaskSystemNetworkComponent));
			
			switch (name)
			{
				case "Assignee":
					networkComponent.UnassignTask(task, executor);
					break;
				case "Executor":
					RemoveTaskExecutor(task, executor);
					break;
			}
			
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditFactionContextMenu(notnull SCR_Task task, int settingIndex)
	{
		DbgUI.Begin(string.Format("Add/Remove Faction for task: %1", task.GetTaskID()));
		
		array<string> factionKeys = task.GetOwnerFactionKeys();
		
		int currentFaction = 0;
		if (factionKeys && !factionKeys.IsEmpty())
		{
			foreach (string factionKey : factionKeys)
			{			
				DbgUI.Text(string.Format("%1 - key:%2", currentFaction, factionKey));
				++currentFaction;
			}
		}
		
		if (currentFaction > 0)
			DbgUI.Text("________________________________________________");
		
		string input;
		DbgUI.InputText("Faction Key: ", input);
		
		if (DbgUI.Button("Add Faction"))
		{		
			AddTaskFaction(task, input);
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		if (DbgUI.Button("Remove Faction"))
		{
			RemoveTaskFaction(task, input);
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditGroupContextMenu(notnull SCR_Task task, int settingIndex)
	{
		DbgUI.Begin(string.Format("Add/Remove Group for task: %1", task.GetTaskID()));
		
		array<int> groupIDs = task.GetOwnerGroupIDs();
		
		int currentGroup = 0;
		if (groupIDs && !groupIDs.IsEmpty())
		{
			foreach (int groupID : groupIDs)
			{			
				DbgUI.Text(string.Format("%1 - id:%2", currentGroup, groupID.ToString()));
				++currentGroup;
			}
		}
		
		if (currentGroup > 0)
			DbgUI.Text("________________________________________________");
		
		string input;
		DbgUI.InputText("Group ID: ", input);
		
		if (DbgUI.Button("Add Group"))
		{		
			AddTaskGroup(task, input.ToInt());
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		if (DbgUI.Button("Remove Group"))
		{
			RemoveTaskGroup(task, input.ToInt());
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_MoveTaskContextMenu(notnull SCR_Task task, int settingIndex)
	{
		DbgUI.Begin(string.Format("Move task: %1", task.GetTaskID()));
		
		vector position = task.GetTaskPosition();
		float x = position[0];
		float y = position[1];
		float z = position[2];
		DbgUI.InputFloat("X: ", x);
		DbgUI.InputFloat("Y: ", y);
		DbgUI.InputFloat("Z: ", z);
		
		if (DbgUI.Button("Move Task"))
		{
			MoveTask(task, {x, y, z});
			task.GetTaskData().diag_checkedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditChildTasksContextMenu(notnull SCR_ExtendedTask extendedTask, int settingIndex)
	{
		DbgUI.Begin(string.Format("Add/Remove Child Task for task: %1", extendedTask.GetTaskID()));
		
		array<SCR_Task> childTasks = extendedTask.GetChildTasks();
		
		string taskName;
		int currentChildTask = 0;
		if (childTasks && !childTasks.IsEmpty())
		{
			foreach (SCR_Task childTask : childTasks)
			{			
				taskName = childTask.GetTaskName();
				if (taskName.Length() > DIAG_MAX_NAME_LENGTH_LIST)
					taskName = taskName.Substring(0, DIAG_MAX_NAME_LENGTH_LIST) + "...";	
			
				DbgUI.Text(string.Format("%1 - id:%2 | name:%3", currentChildTask, childTask.GetTaskID(), taskName));	
				++currentChildTask;
			}
		}
		
		if (currentChildTask > 0)
			DbgUI.Text("________________________________________________");
		
		string input;
		DbgUI.InputText("Task ID: ", input);
		
		if (DbgUI.Button("Parent Task"))
		{		
			SCR_Task task = GetTaskFromTaskID(input);
			if (task)
				AddChildTaskTo(extendedTask, task);
			
			SCR_ExtendedTaskData.Cast(extendedTask.GetTaskData()).diag_checkedExtendedSettings[settingIndex] = false;
		}
		
		if (DbgUI.Button("Unparent Task"))
		{
			SCR_Task task = GetTaskFromTaskID(input);
			if (task)
				RemoveChildTaskFrom(extendedTask, task);
			
			SCR_ExtendedTaskData.Cast(extendedTask.GetTaskData()).diag_checkedExtendedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Diag_EditProgressContextMenu(notnull SCR_ExtendedTask extendedTask, int settingIndex)
	{
		DbgUI.Begin(string.Format("Edit Progress for task: %1", extendedTask.GetTaskID()));
			
		float input = extendedTask.GetTaskProgress();
		DbgUI.InputFloat("Progress: ", input);
		
		if (DbgUI.Button("Set Progress"))
		{		
			SetTaskProgress(extendedTask, input);
			SCR_ExtendedTaskData.Cast(extendedTask.GetTaskData()).diag_checkedExtendedSettings[settingIndex] = false;
		}
		
		if (DbgUI.Button("Add Progress"))
		{		
			AddTaskProgress(extendedTask, input);
			SCR_ExtendedTaskData.Cast(extendedTask.GetTaskData()).diag_checkedExtendedSettings[settingIndex] = false;
		}
		
		if (DbgUI.Button("Remove Progress"))
		{		
			RemoveTaskProgress(extendedTask, input);
			SCR_ExtendedTaskData.Cast(extendedTask.GetTaskData()).diag_checkedExtendedSettings[settingIndex] = false;
		}
		
		DbgUI.End();
	}
}
