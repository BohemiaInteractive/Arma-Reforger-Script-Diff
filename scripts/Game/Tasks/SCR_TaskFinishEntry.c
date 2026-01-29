class SCR_TaskFinishEntry
{
	protected string m_sTaskID;
	protected string m_sTaskName;
	protected ref array<LocalizedString> m_aTaskNameParameters = {};
	protected string m_sTaskTypeName;
	protected WorldTimestamp m_FinishedTimestamp;
	protected SCR_ETaskState m_eFinalState;
	protected ref array<int> m_aAssaigneeIDs = {};

	//------------------------------------------------------------------------------------------------
	string GetTaskID()
	{
		return m_sTaskID;
	}

	//------------------------------------------------------------------------------------------------
	string GetTaskName()
	{
		return m_sTaskName;
	}

	//------------------------------------------------------------------------------------------------
	string GetTaskTypeName()
	{
		return m_sTaskTypeName;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetFinishedTimestamp()
	{
		return m_FinishedTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ETaskState GetFinalState()
	{
		return m_eFinalState;
	}

	//------------------------------------------------------------------------------------------------
	bool WasSuccessful()
	{
		return m_eFinalState == SCR_ETaskState.COMPLETED;
	}

	//------------------------------------------------------------------------------------------------
	array<int> GetAssignees()
	{
		return m_aAssaigneeIDs;
	}

	//------------------------------------------------------------------------------------------------
	void GetUIInfo(out SCR_TaskUIInfo info)
	{
		array<LocalizedString> descParams = {};
		info = SCR_TaskUIInfo.CreateInfo(m_sTaskName, m_aTaskNameParameters, string.Empty, descParams, ResourceName.Empty, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_TaskFinishHistoryManagerComponent)]
	protected bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteString(m_sTaskID);
		writer.WriteString(m_sTaskName);

		writer.WriteInt(m_aTaskNameParameters.Count());
		foreach (LocalizedString parameter : m_aTaskNameParameters)
		{
			writer.WriteString(parameter);
		}
		writer.WriteString(m_sTaskTypeName);

		WorldTimestamp currentTimestamp = GetGame().GetWorld().GetTimestamp();
		float timeStampDifference = currentTimestamp.DiffSeconds(m_FinishedTimestamp);
		writer.WriteFloat(timeStampDifference);

		writer.WriteInt(m_eFinalState);
		writer.WriteInt(m_aAssaigneeIDs.Count());
		foreach (int assigneeID : m_aAssaigneeIDs)
		{
			writer.WriteInt(assigneeID);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_TaskFinishHistoryManagerComponent)]
	protected bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadString(m_sTaskID);
		reader.ReadString(m_sTaskName);

		int nameParameterCount;
		reader.ReadInt(nameParameterCount);
		string nameParameterValue;
		for (int i = 0; i < nameParameterCount; i++)
		{
			reader.ReadString(nameParameterValue);
			m_aTaskNameParameters.Insert(nameParameterValue);
		}
		reader.ReadString(m_sTaskTypeName);

		WorldTimestamp currentTimestamp = GetGame().GetWorld().GetTimestamp();
		float timeStampDifference;
		reader.ReadFloat(timeStampDifference);
		m_FinishedTimestamp = currentTimestamp.PlusSeconds(timeStampDifference);

		reader.ReadInt(m_eFinalState);
		int assigneeCount;
		reader.ReadInt(assigneeCount);
		int assigneeID;
		for (int i = 0; i < assigneeCount; i++)
		{
			reader.ReadInt(assigneeID);
			m_aAssaigneeIDs.Insert(assigneeID);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Init(notnull SCR_Task task, SCR_ETaskState state)
	{
		m_sTaskID = task.GetTaskID();
		m_sTaskName = task.GetTaskUIInfo().GetUnformattedName(m_aTaskNameParameters);
		m_sTaskTypeName = task.Type().ToString();
		m_FinishedTimestamp = task.GetWorld().GetTimestamp();
		m_eFinalState = state;
		m_aAssaigneeIDs = task.GetTaskAssigneePlayerIDs();
	}
}
