[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeAllTaskState : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", desc: "Layer Tasks To Ignore")]
	ref array<string> m_aLayerTasksToIgnore;
	
	[Attribute(desc: "Ignored Task States", uiwidget: UIWidgets.Flags, enumType: SCR_ETaskState)]
	SCR_ETaskState m_eIgnoredTaskStates;
	
	[Attribute(defvalue: "0", desc: "New Task State", uiwidget: UIWidgets.ComboBox, enumType: SCR_ETaskState)]
	SCR_ETaskState m_eNewTaskState;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_TaskSystem tSystem = SCR_TaskSystem.GetInstance();
		if (!tSystem)
			return;
		
		array <SCR_Task> tasks = {};
		tSystem.GetTasks(tasks);
		
		SCR_ScenarioFrameworkTask sfTask;
		SCR_ScenarioFrameworkLayerTask sfLayerTask;
		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;
			
			if (SCR_Enum.HasFlag(m_eIgnoredTaskStates, task.GetTaskState()))
				continue;
			
			sfTask = SCR_ScenarioFrameworkTask.Cast(task);
			if (!sfTask)
			{
				task.SetTaskState(m_eNewTaskState);
				continue;
			}
			
			sfLayerTask = sfTask.GetLayerTask();
			if (!sfLayerTask)
				continue;
			
			if (m_aLayerTasksToIgnore.Contains(sfLayerTask.GetName()))
				continue;
			
			sfLayerTask.ProcessLayerTaskState(m_eNewTaskState, true);
		}
	}
}