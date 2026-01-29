[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskIcon : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with - Use GetTask")]
	ref SCR_ScenarioFrameworkGetTask		m_Getter;

	[Attribute(params: "edds imageset", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Task icon set", category: "Task")]
	ResourceName m_sTaskIconSet;
	
	[Attribute(desc: "Name of the specific icon from the icon set", category: "Task")]
	string m_sTaskIconName;

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
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTaskIconSet))
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task Icon Set empty for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task Icon Set empty for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTaskIconName))
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task Icon Name empty for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task Icon Name empty for Action %1.", this), LogLevel.ERROR);
			return;
		}

		layerTask.m_sTaskIconSet = m_sTaskIconSet;
		layerTask.m_sTaskIconName = m_sTaskIconName;
		task.SetTaskIconPath(m_sTaskIconSet);
		task.SetTaskIconSetName(m_sTaskIconName);
	}
}