class SCR_TaskFinishHistoryManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_TaskFinishHistoryManagerComponent : SCR_BaseGameModeComponent
{
	protected ref array<ref SCR_TaskFinishEntry> m_aFinished = {};

	protected static SCR_TaskFinishHistoryManagerComponent s_Instance;

	//------------------------------------------------------------------------------------------------
	static SCR_TaskFinishHistoryManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return list of all finished task entries
	array<ref SCR_TaskFinishEntry> GetAll()
	{
		array<ref SCR_TaskFinishEntry> outputTasks = {};
		foreach (SCR_TaskFinishEntry taskEntry : m_aFinished)
		{
			outputTasks.Insert(taskEntry);
		}

		return outputTasks;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskID
	//! \return list of finished task entries with the same taskID as input
	array<ref SCR_TaskFinishEntry> GetByTaskID(string taskID)
	{
		array<ref SCR_TaskFinishEntry> outputTasks = {};
		foreach (SCR_TaskFinishEntry taskEntry : m_aFinished)
		{
			if (taskEntry.GetTaskID() == taskID)
				outputTasks.Insert(taskEntry);
		}

		return outputTasks;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return list of finished task entries with player with input playerID as assignee
	array<ref SCR_TaskFinishEntry> GetByPlayerID(int playerID)
	{
		array<ref SCR_TaskFinishEntry> outputTasks = {};
		array<int> assignees = {};
		foreach (SCR_TaskFinishEntry taskEntry : m_aFinished)
		{
			assignees = taskEntry.GetAssignees();
			foreach (int assigneeID : assignees)
			{
				if (assigneeID == playerID)
				{
					outputTasks.Insert(taskEntry);
					break;
				}
			}
		}

		return outputTasks;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] final state
	//! \return list of finished task entries with final state same as input
	array<ref SCR_TaskFinishEntry> GetByFinalState(SCR_ETaskState state)
	{
		array<ref SCR_TaskFinishEntry> outputTasks = {};
		foreach (SCR_TaskFinishEntry taskEntry : m_aFinished)
		{
			if (taskEntry.GetFinalState() == state)
				outputTasks.Insert(taskEntry);
		}

		return outputTasks;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return aggregated counts of task types finished by player with input playerID or returns null if no finished tasks recorded
	map<string, int> GetAggregatedByPlayerID(int playerID)
	{
		if (m_aFinished.IsEmpty())
			return null;

		map<string, int> aggregatedTasks = new map<string, int>();
		string taskTypeName;
		foreach (SCR_TaskFinishEntry entry : m_aFinished)
		{
			if (entry.GetAssignees().Contains(playerID))
			{
				taskTypeName = entry.GetTaskTypeName();
				aggregatedTasks.Set(taskTypeName, aggregatedTasks.Get(taskTypeName) + 1);
			}
		}

		return aggregatedTasks;
	}

	//------------------------------------------------------------------------------------------------
	//! \return aggregated counts of finished task types which are grouped by playerIDs or returns null if no finished tasks recroded
	map<int, ref map<string, int>> GetAggregatedByTypeAndPlayer()
	{
		if (m_aFinished.IsEmpty())
			return null;

		map<int, ref map<string, int>> aggregatedTasks = new map<int, ref map<string, int>>();
		string taskTypeName;
		map<string, int> assigneeAggregatedTasks = new map<string, int>();
		foreach (SCR_TaskFinishEntry entry : m_aFinished)
		{
			taskTypeName = entry.GetTaskTypeName();
			foreach (int assignee : entry.GetAssignees())
			{
				assigneeAggregatedTasks = aggregatedTasks.Get(assignee);
				if (!assigneeAggregatedTasks)
					assigneeAggregatedTasks = new map<string, int>();

				int aggregatedTaskCount = 1;
				if (assigneeAggregatedTasks.Contains(taskTypeName))
					aggregatedTaskCount += assigneeAggregatedTasks.Get(taskTypeName);

				assigneeAggregatedTasks.Set(taskTypeName, aggregatedTaskCount);
				aggregatedTasks.Set(assignee, assigneeAggregatedTasks);
			}
		}

		return aggregatedTasks;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		// We only care about finished task states
		if (!IsFinishedState(newState))
			return;

		// No need to save a task without any assignees
		if (task.GetTaskAssigneeCount() <= 0)
			return;

		RecordIfFinished(task, newState);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsFinishedState(SCR_ETaskState state)
	{
		return state == SCR_ETaskState.COMPLETED || state == SCR_ETaskState.FAILED || state == SCR_ETaskState.CANCELLED;
	}

	//------------------------------------------------------------------------------------------------
	protected void RecordIfFinished(SCR_Task task, SCR_ETaskState newState)
	{
		if (!task)
			return;

		if (!IsFinishedState(newState))
			return;

		SCR_TaskFinishEntry entry = new SCR_TaskFinishEntry();
		entry.Init(task, newState);
		m_aFinished.Insert(entry);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SCR_Task.GetOnTaskStateChanged().Insert(OnTaskStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		SCR_Task.GetOnTaskStateChanged().Remove(OnTaskStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int finishedTasksCount = m_aFinished.Count();
		writer.WriteInt(finishedTasksCount);

		foreach (SCR_TaskFinishEntry entry : m_aFinished)
		{
			entry.RplSave(writer);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int finishedTasksCount;
		reader.ReadInt(finishedTasksCount);

		for (int i = 0; i < finishedTasksCount; i++)
		{
			SCR_TaskFinishEntry entry = new SCR_TaskFinishEntry();
			entry.RplLoad(reader);
			m_aFinished.Insert(entry);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_TaskFinishHistoryManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_TaskFinishHistoryManagerComponent()
	{
		s_Instance = null;
	}
}
