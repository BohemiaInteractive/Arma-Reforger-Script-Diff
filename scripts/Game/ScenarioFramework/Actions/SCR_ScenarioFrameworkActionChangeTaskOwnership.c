[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskOwnership : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with - Use GetTask")]
	ref SCR_ScenarioFrameworkGetTask m_Getter;

	[Attribute(defvalue: SCR_ETaskOwnership.FACTION.ToString(), desc: "Who will be the owner of the task for whom it will be assignable. In Default, it will be owned by the given Faction.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskOwnership), category: "Task")];
	SCR_ETaskOwnership m_eTaskOwnership;

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
		
		layerTask.m_eTaskOwnership = m_eTaskOwnership;
		layerTask.m_TaskSystem.SetTaskOwnership(task, m_eTaskOwnership);
	}
}