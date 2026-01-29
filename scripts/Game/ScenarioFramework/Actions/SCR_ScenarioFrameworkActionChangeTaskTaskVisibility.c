[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskTaskVisibility : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with - Use GetTask")]
	ref SCR_ScenarioFrameworkGetTask m_Getter;

	[Attribute(defvalue: SCR_ETaskVisibility.FACTION.ToString(), desc: "To whom will the task be visible for. In Default, it will be visible for given Faction.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskVisibility), category: "Task")];
	SCR_ETaskVisibility m_eTaskVisibility;

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
		
		layerTask.m_eTaskVisibility = m_eTaskVisibility;
		layerTask.m_TaskSystem.SetTaskVisibility(task, m_eTaskVisibility);
	}
}