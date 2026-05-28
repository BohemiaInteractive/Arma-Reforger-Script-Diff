//! Types of task to track for analytical purposes, not completely related with their real task state
enum SCR_EAnalyticalTaskState
{
	TASK_CREATED,
	REQUEST_CREATED,
	REQUEST_APPROVED,
	REQUEST_CANCELED,
	REQUEST_COMPLETED,
	REQUEST_FAILED,
	TASK_COMPLETED,
	TASK_FAILED,
	TASK_CANCELLED
}

[BaseContainerProps()]
class SCR_AnalyticsDataCollectionTasksModule : SCR_AnalyticsDataCollectionModule
{
	protected ref array<ref SCR_SessionDataEvent> m_TaskCache;
	protected int m_TaskCacheLifeTime = 15000; //! 15 seconds
	
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();
		m_TaskCache = {};		
		SCR_TaskSystem.GetOnTaskCreated().Insert(TaskCreated);
		SCR_TaskSystem.GetOnTaskRemoved().Insert(TaskRemoved);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();
		SCR_TaskSystem.GetOnTaskCreated().Remove(TaskCreated);
		SCR_TaskSystem.GetOnTaskRemoved().Remove(TaskRemoved);
		delete m_TaskCache;
	}

	//------------------------------------------------------------------------------------------------
	//! Task is created, send event to analytics db
	//! \param[in] task
	protected void TaskCreated(SCR_Task task)
	{
		if (IsTaskRequested(task))
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.REQUEST_CREATED);
		else
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.TASK_CREATED);
	}

	//------------------------------------------------------------------------------------------------
	//! Task is removed, send event to analytics db
	//! \param[in] task
	protected void TaskRemoved(SCR_Task task)
	{
		if (IsTaskRequested(task) && task.GetTaskState() == SCR_ETaskState.COMPLETED && SCR_EstablishBaseRequestedTaskEntity.Cast(task))
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.REQUEST_APPROVED);
		else if (IsTaskRequested(task) && task.GetTaskState() == SCR_ETaskState.COMPLETED)
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.REQUEST_COMPLETED);
		else if (IsTaskRequested(task) && task.GetTaskState() == SCR_ETaskState.CANCELLED)
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.REQUEST_CANCELED);
		else if (IsTaskRequested(task))
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.REQUEST_FAILED);
		else if(task.GetTaskState() == SCR_ETaskState.COMPLETED)
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.TASK_COMPLETED);
		else if(task.GetTaskState() == SCR_ETaskState.FAILED)
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.TASK_FAILED);
		else
			SendSessionDataEvent(task, SCR_EAnalyticalTaskState.TASK_CANCELLED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! The SessionDataEvent is sent, added to a queue and removed after a specific time
	//! \param[in] task
	//! \param[in] state
	protected void SendSessionDataEvent(SCR_Task task, SCR_EAnalyticalTaskState state)
	{
		SCR_SessionDataEvent sessionEvent = new SCR_SessionDataEvent();
		sessionEvent.task_name = task.GetTaskName();
		sessionEvent.task_id = task.GetTaskID();
		sessionEvent.task_state = typename.EnumToString(SCR_EAnalyticalTaskState, state);	
		
		if(task.GetAuthorID() != -1)
			sessionEvent.task_author_id = SCR_PlayerIdentityUtils.GetPlayerIdentityId(task.GetAuthorID());
		else
			sessionEvent.task_author_id = task.GetAuthorID().ToString();
		
		m_TaskCache.Insert(sessionEvent);
		
		GetGame().GetStatsApi().CreateSessionEvent(sessionEvent, "taskUpdate");
		GetGame().GetCallqueue().CallLater(RemoveElement, m_TaskCacheLifeTime, false, sessionEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove element from the cache
	//! \param[in] item
	protected void RemoveElement(SCR_SessionDataEvent item)
	{
		m_TaskCache.RemoveItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the given task is requested
	//! \param[in] task
	protected bool IsTaskRequested(SCR_Task task)
	{
		return SCR_BaseRequestedTaskEntity.Cast(task) != null;
	}
}
