//------------------------------------------------------------------------------------------------
//[EntityEditorProps(category: "GameScripted/Tasks", description: "Base task support entity.", color: "0 0 255 255")]
//class SCR_BaseTaskSupportEntityClass: GenericEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//class SCR_BaseTaskSupportEntity : GenericEntity
//{
//	[Attribute("{1844F5647E6D772A}UI/layouts/Tasks/TaskListEntry.layout")]
//	protected ResourceName m_sTaskDescriptionWidgetResource;
//	
//	[Attribute()]
//	protected ResourceName m_sTaskPrefab;
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetFaction(notnull SCR_BaseTask task, notnull Faction faction)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		FactionManager factionManager = GetGame().GetFactionManager();
//		if (!factionManager)
//			return;
//		
//		int taskID, factionIndex;
//		taskID = task.GetTaskID();
//		factionIndex = factionManager.GetFactionIndex(faction);
//		
//		Rpc(RPC_SetTargetFaction, taskID, factionIndex);
//		RPC_SetTargetFaction(taskID, factionIndex);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTargetFaction(int taskID, int factionIndex)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		FactionManager factionManager = GetGame().GetFactionManager();
//		if (!factionManager)
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		task.SetTargetFaction(factionManager.GetFactionByIndex(factionIndex));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_BaseTask CreateTask()
//	{
//		if (!GetTaskManager())
//			return null;
//		
//		return GetTaskManager().SpawnTask(GetResourceName()); // Also spawns task on clients
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void Initialize()
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//Called when task list is opened
//	[Obsolete("Use new SCR_TaskManagerUIComponent instead")]
//	void OnTaskListOpen(notnull SCR_TaskManagerUIComponent uiTaskManagerComponent)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	ResourceName GetTaskDescriptionWidgetResource()
//	{
//		return m_sTaskDescriptionWidgetResource;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	ResourceName GetResourceName()
//	{
//		return m_sTaskPrefab;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	Resource GetTaskPrefab()
//	{
//		if (m_sTaskPrefab.IsEmpty())
//			return null;
//		
//		return Resource.Load(m_sTaskPrefab);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void MoveTask(vector newPosition, int taskID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		task.SetOrigin(newPosition);
//		
//		if (GetTaskManager().IsProxy())
//			return;
//		
//		Rpc(RPC_MoveTask, taskID, newPosition);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_MoveTask(int taskID, vector newPosition)
//	{
//		MoveTask(newPosition, taskID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void AssignTask(notnull SCR_BaseTask task, notnull SCR_BaseTaskExecutor assignee, bool forced = false)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		Rpc(RPC_AssignTask, SCR_BaseTaskExecutor.GetTaskExecutorID(assignee), task.GetTaskID(), GetTaskManager().GetTimestamp(), forced);
//		RPC_AssignTask(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee), task.GetTaskID(), GetTaskManager().GetTimestamp(), forced);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Assigns a task on clients.
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_AssignTask(int playerID, int taskID, float timestamp, bool forced)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		if (!assignee)
//			return;
//		
//		task.AddAssignee(assignee, timestamp);
//		SCR_BaseTaskManager.s_OnTaskAssigned.Invoke(task);
//		
//		if (forced && assignee == SCR_BaseTaskExecutor.GetLocalExecutor())
//			SCR_PopUpNotification.GetInstance().PopupMsg(GetTaskManager().m_sAssignPopupGM);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void CancelTask(int taskID)
//	{
//		Rpc(RPC_CancelTask, taskID);
//		RPC_CancelTask(taskID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	protected void RPC_CancelTask(int taskID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		
//		if (!task)
//			return;
//		
//		task.Cancel();
//		GetTaskManager().DeleteTask(task);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void FinishTask(notnull SCR_BaseTask task)
//	{
//		int taskID = task.GetTaskID();
//		Rpc(RPC_FinishTask, taskID);
//		RPC_FinishTask(taskID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	protected void RPC_FinishTask(int taskID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		
//		if (!task || task.GetTaskState() == SCR_TaskState.FINISHED)
//			return;
//		
//		task.Finish();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void FailTask(notnull SCR_BaseTask task)
//	{
//		int taskID = task.GetTaskID();
//		Rpc(RPC_FailTask, taskID);
//		RPC_FailTask(taskID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	protected void RPC_FailTask(int taskID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		
//		if (!task)
//			task = GetTaskManager().GetFinishedTask(taskID);
//		
//		if (!task)
//			return;
//		
//		task.Fail();
//		GetTaskManager().DeleteTask(task);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void UnassignTask(notnull SCR_BaseTask task, notnull SCR_BaseTaskExecutor assignee, SCR_EUnassignReason reason)
//	{
//		int taskID = task.GetTaskID();
//		int executorID = SCR_BaseTaskExecutor.GetTaskExecutorID(assignee);
//		Rpc(RPC_UnassignTask, taskID, executorID, reason);
//		RPC_UnassignTask(taskID, executorID, reason);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Assigns a task on clients.
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_UnassignTask(int taskID, int playerID, SCR_EUnassignReason reason)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		if (!assignee)
//			return;
//		
//		switch (reason)
//		{
//			case SCR_EUnassignReason.ASSIGNEE_TIMEOUT:
//				break;
//			case SCR_EUnassignReason.ASSIGNEE_DISCONNECT:
//				break;
//			case SCR_EUnassignReason.ASSIGNEE_ABANDON:
//			{
//				GetTaskManager().AddAssigneeAbandoned(assignee);
//				break;
//			}
//			case SCR_EUnassignReason.GM_REASSIGN:
//			{
//				if (assignee == SCR_BaseTaskExecutor.GetLocalExecutor())
//					SCR_PopUpNotification.GetInstance().PopupMsg(GetTaskManager().GetReassignText());
//				break;
//			}
//		}
//		
//		task.RemoveAssignee(assignee, reason);
//		SCR_BaseTaskManager.s_OnTaskUnassigned.Invoke(task);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void SetTaskState(notnull SCR_BaseTask task, SCR_TaskState state)
//	{
//		int taskID = task.GetTaskID();
//		RPC_SetTaskState(taskID, state);
//		Rpc(RPC_SetTaskState, taskID, state);
//	}
//		
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTaskState(int taskID, SCR_TaskState state)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		task.SetState(state);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void SetTaskDescription(notnull SCR_BaseTask task, string description)
//	{
//		int taskID = task.GetTaskID();
//		RPC_SetTaskDescription(taskID, description);
//		Rpc(RPC_SetTaskDescription, taskID, description);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTaskDescription(int taskID, string description)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		task.SetDescription(description);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void SetTaskTitle(notnull SCR_BaseTask task, string title)
//	{
//		int taskID = task.GetTaskID();
//		RPC_SetTaskTitle(taskID, title);
//		Rpc(RPC_SetTaskTitle, taskID, title);
//	}
//		
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTaskTitle(int taskID, string title)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		task.SetTitle(title);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void RegisterContextualMenuCallbacks()
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnMapOpen(MapConfiguration config)
//	{
//		RegisterContextualMenuCallbacks();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void EOnInit(IEntity owner)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		GetTaskManager().RegisterSupportEntity(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_BaseTaskSupportEntity(IEntitySource src, IEntity parent)
//	{
//		SetEventMask(EntityEvent.INIT);
//		SCR_MapEntity.GetMapInstance().GetOnMapOpen().Insert(OnMapOpen);
//		
//		if (!GetTaskManager())
//			return;
//		
//		RplComponent rplComponent = RplComponent.Cast(GetTaskManager().FindComponent(RplComponent));
//		if (!rplComponent)
//			return;
//		
//		rplComponent.InsertItem(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_BaseTaskSupportEntity()
//	{
//		if (!GetTaskManager())
//			return;
//		
//		GetTaskManager().UnregisterSupportEntity(this);
//	}
//};