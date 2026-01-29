//[EntityEditorProps(category: "GameScripted/Tasks", description: "Entity that takes care of managing tasks.", color: "0 0 255 255")]
//class SCR_BaseTaskManagerClass: GenericEntityClass
//{
//};
//
//SCR_BaseTaskManager g_TaskManagerInstance;
//SCR_BaseTaskManager GetTaskManager()
//{
//	return g_TaskManagerInstance;
//};
//
////------------------------------------------------------------------------------------------------
//enum SCR_ETaskEventMask
//{
//	TASK_CREATED = 1,
//	TASK_CANCELED = 2,
//	TASK_PROPERTY_CHANGED = 4,
//	TASK_ASSIGNEE_CHANGED = 8,
//	TASK_FINISHED = 16,
//	TASK_FAILED = 32,
//	TASK_REMOVED = 64,
//};
//
////------------------------------------------------------------------------------------------------
//class SCR_BaseTaskManager : GenericEntity
//{
//
//#ifdef ENABLE_DIAG
//	protected bool m_bPrintedTasks = false;
//#endif
//	static const float DEFAULT_ABANDON_WAITING_TIME = 15;
//	
//	//**************************//
//	//PROTECTED MEMBER VARIABLES//
//	//**************************//
//	protected ref array<SCR_BaseTaskSupportEntity> m_aSupportedTaskTypes = {};
//	
//	[RplProp()]
//	protected float m_fTimestamp = 0;
//	protected float m_fTimestampTimer = 0;
//	protected ref array<SCR_Task> m_aTaskList = new array<SCR_Task>();
//	protected ref array<SCR_Task> m_aFinishedTaskList = {};
//	protected ref array<ref SCR_TaskAssignmentData> m_aCachedTaskAssignments = new array<ref SCR_TaskAssignmentData>();
//	
//	[Attribute("1")]
//	protected float m_fAssigneeCacheTimer;
//	
//	[Attribute("#AR-Tasks_TitleLocalRequest-UC")]
//	string m_sLocalRequestTitle;
//	
//	[Attribute("#AR-Tasks_ReassignPopup")]
//	string m_sReassignPopup;
//	
//	[Attribute("#AR-Tasks_AssignPopupGM")]
//	string m_sAssignPopupGM;
//	
//	protected float m_fAssigneeCacheCheckTimestamp;
//	protected ref map<SCR_BaseTaskExecutor, float> m_mAssigneesAbandoned = new map<SCR_BaseTaskExecutor, float>();
//	
//	protected bool m_bInitialized = false;
//	
//	protected RplComponent m_RplComponent;
//	
//	ref map<SCR_BaseTask, SCR_ETaskEventMask> m_mTaskEventMaskMap = new map<SCR_BaseTask, SCR_ETaskEventMask>();
//	
//	protected ref array<SCR_BaseTask> m_aTasksToDelete = {};
//	
//	//*****************************//
//	//PUBLIC STATIC SCRIPT INVOKERS//
//	//*****************************//
//	
//	static ref ScriptInvoker s_OnTaskUpdate = new ScriptInvoker();
//	static ref ScriptInvoker s_OnTaskFinished = new ScriptInvoker();
//	static ref ScriptInvoker s_OnTaskFailed = new ScriptInvoker();
//	static ref ScriptInvoker s_OnTaskRemoved = new ScriptInvoker();
//	static ref ScriptInvoker s_OnTaskCancelled = new ScriptInvoker();
//	
//	static ref ScriptInvoker s_OnTaskAssigned = new ScriptInvoker();
//	static ref ScriptInvoker s_OnTaskUnassigned = new ScriptInvoker();
//	
//	static ref ScriptInvoker s_OnTaskFactionAssigned = new ScriptInvoker();
//	
//	static ref ScriptInvoker s_OnTaskCreated = new ScriptInvoker();
//	static ref ScriptInvoker s_OnTaskDeleted = new ScriptInvoker();
//	
//	static ref ScriptInvoker s_OnPeriodicalCheck2Second = new ScriptInvoker();
//	static ref ScriptInvoker s_OnPeriodicalCheck5Second = new ScriptInvoker();
//	static ref ScriptInvoker s_OnPeriodicalCheck60Second = new ScriptInvoker();
//	
//	//***************************//
//	//PUBLIC EVENT MEMBER METHODS//
//	//***************************//
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a player disconnects
//	//! Server only
//	void OnPlayerDisconnected(int id)
//	{
//#ifdef ENABLE_DIAG
//		//if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
//			//return;
//#endif
//		
//		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.GetTaskExecutorByID(id);
//		SCR_BaseTask task = null;
//		if (executor)
//			task = executor.GetAssignedTask();
//		
//		SCR_BaseTaskSupportEntity supportEntity = FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		if (task)
//			supportEntity.UnassignTask(task, executor, SCR_EUnassignReason.ASSIGNEE_DISCONNECT);
//		
//		task = FindRequestedTask(executor);
//		
//		if (!task)
//			return;
//		
//		supportEntity.CancelTask(task.GetTaskID());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a task is deleted.
//	void OnTaskDeleted(SCR_Task task)
//	{
//		if (m_aTaskList.Contains(task))
//			m_aTaskList.RemoveItem(task);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a new task is created.
//	void OnTaskCreated(SCR_Task task)
//	{
//		s_OnTaskFactionAssigned.Insert(RefreshTaskList);
//		
//		SCR_EditorTask editorTask = SCR_EditorTask.Cast(task);
//		if (!editorTask)
//			return;
//		
//		SCR_EditableDescriptorComponent editableDescriptor = SCR_EditableDescriptorComponent.Cast(editorTask.FindComponent(SCR_EditableDescriptorComponent));
//		if (!editableDescriptor)
//			return;
//		
//		editableDescriptor.GetOnChange().Insert(RefreshTaskListCommentComponent);
//		editableDescriptor.GetOnUIRefresh().Insert(RefreshTaskListVoid);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	private void RefreshTaskListCommentComponent(SCR_EditableCommentComponent editableCommentComponent)
//	{
//		RefreshTaskList();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	private void RefreshTaskListVoid()
//	{
//		RefreshTaskList();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Refresh task list.
//	[Obsolete("Use new SCR_TaskManagerUIComponent methods instead")]
//	void RefreshTaskList(SCR_BaseTask task = null)
//	{
//		s_OnTaskFactionAssigned.Remove(RefreshTaskList);
//		
//		SCR_TaskManagerUIComponent UItaskManager = SCR_TaskManagerUIComponent.GetInstance();
//		if (!UItaskManager || !UItaskManager.IsTaskListOpen())
//			return;
//		
//		if (task)
//		{
//			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
//			if (!factionManager)
//				return;
//
//			Faction taskTargetFaction = task.GetTargetFaction();
//			Faction playerFaction = factionManager.GetLocalPlayerFaction();	
//			if (taskTargetFaction != playerFaction)
//				return;
//		}
//		
//		/*
//		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
//		if (!mapEntity)
//			return;
//		
//		SCR_MapTaskListUI taskListUI = SCR_MapTaskListUI.Cast(mapEntity.GetMapUIComponent(SCR_MapTaskListUI));
//		if (taskListUI)
//			taskListUI.RefreshTaskList();
//		*/
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a task is finished.
//	void OnTaskFinished(SCR_Task task)
//	{
//		if (!m_aFinishedTaskList.Contains(task))
//			m_aFinishedTaskList.Insert(task);
//		
//		OnTaskDeleted(task);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called whenever any task has been changed, updated, created.
//	void OnTaskUpdate(SCR_Task task, SCR_ETaskEventMask mask)
//	{
//		if (m_mTaskEventMaskMap.Contains(task))
//		{
//			SCR_ETaskEventMask tempMask;
//			tempMask = m_mTaskEventMaskMap.Get(task); // returns mask for the task
//			tempMask = tempMask | mask;
//			m_mTaskEventMaskMap.Set(task, tempMask);
//		}
//		else
//		{
//			m_mTaskEventMaskMap.Insert(task, mask);
//		}
//	}
//	
//	//************************//
//	//PROTECTED MEMBER METHODS//
//	//************************//
//	
//	//------------------------------------------------------------------------------------------------
//	protected void PeriodicalCheck2Second()
//	{
//		s_OnPeriodicalCheck2Second.Invoke();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void PeriodicalCheck5Second()
//	{
//		s_OnPeriodicalCheck5Second.Invoke();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void PeriodicalCheck60Second()
//	{
//		s_OnPeriodicalCheck60Second.Invoke();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected SCR_EvacuateTask CreateNewEvacuationTask(int requesterID)
//	{
//		SCR_BaseTaskSupportEntity supportClass = GetTaskManager().FindSupportEntity(SCR_EvacuateTaskSupportEntity);
//		if (!supportClass)
//			return null;
//		
//		SCR_EvacuateTask evacuateTask = SCR_EvacuateTask.Cast(GetGame().SpawnEntityPrefab(supportClass.GetTaskPrefab(), GetWorld()));
//		if (!evacuateTask)
//			return null;
//		
//		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
//		
//		evacuateTask.SetRequester(requester);
//		
//		return evacuateTask;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Checks every task for assignee timeout
//	protected void CheckAssigneeTimeout()
//	{
//		foreach (SCR_BaseTask task : m_aTaskList)
//		{
//			if (task && task.IsIndividual())
//				task.CheckAssigneeTimeout();
//		}
//	}
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	string GetReassignText()
//	{
//		return m_sReassignPopup;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void AddAssigneeAbandoned(SCR_BaseTaskExecutor assignee)
//	{
//		if (m_mAssigneesAbandoned)
//			m_mAssigneesAbandoned.Set(assignee, m_fTimestamp + DEFAULT_ABANDON_WAITING_TIME);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Checks if this entity is locally owned
//	//! Public because it's also used by tasks as they don't have any RplComponents themselves
//	bool IsProxy()
//	{
//		return (m_RplComponent && m_RplComponent.IsProxy());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void UnregisterSupportEntity(SCR_BaseTaskSupportEntity supportEntity)
//	{
//		m_aSupportedTaskTypes.RemoveItem(supportEntity);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void RegisterSupportEntity(SCR_BaseTaskSupportEntity supportEntity)
//	{
//		m_aSupportedTaskTypes.Insert(supportEntity);
//		supportEntity.Initialize();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the found task if it exists.
//	SCR_BaseTask GetTask(int taskID)
//	{
//		if (taskID == -1)
//			return null;
//		
//		foreach (SCR_BaseTask task : m_aTaskList)
//		{
//			if (task.GetTaskID() == taskID)
//				return task;
//		}
//		
//		foreach (SCR_BaseTask finishedTask : m_aFinishedTaskList)
//		{
//			if (finishedTask.GetTaskID() == taskID)
//				return finishedTask;
//		}
//		
//		return null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Registers Tasks and based on their State it inserts them to proper list.
//	void RegisterTask(SCR_BaseTask task)
//	{
//		if (task.GetTaskState() == SCR_TaskState.OPENED && !m_aTaskList.Contains(task))
//			m_aTaskList.Insert(task);
//			
//		if (task.GetTaskState() == SCR_TaskState.FINISHED && !m_aFinishedTaskList.Contains(task))
//		{
//			m_aFinishedTaskList.Insert(task);
//			m_aTaskList.RemoveItem(task);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the found finished task if it exists.
//	SCR_BaseTask GetFinishedTask(int taskID)
//	{
//		if (taskID == -1)
//			return null;
//		
//		foreach (SCR_BaseTask task : m_aFinishedTaskList)
//		{
//			if (task.GetTaskID() == taskID)
//			{
//				return task;
//			}
//		}
//		
//		return null;
//	}
//	
//	#ifdef ENABLE_DIAG
//	//------------------------------------------------------------------------------------------------
//	bool CheckMasterOnlyMethod(string methodName)
//	{
//		if (IsProxy())
//		{
//		    Print("Master-only method (SCR_BaseTaskManager." + methodName + ") called on proxy, some functionality might be broken!", LogLevel.WARNING);
//			return false;
//		}
//		
//		return true;
//	}
//	#endif
//	
//	//------------------------------------------------------------------------------------------------
//	bool HasRequestedTask(int playerID)
//	{
//		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		if (!executor)
//			return true;
//		
//		return GetTaskManager().FindRequestedTask(executor) != null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool Initialized()
//	{
//		return m_bInitialized;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void DeleteTask(SCR_BaseTask task)
//	{
//		if (m_aTasksToDelete.Contains(task))
//			return;
//		
//		m_aTasksToDelete.Insert(task);
//		task.OnDelete();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_BaseTaskSupportEntity FindSupportEntity(typename type)
//	{
//		if (!m_aSupportedTaskTypes)
//			return null;
//		
//		for (int i = m_aSupportedTaskTypes.Count() - 1; i >= 0; i--)
//		{
//			if (type == m_aSupportedTaskTypes[i].Type())
//				return m_aSupportedTaskTypes[i];
//		}
//		
//		return null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//Returns the list of all supported task types
//	//Used f.e. for request buttons in task list, which is temporarily disabled
//	array<SCR_BaseTaskSupportEntity> GetSupportedTasks()
//	{
//		return m_aSupportedTaskTypes;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	notnull SCR_BaseTaskExecutor LocalCreateTaskExecutor(int playerID)
//	{
//		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.FindTaskExecutorByID(playerID);
//		if (executor)
//			return executor;
//		
//		executor = SCR_BaseTaskExecutor.Cast(GetGame().SpawnEntity(SCR_BaseTaskExecutor, GetGame().GetWorld()));
//		executor.SetPlayerID(playerID);
//		
//		return executor;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_RequestedTask FindRequestedTask(SCR_BaseTaskExecutor requester)
//	{
//		for (int i = 0; i < m_aTaskList.Count(); i++)
//		{
//			SCR_RequestedTask requestedTask = SCR_RequestedTask.Cast(m_aTaskList[i]);
//			if (!requestedTask)
//				continue;
//			
//			SCR_BaseTaskExecutor taskRequester = requestedTask.GetRequester();
//			if (requester == taskRequester)
//				return requestedTask;
//		}
//		
//		return null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Outs an array of tasks filtered by faction.
//	//! filterFaction param is the allowed faction!
//	int GetFilteredTasks(notnull out array<SCR_BaseTask> tasks, Faction filterFaction = null)
//	{
//		int count = 0;
//		tasks.Clear();
//		
//		if (!filterFaction)
//			return GetTasks(tasks);
//		
//		Faction taskFaction;
//		foreach (SCR_BaseTask task : m_aTaskList)
//		{
//			if (!task) //--- TODO: this runs before the task is deleted, should not
//				continue;
//			
//			taskFaction = task.GetTargetFaction();
//			if (!taskFaction || taskFaction == filterFaction)
//			{
//				tasks.Insert(task);
//				count++;
//			}
//		}
//		
//		return count;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Outs an array of tasks filtered by faction.
//	//! filterFaction param is the allowed faction!
//	int GetFilteredFinishedTasks(notnull out array<SCR_BaseTask> tasks, Faction filterFaction = null)
//	{
//		int count = 0;
//		tasks.Clear();
//		
//		if (!filterFaction)
//			return GetTasks(tasks);
//		
//		Faction taskFaction;
//		foreach (SCR_BaseTask task : m_aFinishedTaskList)
//		{
//			taskFaction = task.GetTargetFaction();
//			if (!taskFaction || taskFaction == filterFaction)
//			{
//				tasks.Insert(task);
//				count++;
//			}
//		}
//		
//		return count;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool GetAssigneeAbandoned(SCR_BaseTaskExecutor assignee)
//	{
//		if (!m_mAssigneesAbandoned || !assignee)
//			return false;
//		
//		float timestamp = m_mAssigneesAbandoned.Get(assignee);
//		
//		if (m_fTimestamp < timestamp)
//			return true;
//		
//		return false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void AbandonTask(int taskID, notnull SCR_BaseTaskExecutor assignee)
//	{
//		#ifdef ENABLE_DIAG
//		if (!CheckMasterOnlyMethod("AbandonTask()"))
//			return;
//		#endif
//		
//		SCR_BaseTask task = GetTask(taskID);
//		
//		if (!task)
//			return;
//		
//		if (task.NotifyUnassign())
//			task.DoNotifyUnassign(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee));
//		
//		SCR_BaseTaskSupportEntity supportEntity = FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		supportEntity.UnassignTask(task, assignee, SCR_EUnassignReason.ASSIGNEE_ABANDON);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Used for assigning the task with given task ID to the assignee in parameter
//	void RequestAssignment(int taskID, notnull SCR_BaseTaskExecutor assignee)
//	{
//		if (!assignee || IsProxy())
//			return;
//		
//		SCR_BaseTask task = GetTask(taskID);
//		
//		if (!task)
//			return;
//		
//		if (task.NotifyAssignment())
//			task.DoNotifyAssignment(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee));
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		supportEntity.AssignTask(task, assignee);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnPlayerRegistered(int registeredPlayerID)
//	{
//		if (registeredPlayerID == SCR_PlayerController.GetLocalPlayerId())
//			CreateTaskExecutors();
//		else
//			LocalCreateTaskExecutor(registeredPlayerID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Creates a task executor for each connected player
//	void CreateTaskExecutors()
//	{
//		array<int> players = new array<int>();
//		GetGame().GetPlayerManager().GetPlayers(players);
//		
//		foreach (int playerID : players)
//		{
//			if (SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID))
//				continue;
//			
//			LocalCreateTaskExecutor(playerID);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the current timestamp
//	float GetTimestamp()
//	{
//		return m_fTimestamp;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Outs an array of all of the tasks.
//	int GetTasks(notnull out array<SCR_BaseTask> tasks)
//	{
//		int count = 0;
//		
//		foreach (SCR_BaseTask task : m_aTaskList)
//		{
//			tasks.Insert(task);
//			count++;
//		}
//		
//		return count;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Outs an array of all of the finished tasks.
//	int GetFinishedTasks(notnull out array<SCR_BaseTask> tasks)
//	{
//		int count = 0;
//		
//		foreach (SCR_BaseTask task : m_aFinishedTaskList)
//		{
//			tasks.Insert(task);
//			count++;
//		}
//		
//		return count;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Called from SpawnTask and RPC_SpawnTask methods
//	//! Don't call directly!
//	SCR_BaseTask LocalSpawnTask(ResourceName resourceName)
//	{
//		if (resourceName.IsEmpty())
//			return null;
//		
//		Resource resource = Resource.Load(resourceName);
//		if (!resource.IsValid())
//			return null;
//		
//		return SCR_BaseTask.Cast(GetGame().SpawnEntityPrefab(resource, GetWorld()));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Call this on the server only
//	SCR_BaseTask SpawnTask(ResourceName resourceName)
//	{
//		SCR_BaseTask task = LocalSpawnTask(resourceName);
//		if (!task)
//			return null;
//		
//		Rpc(RPC_SpawnTask, resourceName, task.GetTaskID());
//		
//		return task;
//	}
//	
//	//***********//
//	//RPC METHODS//
//	//***********//
//	
//	//------------------------------------------------------------------------------------------------
//	//! Called by SpawnTask()
//	//! Don't call directly!
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	protected void RPC_SpawnTask(ResourceName resourceName, int taskID)
//	{
//		SCR_BaseTask task = LocalSpawnTask(resourceName);
//		if (!task)
//			return;
//		
//		task.SetTaskID(taskID);
//	}
//	
//	//*************//
//	//ENTITY EVENTS//
//	//*************//
//
//	//------------------------------------------------------------------------------------------------
//	protected override void EOnInit(IEntity owner)
//	{
//		m_bInitialized = true;
//		
//		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//		if (gameMode)
//			gameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
//		
//		// If replicated make sure it never streams out
//		auto rplComponent = RplComponent.Cast(FindComponent(RplComponent));
//		if (rplComponent && rplComponent.IsSelfInserted())
//		{
//			auto ctx = new GameRplSchedulerInsertionCtx;
//			ctx.CanBeStreamed = false;
//			rplComponent.InsertToReplication(ctx);
//		}
//		
//		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
//		
//		// This is for individual tasks, so they don't have to tick their own timers, but it's centralized on the task manager.
//		// This could be further improved for more custom timers, but for now it is sufficient as is.
//		GetGame().GetCallqueue().CallLater(PeriodicalCheck2Second, 2000, true);
//		GetGame().GetCallqueue().CallLater(PeriodicalCheck5Second, 5000, true);
//		GetGame().GetCallqueue().CallLater(PeriodicalCheck60Second, 60000, true);
//	}
//	//------------------------------------------------------------------------------------------------
//	void UpdateTasks()
//	{
//		SCR_BaseTask task;
//		SCR_ETaskEventMask mask;
//		
//		for (int i = m_mTaskEventMaskMap.Count() - 1; i >= 0; i--)
//		{
//			task = m_mTaskEventMaskMap.GetKey(i);
//			mask = m_mTaskEventMaskMap.Get(task);
//			s_OnTaskUpdate.Invoke(task, mask);
//		}
//		
//		m_mTaskEventMaskMap.Clear();
//		
//		foreach (SCR_BaseTask taskToDelete : m_aTasksToDelete)
//		{
//			if (!taskToDelete)
//				continue;
//			
//			IEntity taskChild = taskToDelete.GetChildren();
//			while (taskChild)
//			{
//				IEntity childToDelete = taskChild;
//				taskChild = taskChild.GetSibling();
//
//				delete childToDelete;
//			}
//			
//			m_aTaskList.RemoveItem(taskToDelete);
//			delete taskToDelete;
//		}
//		
//		for (int i = m_aFinishedTaskList.Count() - 1; i >= 0; i--)
//		{
//			if (!m_aFinishedTaskList[i])
//				m_aFinishedTaskList.Remove(i);
//		}
//		
//		m_aTasksToDelete.Clear();
//	}
//	//------------------------------------------------------------------------------------------------
//	protected override void EOnFrame(IEntity owner, float timeSlice)
//	{
//		if (GetTaskManager() && GetTaskManager() != this)
//		{
//			delete this;
//			return;
//		}
//		
//		if (!m_mTaskEventMaskMap.IsEmpty())
//			UpdateTasks();
//#ifdef ENABLE_DIAG
//		/*if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SHOW_ALL_TASKS))
//		{
//			if (!m_bPrintedTasks)
//			{
//				m_bPrintedTasks = true;
//				for (int i = m_aTaskList.Count() - 1; i >= 0; i--)
//				{
//					Print(m_aTaskList[i].GetDescription());
//				}
//			}
//		}
//		else
//			m_bPrintedTasks = false;
//		
//		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
//			return;*/
//#endif
//		
//		m_fTimestamp += timeSlice;
//		m_fTimestampTimer -= timeSlice;
//		if (m_fTimestampTimer <= 0)
//		{
//			Replication.BumpMe();
//			m_fTimestampTimer = 1;
//		}
//		
//		if (!IsProxy() && m_fTimestamp > m_fAssigneeCacheCheckTimestamp)
//		{
//			m_fAssigneeCacheCheckTimestamp = m_fTimestamp + m_fAssigneeCacheTimer;
//			CheckAssigneeTimeout();
//		}
//	}
//	
//	//***********//
//	//JIP METHODS//
//	//***********//
//	
//	//------------------------------------------------------------------------------------------------
//	void SaveTasksForRpl(ScriptBitWriter writer, array<SCR_BaseTask> taskArray)
//	{
//		int count = taskArray.Count();
//		writer.WriteInt(count);
//		
//		for (int i = 0; i < count; i++)
//		{
//			SCR_BaseTask task = taskArray[i];
//			if (!task || task.FindComponent(RplComponent)) //--- Assume that tasks with their own replication will sync data themselves
//			{
//				writer.WriteBool(false);
//				continue;
//			}
//			
//			EntityPrefabData prefabData = task.GetPrefabData();
//			ResourceName resourceName;
//			if (prefabData)
//				resourceName = prefabData.GetPrefabName();
//			
//			Resource resource = Resource.Load(resourceName);
//			if (!resource.IsValid())
//			{
//				// Invalid resouce name means we wouldn't be able to recreate the task on clients.
//				// As a result, we would start reading from incorrect place in the serialized buffer
//				// which is undefined behavior and could easily end-up crashing the game.
//				writer.WriteBool(false);
//				continue;
//			}
//			
//			writer.WriteBool(true);
//			
//			writer.WriteResourceName(resourceName); //Write prefab, then read it in load & spawn correct task
//			
//			task.Serialize(writer);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool RplSave(ScriptBitWriter writer)
//	{
//		
//		SaveTasksForRpl(writer, m_aTaskList);
//		SaveTasksForRpl(writer, m_aFinishedTaskList);
//		
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool LoadTasksForRpl(ScriptBitReader reader, int count)
//	{
//		reader.ReadInt(count);
//		
//		ResourceName resourceName;
//		Resource resource;
//		SCR_BaseTask task;
//		bool readTask;
//		for (int i = 0; i < count; i++)
//		{
//			reader.ReadBool(readTask);
//			if (!readTask)
//				continue;
//			
//			reader.ReadResourceName(resourceName);
//			
//			resource = Resource.Load(resourceName);
//			if (!resource.IsValid())
//			{
//				// https://jira.bistudio.com/browse/ARMA4-62494
//				// We have to return. This is a fatal failure and because these
//				// tasks are not replicated by themselves the task that follows
//				// wouldn't know where to read its data from. It would simply
//				// read from wherewhere we left form and most likely cause a crash
//				// during deserialization.
//				Print("Invalid resource name for a task. Tasks can't be synchronized properly!", LogLevel.ERROR);
//				return false; // DON'T CONTINUE
//			}
//			
//			task = SCR_BaseTask.Cast(GetGame().SpawnEntityPrefab(resource, GetWorld()));
//			if (!task)
//			{
//				// https://jira.bistudio.com/browse/ARMA4-62494
//				// We have to return. This is a fatal failure and because these
//				// tasks are not replicated by themselves the task that follows
//				// wouldn't know where to read its data from. It would simply
//				// read from wherewhere we left form and most likely cause a crash
//				// during deserialization.
//				Print(string.Format("Unable to spawn a task for resource name %1. Some tasks can't synchronized!", resourceName), LogLevel.ERROR);
//				return false; // DON'T CONTINUE
//			}
//			
//			task.Deserialize(reader);
//		}
//
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool RplLoad(ScriptBitReader reader)
//	{
//		// https://jira.bistudio.com/browse/ARMA4-62494
//		// We keep reading only as long as valid tasks are present.
//		// The first broken task would break deserialization and cause crashes.
//		// Therefore, we won't even attempt reading further if an issue is detected
//		// and we'll try to intialize at least the tasks we managed to extract.
//		const int count; // TODO: check for good const usage
//		if (LoadTasksForRpl(reader, count))
//			LoadTasksForRpl(reader, count);
//		
//		//Let the game mode know, the tasks are ready
//		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//		if (gamemode)
//			gamemode.HandleOnTasksInitialized();
//		
//		//AssignCachedTasks();
//		
//		return true;
//	}
//	
//	//************************//
//	//CONSTRUCTOR & DESTRUCTOR//
//	//************************//
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_BaseTaskManager(IEntitySource src, IEntity parent)
//	{
//		if (!g_TaskManagerInstance)
//			g_TaskManagerInstance = this;
//		
//#ifdef ENABLE_DIAG
//		/*DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_TASKS_MENU, "Tasks", "");
//		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS, "", "Execute tasks", "Tasks", true);
//		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SHOW_ALL_TASKS, "", "Print all tasks", "Tasks", false);*/
//#endif
//		SetEventMask(EntityEvent.FRAME | EntityEvent.INIT);
//		SetFlags(EntityFlags.NO_LINK, false);
//		
//		//Register to Script Invokers
//		s_OnTaskCreated.Insert(OnTaskCreated);
//		s_OnTaskFinished.Insert(OnTaskFinished);
//		s_OnTaskDeleted.Insert(OnTaskDeleted);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_BaseTaskManager()
//	{
//		
//		//Unregister from Script Invokers
//		s_OnTaskCreated.Remove(OnTaskCreated);
//		s_OnTaskFinished.Remove(OnTaskFinished);
//		s_OnTaskDeleted.Remove(OnTaskDeleted);
//	}
//};
