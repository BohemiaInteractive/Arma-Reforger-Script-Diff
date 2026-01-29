[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskTitleOrDescription : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with")]
	ref SCR_ScenarioFrameworkGetTask m_Getter;

	[Attribute(desc: "New task name")]
	string m_sTaskTitle;
	
	[Attribute(desc: "New task description")]
	string m_sTaskDescription;

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

		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTaskTitle))
			task.SetTaskName(m_sTaskTitle);
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTaskDescription))
			task.SetTaskDescription(m_sTaskDescription);
	}
}