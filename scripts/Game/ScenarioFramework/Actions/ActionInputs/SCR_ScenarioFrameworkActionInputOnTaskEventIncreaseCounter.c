[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputOnTaskEventIncreaseCounter : SCR_ScenarioFrameworkActionInputBase
{
	[Attribute(desc: "Insert name of the task layer or leave empty for any task")];
	string m_sTaskLayerName;
	
	[Attribute(defvalue: SCR_ETaskState.COMPLETED.ToString(), UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum(SCR_ETaskState))];
	SCR_ETaskState m_eEventName;
	
	[Attribute(desc: "Names of task layers whose events should be ignored")]
	ref array<string> m_aIgnoredLayerNames;
	
	protected int m_iActionsInput;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		
		SCR_Task.GetOnTaskStateChanged().Remove(OnActivate);
		SCR_Task.GetOnTaskStateChanged().Insert(OnActivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \param[in] mask
	void OnActivate(SCR_Task task, SCR_ETaskState taskState)
	{
		if (!task)
			return;
		
		if (task.GetTaskState() != m_eEventName || !m_Input)
			return;
			
		SCR_ScenarioFrameworkLayerTask taskLayer;
		string sTaskLayerName = "";
		if (SCR_ScenarioFrameworkTask.Cast(task))
		{
			taskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetLayerTask();
			if (taskLayer)
			{
				sTaskLayerName = taskLayer.GetOwner().GetName();
				if (m_aIgnoredLayerNames && m_aIgnoredLayerNames.Contains(sTaskLayerName))
					return;
				
				if (taskLayer.GetLayerTaskResolvedBeforeLoad())
					return;
			}
		} 
		
		if (m_sTaskLayerName.IsEmpty() || m_sTaskLayerName == sTaskLayerName)
		{
			if (taskState == SCR_ETaskState.ASSIGNED)
				return;
			
			m_Input.OnActivate(1, null);
		}
	}	
}