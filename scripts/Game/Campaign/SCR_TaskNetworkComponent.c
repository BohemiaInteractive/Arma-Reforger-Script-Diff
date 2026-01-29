//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Handles client > server communication for tasks. Should be attached to PlayerController.", color: "0 0 255 255")]
//class SCR_TaskNetworkComponentClass : ScriptComponentClass
//{
//}
//
////! Takes care of tasks-specific server <> client communication and requests
//[Obsolete()]
//class SCR_TaskNetworkComponent : ScriptComponent
//{
//	protected static const int INVALID_PLAYER_ID = -1;
//	
//	// Member variables 
//	protected SCR_PlayerController m_PlayerController;
//	protected RplComponent m_RplComponent;
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	//! Allows the requester to cancel a task requested by them.
//	//! \param[in] taskID
//	void CancelTask(int taskID)
//	{
//		Rpc(RPC_CancelTask, taskID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Allows the local player to abandon a task assigned to them.
//	//! \param[in] taskID
//	void AbandonTask(int taskID)
//	{
//		if (m_RplComponent && m_RplComponent.Role() == RplRole.Authority)
//		{
//			// On server we call this just like any other method
//			RPC_AbandonTask(taskID, INVALID_PLAYER_ID);
//		}
//		else
//		{
//			// Find local task executor
//			SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
//			if (!taskExecutor)
//				return;
//			
//			// Send RPC to server requesting assignment
//			int playerID = SCR_BaseTaskExecutor.GetTaskExecutorID(taskExecutor);
//			Rpc(RPC_AbandonTask, taskID, playerID);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Allows the local player to request assignment to a task.
//	//! \param[in] taskID
//	void RequestAssignment(int taskID)
//	{
//		if (m_RplComponent && m_RplComponent.Role() == RplRole.Authority)
//		{
//			// On server we call this just like any other method
//			RPC_RequestAssignment(taskID, INVALID_PLAYER_ID);
//		}
//		else
//		{
//			// Find local task executor
//			SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
//			if (!taskExecutor)
//				return;
//			
//			// Send RPC to server requesting assignment
//			int playerID = SCR_BaseTaskExecutor.GetTaskExecutorID(taskExecutor);
//			Rpc(RPC_RequestAssignment, taskID, playerID);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] gmID
//	//! \param[in] taskID
//	//! \param[in] playerID
//	void AssignTaskToPlayer(int gmID, int taskID, int playerID)
//	{
//		//Check if the player is game master
//		//This check is here in case someone called it even though they're not GM, prevents unncessary RPC sending
//		if (!SCR_EditorManagerEntity.IsLimitedInstance())
//			Rpc(RPC_AssignTaskToPlayer, gmID, taskID, playerID);
//	}
//	
//	//***********//
//	//RPC METHODS//
//	//***********//
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_AssignTaskToPlayer(int gmID, int taskID, int playerID)
//	{
//		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
//		if (!core)
//		{
//			//Player cannot be GM
//			return;
//		}
//		
//		SCR_EditorManagerEntity manager = core.GetEditorManager(gmID);
//		if (!manager || manager.IsLimited())
//		{
//			//Player isn't GM
//			return;
//		}
//		
//		SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		if (!assignee)
//			return;
//		
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		SCR_BaseTask task = GetTaskManager().GetTask(taskID);
//		if (!task)
//			return;
//		
//		SCR_BaseTask assigneeTask = assignee.GetAssignedTask();
//		if (assigneeTask)
//			supportEntity.UnassignTask(assigneeTask, assignee, SCR_EUnassignReason.GM_REASSIGN);
//		
//		supportEntity.AssignTask(task, assignee, true);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An RPC executed on the server, tells task manager to cancel given task by given player
//	//! \param[in] taskID
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_CancelTask(int taskID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (supportEntity)
//			supportEntity.CancelTask(taskID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An RPC executed on the server, tells task manager to abandon given task by given player
//	//! \param[in] taskID
//	//! \param[in] playerID
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_AbandonTask(int taskID, int playerID)
//	{
//		SCR_BaseTaskExecutor assignee = null;
//		
//		if (playerID == INVALID_PLAYER_ID)
//			assignee = SCR_BaseTaskExecutor.GetLocalExecutor();
//		else
//			assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		
//		if (!assignee)
//			return;
//		
//		GetTaskManager().AbandonTask(taskID, assignee);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An RPC executed on the server, tells task manager to assign given task to given player
//	//! \param[in] taskID
//	//! \param[in] playerID
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_RequestAssignment(int taskID, int playerID)
//	{
//		SCR_BaseTaskExecutor assignee = null;
//		
//		if (playerID == INVALID_PLAYER_ID)
//			assignee = SCR_BaseTaskExecutor.GetLocalExecutor();
//		else
//			assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		
//		if (!assignee)
//			return;
//		
//		GetTaskManager().RequestAssignment(taskID, assignee);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void EOnInit(IEntity owner)
//	{
//		m_PlayerController = SCR_PlayerController.Cast(PlayerController.Cast(owner));
//		
//		if (!m_PlayerController)
//		{
//			Print("SCR_TaskNetworkComponent must be attached to SCR_PlayerController!", LogLevel.ERROR);
//			return;
//		}
//
//		m_RplComponent = RplComponent.Cast(m_PlayerController.FindComponent(RplComponent));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void OnPostInit(IEntity owner)
//	{
//		super.OnPostInit(owner);
//		SetEventMask(owner, EntityEvent.INIT);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	// constructor
//	void SCR_TaskNetworkComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
//	{
//	}
//}
