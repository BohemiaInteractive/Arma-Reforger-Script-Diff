class SCR_ExtendedTaskClass : SCR_TaskClass
{
}

void ChildTaskInvokerDelegate(SCR_Task task);
typedef func ChildTaskInvokerDelegate;
typedef ScriptInvokerBase<ChildTaskInvokerDelegate> SCR_ChildTaskInvoker;

class SCR_ExtendedTask : SCR_Task 
{
	[Attribute("1")]
	protected bool m_bLinkDataToRelatedTasks;
	
	[Attribute("0")]
	protected bool m_bEnableProgress;
	
	[Attribute("0.0", UIWidgets.Slider, "Progression of the task", "0.0 100.0 1.0")]
	protected float m_fProgress;
	
	[Attribute("#AR-Tasks_TaskCompletionConditions")]
	protected LocalizedString m_sCustomSubtaskDescription;
	
	protected SCR_ETaskState m_eCachedTaskState; //Used for task progression
	
	protected ref SCR_ChildTaskInvoker m_OnChildTaskAdded;
	protected ref SCR_ChildTaskInvoker m_OnChildTaskRemoved;
	
	protected ref ScriptInvokerFloat2 m_OnProgressChanged;
	
	protected const float MIN_TASK_PROGRESSION_PERCENTAGE = 0.0;
	protected const float MAX_TASK_PROGRESSION_PERCENTAGE = 100.0;
	
	//------------------------------------------------------------------------------------------------
	//! Returns custom description string for subtasks
	//! \return
	LocalizedString GetCustomSubtaskDescription()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);	
		if (!extendedData)
			return string.Empty;
		
		return extendedData.m_sCustomSubtaskDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets custom description string for subtasks
	//! \return
	void SetCustomSubtaskDescription(LocalizedString subTaskDescription)
	{
		Rpc_SetCustomSubtaskDescription(subTaskDescription);
		Rpc(Rpc_SetCustomSubtaskDescription, subTaskDescription);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetCustomSubtaskDescription(LocalizedString subTaskDescription)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);	
		if (!extendedData)
			return;
		
		extendedData.m_sCustomSubtaskDescription = subTaskDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if this task is parented to another
	//! \return
	bool IsChildTask()
	{
		return GetParentTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if other tasks are parented to this one
	//! \return
	bool HasChildTasks()
	{		
		array<SCR_Task> childTasks = GetChildTasks();		
		return childTasks && !childTasks.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns exact number of tasks parented to this one
	//! \return
	int GetChildTaskCount()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);	
		if (!extendedData)
			return 0;
		
		return extendedData.m_aChildTasks.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if this task is assignable to entities; Will always return false if it is a parent task
	//! \return
	bool CanBeAssigned()
	{		
		return !HasChildTasks();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns depth within the task hierarchy; Parent task is always 0, while child tasks are 1
	//! \return
	int GetNodeDepth()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);	
		if (!extendedData)
			return -1;
		
		return extendedData.m_iNodeDepth;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns parent task or null if there is none
	//! \return
	SCR_Task GetParentTask()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || !extendedData.m_ParentTask)
			return null;

		RplComponent rpl = RplComponent.Cast(Replication.FindItem(extendedData.m_ParentTask));
		if (!rpl)
			return null;
		
		return SCR_Task.Cast(rpl.GetEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetParentTask(SCR_ExtendedTask parentTask)
	{
		RplComponent rpl = RplComponent.Cast(parentTask.FindComponent(RplComponent));
		if (!rpl)
			return;
		
		RplId rplID = rpl.Id();
		if (!rplID)
			return;
		
		Rpc_SetParentTask(rplID);
		Rpc(Rpc_SetParentTask, rplID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetParentTask(RplId parentTaskRplID)
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(parentTaskRplID));
		if (!rpl)
			return;
		
		SCR_ExtendedTask extendedParentTask = SCR_ExtendedTask.Cast(rpl.GetEntity());
		if (!extendedParentTask)
			return;
		
		SCR_ExtendedTaskData parentTaskData = SCR_ExtendedTaskData.Cast(extendedParentTask.GetTaskData());
		if (!parentTaskData)
			return;
		
		//Merge parent data with child data; Unique identifiers such as taskID will remain the same while states and assignees will be copied to this task from parent
		if (parentTaskData.m_bLinkDataToRelatedTasks)
			m_TaskData = parentTaskData.Merge(SCR_ExtendedTaskData.Cast(m_TaskData));
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		extendedData.m_ParentTask = parentTaskRplID;
		extendedData.m_iNodeDepth = extendedParentTask.GetNodeDepth() + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns array of tasks parented to this one
	//! \return
	array<SCR_Task> GetChildTasks()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);	
		if (!extendedData || !extendedData.m_aChildTasks)
			return null;
		
		RplComponent rpl;
		SCR_Task task;
		
		array<SCR_Task> childTasks = {};
		foreach (RplId rplID : extendedData.m_aChildTasks)
		{
			rpl = RplComponent.Cast(Replication.FindItem(rplID));
			if (!rpl)
				continue;
			
			task = SCR_Task.Cast(rpl.GetEntity());
			if (!task)
				continue;
			
			childTasks.Insert(task);
		}
		
		return childTasks;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to parent task to this one
	//! \param[in] task
	//! \return
	bool AddChildTask(SCR_Task task)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
			return false;
		
		RplComponent rpl = RplComponent.Cast(extendedTask.FindComponent(RplComponent));
		if (!rpl)
			return false;
		
		RplId rplID = rpl.Id();
		if (!rplID)
			return false;
		
		extendedTask.SetParentTask(this);
		
		Rpc_AddChildTask(rplID);
		Rpc(Rpc_AddChildTask, rplID);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddChildTask(RplId taskRplID)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || !taskRplID)
			return;
		
		if (!extendedData.m_aChildTasks)
			extendedData.m_aChildTasks = new array<RplId>();
		
		if (extendedData.m_aChildTasks.Contains(taskRplID))
			return;
		
		extendedData.m_aChildTasks.Insert(taskRplID);
		
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(taskRplID));
		if (!rpl)
			return;
			
		SCR_Task task = SCR_Task.Cast(rpl.GetEntity());
		if (!task)
			return;
		
		if (m_OnChildTaskAdded)
			m_OnChildTaskAdded.Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to unparent task from this one
	//! \param[in] task
	//! \return
	bool RemoveChildTask(SCR_Task task)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (!extendedTask)
			return false;
		
		RplComponent rpl = RplComponent.Cast(extendedTask.FindComponent(RplComponent));
		if (!rpl)
			return false;
		
		RplId rplID = rpl.Id();
		if (!rplID)
			return false;
		
		array<ref SCR_TaskExecutor> assignees = extendedTask.GetTaskAssignees();
		if (assignees && !assignees.IsEmpty())
		{
			foreach (SCR_TaskExecutor assignee : assignees)
			{
				extendedTask.RemoveTaskAssignee(assignee);
			}
		}
		
		Rpc_RemoveChildTask(rplID);
		Rpc(Rpc_RemoveChildTask, rplID);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveChildTask(RplId taskRplID)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || !taskRplID)
			return;
		
		if (!extendedData.m_aChildTasks || !extendedData.m_aChildTasks.Contains(taskRplID))
			return;
		
		extendedData.m_aChildTasks.RemoveItem(taskRplID);
		
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(taskRplID));
		if (!rpl)
			return;
			
		SCR_Task task = SCR_Task.Cast(rpl.GetEntity());
		if (!task)
			return;
		
		if (m_OnChildTaskRemoved)
			m_OnChildTaskRemoved.Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns progress of task as float; Value ranges from 0-100
	//! \return
	float GetTaskProgress()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return -1;
		
		return extendedData.m_fProgress;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets progress of task in percentage
	//! \param[in] percentage
	//! \return
	void SetTaskProgress(float percentage, bool changeState = true)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || percentage == extendedData.m_fProgress)
			return;
		
		if (percentage < MIN_TASK_PROGRESSION_PERCENTAGE)
			percentage = MIN_TASK_PROGRESSION_PERCENTAGE;
		else
		if (percentage > MAX_TASK_PROGRESSION_PERCENTAGE)
			percentage = MAX_TASK_PROGRESSION_PERCENTAGE;
		
		float previousProgress = extendedData.m_fProgress;
		
		Rpc_SetTaskProgress(percentage, previousProgress);
		Rpc(Rpc_SetTaskProgress, percentage, previousProgress);
		
		if (changeState)
		{
			if (extendedData.m_fProgress >= previousProgress)
			{
				if (extendedData.m_fProgress >= MAX_TASK_PROGRESSION_PERCENTAGE && extendedData.m_eState != SCR_ETaskState.COMPLETED)
				{
					m_eCachedTaskState = extendedData.m_eState;
					SetTaskState(SCR_ETaskState.COMPLETED);
				}
				else if (extendedData.m_fProgress > MIN_TASK_PROGRESSION_PERCENTAGE && extendedData.m_eState != SCR_ETaskState.PROGRESSED)
				{
					m_eCachedTaskState = extendedData.m_eState;
					SetTaskState(SCR_ETaskState.PROGRESSED);
				}
			}
			else
			{
				if (extendedData.m_fProgress <= MIN_TASK_PROGRESSION_PERCENTAGE)
				{
					SetTaskState(m_eCachedTaskState);
				}
				else if (extendedData.m_fProgress < MAX_TASK_PROGRESSION_PERCENTAGE && extendedData.m_eState != SCR_ETaskState.PROGRESSED)
				{
					SetTaskState(SCR_ETaskState.PROGRESSED);
				}
			}
		}
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		SCR_ExtendedTask parent = SCR_ExtendedTask.Cast(GetParentTask());
		if (!parent)
			return;
		
		int childTaskCount = parent.GetChildTaskCount();
		
		previousProgress /= childTaskCount;
		parent.RemoveTaskProgress(previousProgress, changeState);
		
		percentage /= childTaskCount;
		parent.AddTaskProgress(percentage, changeState);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskProgress(float percentage, float previousProgress)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		extendedData.m_fProgress = percentage;
		
		if (m_OnProgressChanged)
			m_OnProgressChanged.Invoke(previousProgress, percentage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds progress to task in percentage
	//! \param[in] percentage
	//! \return
	void AddTaskProgress(float percentage, bool changeState = true)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || percentage <= 0)
			return;
		
		if (percentage > MAX_TASK_PROGRESSION_PERCENTAGE - extendedData.m_fProgress)
			percentage = MAX_TASK_PROGRESSION_PERCENTAGE - extendedData.m_fProgress;
		
		Rpc_AddTaskProgress(percentage);
		Rpc(Rpc_AddTaskProgress, percentage);
		
		if (changeState)
		{
			if (extendedData.m_fProgress >= MAX_TASK_PROGRESSION_PERCENTAGE && extendedData.m_eState != SCR_ETaskState.COMPLETED)
			{
				m_eCachedTaskState = extendedData.m_eState;
				SetTaskState(SCR_ETaskState.COMPLETED);
			}
			else if (extendedData.m_fProgress > MIN_TASK_PROGRESSION_PERCENTAGE && extendedData.m_eState != SCR_ETaskState.PROGRESSED)
			{
				m_eCachedTaskState = extendedData.m_eState;
				SetTaskState(SCR_ETaskState.PROGRESSED);
			}
		}
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		SCR_ExtendedTask parent = SCR_ExtendedTask.Cast(GetParentTask());
		if (!parent)
			return;
		
		percentage /= parent.GetChildTaskCount();
		parent.AddTaskProgress(percentage, changeState);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddTaskProgress(float percentage)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || percentage <= 0)
			return;
		
		extendedData.m_fProgress += percentage;
		
		if (m_OnProgressChanged)
			m_OnProgressChanged.Invoke(extendedData.m_fProgress - percentage, extendedData.m_fProgress);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes progress from task in percentage
	//! \param[in] percentage
	//! \return
	void RemoveTaskProgress(float percentage, bool changeState = true)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || percentage <= 0)
			return;
		
		if (percentage > extendedData.m_fProgress)
			percentage = extendedData.m_fProgress;
		
		Rpc_RemoveTaskProgress(percentage);
		Rpc(Rpc_RemoveTaskProgress, percentage);
		
		if (changeState)
		{
			if (extendedData.m_fProgress <= MIN_TASK_PROGRESSION_PERCENTAGE)
				SetTaskState(m_eCachedTaskState);
			else if (extendedData.m_fProgress < MAX_TASK_PROGRESSION_PERCENTAGE && extendedData.m_eState != SCR_ETaskState.PROGRESSED)
				SetTaskState(SCR_ETaskState.PROGRESSED);	
		}
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		SCR_ExtendedTask parent = SCR_ExtendedTask.Cast(GetParentTask());
		if (!parent)
			return;
		
		percentage /= parent.GetChildTaskCount();
		parent.RemoveTaskProgress(percentage, changeState);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_RemoveTaskProgress(float percentage)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData || percentage <= 0)
			return;
		
		extendedData.m_fProgress -= percentage;
		
		if (m_OnProgressChanged)
			m_OnProgressChanged.Invoke(extendedData.m_fProgress + percentage, extendedData.m_fProgress);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if task progress can be shown
	//! \return
	bool CanTaskProgressBeShown()
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		return extendedData.m_bEnableProgress;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] enable
	//! \return
	void ShowTaskProgress(bool enable)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		Rpc_ShowTaskProgress(enable);
		Rpc(Rpc_ShowTaskProgress, enable);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_ShowTaskProgress(bool enable)
	{
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		extendedData.m_bEnableProgress = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to assign this task to the given entity
	//! If linking data to other tasks is enabled and this task is parented to another, 
	//! the parent task will be assigned to the entity as well
	//! \param[in] executor
	//! \param[in] listenToInvokers
	//! \param[in] requesterID
	//! \return
	override bool AddTaskAssignee(SCR_TaskExecutor executor, bool listenToInvokers = true, int requesterID = 0)
	{
		if (!super.AddTaskAssignee(executor, listenToInvokers, requesterID))
			return false;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		SCR_ExtendedTask parent = SCR_ExtendedTask.Cast(GetParentTask());
		if (parent && !parent.AddTaskAssignee(executor, listenToInvokers, requesterID))
			return false;
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to unassign this task from the given entity
	//! If linking data to other tasks is enabled and this task is parented to another, 
	//! the parent task will be unassigned from the entity as well
	//! \param[in] executor
	//! \param[in] listenToInvokers
	//! \param[in] requesterID
	//! \return
	override bool RemoveTaskAssignee(SCR_TaskExecutor executor, bool listenToInvokers = true, int requesterID = 0)
	{
		if (!super.RemoveTaskAssignee(executor, listenToInvokers, requesterID))
			return false;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		SCR_ExtendedTask parent = SCR_ExtendedTask.Cast(GetParentTask());
		if (parent)
		{
			if (!parent.RemoveTaskAssignee(executor, listenToInvokers, requesterID))
				return false;
			
			return true;
		}
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return true;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.RemoveTaskAssignee(executor, listenToInvokers, requesterID);
		}
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void Rpc_AddOwnerFactionKey(FactionKey factionKey)
	{
		super.Rpc_AddOwnerFactionKey(factionKey);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.AddOwnerFactionKey(factionKey);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void Rpc_RemoveOwnerFactionKey(FactionKey factionKey)
	{
		super.Rpc_RemoveOwnerFactionKey(factionKey);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.RemoveOwnerFactionKey(factionKey);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void Rpc_AddOwnerGroupID(int groupID)
	{
		super.Rpc_AddOwnerGroupID(groupID);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.AddOwnerGroupID(groupID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void Rpc_RemoveOwnerGroupID(int groupID)
	{
		super.Rpc_RemoveOwnerGroupID(groupID);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		SCR_ExtendedTask child
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.RemoveOwnerGroupID(groupID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds given executor to task as an owner/viewer depending on task ownership and visibility
	//! \param[in] executor
	//! \return
	override bool AddOwnerExecutor(SCR_TaskExecutor executor)
	{
		if (!super.AddOwnerExecutor(executor))
			return false;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return true;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return true;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.AddOwnerExecutor(executor);
		}
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes given executor from task as an owner/viewer depending on task ownership and visibility
	//! \param[in] executor
	//! \return
	override bool RemoveOwnerExecutor(SCR_TaskExecutor executor)
	{
		if (!super.RemoveOwnerExecutor(executor))
			return false;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return false;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return true;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return true;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			child.RemoveOwnerExecutor(executor);
		}
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		super.SetTaskState(state);
		
		if (state != SCR_ETaskState.COMPLETED)
			return;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		SCR_ExtendedTask parent = SCR_ExtendedTask.Cast(GetParentTask());
		if (!parent)
			return;
		
		SCR_ExtendedTaskData extendedDataParent = SCR_ExtendedTaskData.Cast(parent.GetTaskData());
		if (!extendedDataParent)
			return;
		
		if (!extendedDataParent.m_bEnableProgress)
			return;
		
		float percentage = 100 / parent.GetChildTaskCount();
		parent.AddTaskProgress(percentage);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void Rpc_SetTaskOwnership(SCR_ETaskOwnership ownership)
	{
		super.Rpc_SetTaskOwnership(ownership);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			if (child.GetTaskOwnership() > ownership)
				child.SetTaskOwnership(ownership);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void Rpc_SetTaskVisibility(SCR_ETaskVisibility visibility)
	{
		super.Rpc_SetTaskVisibility(visibility);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		if (!extendedData.m_bLinkDataToRelatedTasks)
			return;
		
		array<SCR_Task> childTasks = GetChildTasks();
		if (!childTasks || childTasks.IsEmpty())
			return;
		
		SCR_ExtendedTask child;
		foreach (SCR_Task childTask : childTasks)
		{
			child = SCR_ExtendedTask.Cast(childTask);
			if (!child)
				continue;
			
			if (child.GetTaskVisibility() > visibility)
				child.SetTaskVisibility(visibility);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_ChildTaskInvoker GetOnChildTaskAdded()
	{
		if (!m_OnChildTaskAdded)
			m_OnChildTaskAdded = new SCR_ChildTaskInvoker();
		
		return m_OnChildTaskAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_ChildTaskInvoker GetOnChildTaskRemoved()
	{
		if (!m_OnChildTaskRemoved)
			m_OnChildTaskRemoved = new SCR_ChildTaskInvoker();
		
		return m_OnChildTaskRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	ScriptInvokerFloat2 GetOnProgressChanged()
	{
		if (!m_OnProgressChanged)
			m_OnProgressChanged = new ScriptInvokerFloat2();
		
		return m_OnProgressChanged;
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		vector textPosition = GetOrigin() + vector.Up * 0.5;
		m_DebugText = DebugTextWorldSpace.Create(GetWorld(), "ExtendedTask [" + m_sTaskID + "]", DebugTextFlags.CENTER, textPosition[0], textPosition[1], textPosition[2], 15.0, Color.CYAN, Color.BLUE);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return true;
		
		writer.WriteRplId(extendedData.m_ParentTask);
		writer.WriteInt(extendedData.m_iNodeDepth);
		
		writer.WriteBool(extendedData.m_bLinkDataToRelatedTasks);
		
		writer.WriteFloat(extendedData.m_fProgress);
		writer.WriteBool(extendedData.m_bEnableProgress);
		
		writer.WriteString(extendedData.m_sCustomSubtaskDescription);
		
		int childTaskCount = 0;
		if (extendedData.m_aChildTasks)
			childTaskCount = extendedData.m_aChildTasks.Count();
		
		writer.WriteInt(childTaskCount);
		if (childTaskCount > 0)
		{
			foreach (RplId rplID : extendedData.m_aChildTasks)
			{
				writer.WriteRplId(rplID);
			}	
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		if (!m_TaskData)
			m_TaskData = new SCR_ExtendedTaskData();
		
		if (!super.RplLoad(reader))
			return false;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return true;
		
		reader.ReadRplId(extendedData.m_ParentTask);
		reader.ReadInt(extendedData.m_iNodeDepth);
		
		reader.ReadBool(extendedData.m_bLinkDataToRelatedTasks);
		
		reader.ReadFloat(extendedData.m_fProgress);
		reader.ReadBool(extendedData.m_bEnableProgress);
		
		reader.ReadString(extendedData.m_sCustomSubtaskDescription);

		int childTaskCount;
		reader.ReadInt(childTaskCount);
		if (childTaskCount > 0)
		{
			extendedData.m_aChildTasks = {};
			
			for (int i = 0; i < childTaskCount; i++)
			{
				RplId rplID;
				reader.ReadRplId(rplID);
				
				if (rplID.IsValid())
					extendedData.m_aChildTasks.Insert(rplID);
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void InitializeData()
	{
		if (!m_TaskData)
			m_TaskData = new SCR_ExtendedTaskData();
		
		super.InitializeData();
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		extendedData.m_ParentTask = null;
		extendedData.m_iNodeDepth = 0;
	
		extendedData.m_aChildTasks = {};
		
		extendedData.m_bLinkDataToRelatedTasks = m_bLinkDataToRelatedTasks;
		
		extendedData.m_fProgress = m_fProgress;
		extendedData.m_bEnableProgress = m_bEnableProgress;
		
		extendedData.m_sCustomSubtaskDescription = m_sCustomSubtaskDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitializeChildTasks()
	{	
		float combinedProgress = 0;
		
		IEntity child = GetChildren();
		while (child)
		{
			SCR_ExtendedTask childTask = SCR_ExtendedTask.Cast(child);
			if (childTask)
			{
				AddChildTask(childTask);
				
				if (childTask.GetTaskState() == SCR_ETaskState.COMPLETED)
					combinedProgress += MAX_TASK_PROGRESSION_PERCENTAGE;
				else
					combinedProgress += childTask.GetTaskProgress();
			}
			
			child = child.GetSibling();
		}
		
		if (combinedProgress <= 0)
			return;
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(m_TaskData);
		if (!extendedData)
			return;
		
		extendedData.m_fProgress = combinedProgress / GetChildTaskCount();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (Replication.IsClient())
			return;
		
		// Wait until task data has been initialized for all tasks until trying to parent
		GetGame().GetCallqueue().CallLater(InitializeChildTasks, 100);
	}
}
