//[EntityEditorProps(category: "GameScripted/Tasks", description: "This this entity to receive and complete tasks.", color: "0 0 255 255")]
//class SCR_BaseTaskExecutorClass: GenericEntityClass
//{
//};
//
////------------------------------------------------------------------------------------------------
//class SCR_BaseTaskExecutor : GenericEntity
//{
//	
//	static const int INVALID_PLAYER_ID = -1;
//	protected static ref map<SCR_BaseTaskExecutor, int> s_mTaskExecutors = new map<SCR_BaseTaskExecutor, int>();
//	protected static SCR_BaseTaskExecutor s_LocalTaskExecutor = null;
//	
//	//**************************//
//	//PROTECTED MEMBER VARIALBES//
//	//**************************//
//	
//	protected SCR_BaseTask m_CurrentTask;
//	
//	//*********************//
//	//PUBLIC STATIC METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	static map<SCR_BaseTaskExecutor, int> GetTaskExecutorsMap()
//	{
//		return s_mTaskExecutors;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static int GetTaskExecutorID(SCR_BaseTaskExecutor taskExecutor)
//	{
//		if (!taskExecutor)
//			return INVALID_PLAYER_ID;
//		
//		return s_mTaskExecutors.Get(taskExecutor);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static int GetLocalExecutorID()
//	{
//		return GetTaskExecutorID(GetLocalExecutor());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the local task executor
//	static SCR_BaseTaskExecutor GetLocalExecutor()
//	{
//		if (s_LocalTaskExecutor)
//			return s_LocalTaskExecutor;
//		
//		PlayerController playerController = GetGame().GetPlayerController();
//		if (!playerController)
//			return null;
//		
//		int playerID = playerController.GetPlayerId();
//		
//		SCR_BaseTaskExecutor taskExecutor = s_mTaskExecutors.GetKeyByValue(playerID);
//		
//		// Local task executor not found, create new one
//		if (!taskExecutor)
//			taskExecutor = GetTaskManager().LocalCreateTaskExecutor(GetGame().GetPlayerController().GetPlayerId());
//		
//		return taskExecutor;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static SCR_BaseTaskExecutor FindTaskExecutorByID(int playerID)
//	{
//		return s_mTaskExecutors.GetKeyByValue(playerID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the task executor with given ID
//	static notnull SCR_BaseTaskExecutor GetTaskExecutorByID(int playerID)
//	{
//		SCR_BaseTaskExecutor taskExecutor = s_mTaskExecutors.GetKeyByValue(playerID);
//		
//		if (!taskExecutor)
//		{
//			taskExecutor = GetTaskManager().LocalCreateTaskExecutor(playerID);
//		}
//		
//		return taskExecutor;
//	}
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	int GetExecutorID()
//	{
//		return GetTaskExecutorID(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	IEntity GetControlledEntity()
//	{
//		return SCR_PossessingManagerComponent.GetPlayerMainEntity(s_mTaskExecutors.Get(this));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns currently assigned task.
//	SCR_BaseTask GetAssignedTask()
//	{
//		return m_CurrentTask;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns player name of this task executor
//	string GetPlayerName()
//	{
//		return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(GetTaskExecutorID(this));
//		
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Sets this task executor the given player ID
//	void SetPlayerID(int playerID)
//	{
//		if (!s_mTaskExecutors || playerID == INVALID_PLAYER_ID)
//			return;
//		
//		s_mTaskExecutors.Set(this, playerID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Assigns new task.
//	void AssignNewTask(SCR_BaseTask task)
//	{
//		if (task == m_CurrentTask)
//			return;
//		
//		OnNewTaskAssigned(m_CurrentTask, task);
//		m_CurrentTask = task;
//	}
//	
//	//***************************//
//	//PUBLIC EVENT MEMBER METHODS//
//	//***************************//
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when the assigned task has been removed.
//	void OnAssignedTaskRemoved()
//	{
//		m_CurrentTask = null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when the assigned task has failed.
//	void OnAssignedTaskFailed()
//	{
//		m_CurrentTask = null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when the assigned task has been finished.
//	void OnAssignedTaskFinished()
//	{
//		m_CurrentTask = null;
//	}
//	
//	//******************************//
//	//PROTECTED MEMBER EVENT METHODS//
//	//******************************//
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a new task has been assigned to this task executor.
//	protected void OnNewTaskAssigned(SCR_BaseTask oldTask, SCR_BaseTask newTask)
//	{
//		
//	}
//
//	//************************//
//	//CONSTRUCTOR & DESTRUCTOR//
//	//************************//
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_BaseTaskExecutor(IEntitySource src, IEntity parent)
//	{
//		if (s_mTaskExecutors)
//			s_mTaskExecutors.Insert(this, INVALID_PLAYER_ID);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_BaseTaskExecutor()
//	{
//		if (s_mTaskExecutors)
//			s_mTaskExecutors.Remove(this);
//	}
//
//};
//
