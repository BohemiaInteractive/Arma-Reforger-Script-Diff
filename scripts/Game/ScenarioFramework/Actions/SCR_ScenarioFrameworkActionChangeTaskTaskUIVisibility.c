[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskTaskUIVisibility : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with - Use GetTask")]
	ref SCR_ScenarioFrameworkGetTask m_Getter;

	[Attribute(defvalue: SCR_ETaskUIVisibility.ALL.ToString(), desc: "Where will the task be visible in UI. In default, it will be visible in the Task List and on the Map.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskUIVisibility), category: "Task")];
	SCR_ETaskUIVisibility m_eTaskUIVisibility;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1.", this), LogLevel.ERROR);

			return;
		}
		
		SCR_ScenarioFrameworkLayerTask layerTask = task.GetLayerTask();
		if (!layerTask)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Layer Task not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Layer Task not found for Action %1.", this), LogLevel.ERROR);

			return;
		}
		
		layerTask.m_eTaskUIVisibility = m_eTaskUIVisibility;
		layerTask.m_TaskSystem.SetTaskUIVisibility(task, m_eTaskUIVisibility);
	}
}