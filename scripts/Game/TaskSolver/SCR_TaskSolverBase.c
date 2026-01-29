void OnTaskSolverStateChangedDelegate(SCR_TaskSolverBase taskSolver, SCR_ETaskSolverState taskSolverState);
typedef func OnTaskSolverStateChangedDelegate;
typedef ScriptInvokerBase<OnTaskSolverStateChangedDelegate> OnTaskSolverStateChangedInvoker;

class SCR_TaskSolverBase
{
	protected SCR_Task m_Task;
	protected SCR_AIGroup m_AiGroup;
	protected SCR_ETaskSolverState m_eState;
	protected int m_iId = -1;

	protected ref OnTaskSolverStateChangedInvoker m_OnStateChanged;

	//------------------------------------------------------------------------------------------------
	int GetId()
	{
		return m_iId;
	}

	//------------------------------------------------------------------------------------------------
	void SetId(int id)
	{
		if (m_iId == id)
			return;

		m_iId = id;
	}

	//------------------------------------------------------------------------------------------------
	void SetAiGroup(SCR_AIGroup aiGroup)
	{
		if (m_AiGroup == aiGroup)
			return;

		m_AiGroup = aiGroup;
	}

	//------------------------------------------------------------------------------------------------
	OnTaskSolverStateChangedInvoker GetOnStateChanged()
	{
		if (!m_OnStateChanged)
			m_OnStateChanged = new OnTaskSolverStateChangedInvoker();

		return m_OnStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_Task GetTask()
	{
		return m_Task;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ETaskSolverState GetState()
	{
		return m_eState;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks the task solver as in progress.
	void StartTaskSolving()
	{
		SetState(SCR_ETaskSolverState.IN_PROGRESS);
	}

	//------------------------------------------------------------------------------------------------
	//! Marks the task solver as failed.
	void FailTaskSolving()
	{
		SetState(SCR_ETaskSolverState.FAILED);
	}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice);

	//------------------------------------------------------------------------------------------------
	protected void SetState(SCR_ETaskSolverState state)
	{
		if (m_eState == state)
			return;

		string previousStateName = SCR_Enum.GetEnumName(SCR_ETaskSolverState, m_eState);
		string currentStateName = SCR_Enum.GetEnumName(SCR_ETaskSolverState, state);
		PrintFormat("SCR_TaskSolverBase: Changed state from %1 to %2.", previousStateName, currentStateName, level: LogLevel.DEBUG);

		m_eState = state;

		if (m_OnStateChanged)
			m_OnStateChanged.Invoke(this, state);
	}

	//------------------------------------------------------------------------------------------------
	//! Marks the task solver as completed.
	protected void CompleteTaskSolving()
	{
		SetState(SCR_ETaskSolverState.COMPLETED);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != m_Task)
			return;

		if (newState != SCR_ETaskState.COMPLETED && newState != SCR_ETaskState.FAILED && newState != SCR_ETaskState.CANCELLED)
			return;

		CompleteTaskSolving();
	}

	//------------------------------------------------------------------------------------------------
	void Init(SCR_Task task, SCR_TaskSolverContext context)
	{
		SetState(SCR_ETaskSolverState.NOT_STARTED);

		m_Task = task;

		SCR_Task.GetOnTaskStateChanged().Insert(OnTaskStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	void Deinit()
	{
		SCR_Task.GetOnTaskStateChanged().Remove(OnTaskStateChanged);
	}
}
