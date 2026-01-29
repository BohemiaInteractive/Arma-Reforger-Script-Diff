class SCR_TaskSystemNetworkComponentClass : ScriptComponentClass
{
}

class SCR_TaskSystemNetworkComponent : ScriptComponent
{
	protected SCR_TaskSystem m_TaskSystem;
	protected PlayerController m_PlayerController;
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	//! Called locally from task UI; If authority, assign task directly. If not, send rpc to server
	//! \param[in] task
	//! \param[in] executor
	//! \param[in] force
	//! \param[in] requesterID
	void AssignTask(SCR_Task task, SCR_TaskExecutor executor, bool force = false, int requesterID = 0)
	{	
		if (!task || !executor)
			return;
		
		if (!m_RplComponent.IsProxy())
		{
			AssignTask_S(task.GetTaskID(), executor, force, requesterID);
			return;
		}
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);	
		if (executorPlayer)
		{
			Rpc(Rpc_AssignTaskPlayer_S, task.GetTaskID(), executorPlayer, force, requesterID);
			return;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{
			Rpc(Rpc_AssignTaskEntity_S, task.GetTaskID(), executorEntity, force, requesterID);
			return;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
			Rpc(Rpc_AssignTaskGroup_S, task.GetTaskID(), executorGroup, force, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignTask_S(string taskID, SCR_TaskExecutor executor, bool force, int requesterID)
	{
		if (!m_TaskSystem)
			return;
		
		SCR_Task task = m_TaskSystem.GetTaskFromTaskID(taskID);
		if (!task)
			return;
		
		m_TaskSystem.AssignTask(task, executor, force, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_AssignTaskPlayer_S(string taskID, SCR_TaskExecutorPlayer executor, bool force, int requesterID)
	{
		AssignTask_S(taskID, executor, force, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_AssignTaskEntity_S(string taskID, SCR_TaskExecutorEntity executor, bool force, int requesterID)
	{
		AssignTask_S(taskID, executor, force, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_AssignTaskGroup_S(string taskID, SCR_TaskExecutorGroup executor, bool force, int requesterID)
	{
		AssignTask_S(taskID, executor, force, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called locally from task UI; If authority, unassign task directly. If not, send rpc to server
	//! \param[in] task
	//! \param[in] executor
	//! \param[in] requesterID
	void UnassignTask(SCR_Task task, SCR_TaskExecutor executor, int requesterID = 0)
	{
		if (!task || !executor)
			return;
		
		if (!m_RplComponent.IsProxy())
		{
			UnassignTask_S(task.GetTaskID(), executor, requesterID);
			return;
		}
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);	
		if (executorPlayer)
		{
			Rpc(Rpc_UnassignTaskPlayer_S, task.GetTaskID(), executorPlayer, requesterID);
			return;
		}
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
		{
			Rpc(Rpc_UnassignTaskEntity_S, task.GetTaskID(), executorEntity, requesterID);
			return;
		}
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
			Rpc(Rpc_UnassignTaskGroup_S, task.GetTaskID(), executorGroup, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnassignTask_S(string taskID, SCR_TaskExecutor executor, int requesterID)
	{
		if (!m_TaskSystem)
			return;
		
		SCR_Task task = m_TaskSystem.GetTaskFromTaskID(taskID);
		if (!task)
			return;
		
		m_TaskSystem.UnassignTask(task, executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_UnassignTaskPlayer_S(string taskID, SCR_TaskExecutorPlayer executor, int requesterID)
	{
		UnassignTask_S(taskID, executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_UnassignTaskEntity_S(string taskID, SCR_TaskExecutorEntity executor, int requesterID)
	{
		UnassignTask_S(taskID, executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_UnassignTaskGroup_S(string taskID, SCR_TaskExecutorGroup executor, int requesterID)
	{
		UnassignTask_S(taskID, executor, requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_PlayerController = SCR_PlayerController.Cast(owner);		
		if (!m_PlayerController)
		{
			Print("SCR_TaskSystemNetworkComponent must be attached to SCR_PlayerController!", LogLevel.ERROR);
			return;
		}

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
}
