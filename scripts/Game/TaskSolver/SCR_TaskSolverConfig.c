[BaseContainerProps(configRoot: true)]
class SCR_TaskSolverConfig
{
	[Attribute()]
	protected ref array<ref SCR_TaskSolverEntry> m_aTaskSolverEntries;

	//------------------------------------------------------------------------------------------------
	SCR_TaskSolverEntry GetTaskSolverEntry(typename type)
	{
		foreach (SCR_TaskSolverEntry taskSolverEntry : m_aTaskSolverEntries)
		{
			if (taskSolverEntry.Type() == type)
				return taskSolverEntry;
		}

		return null;
	}
}
