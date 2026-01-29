[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionProcessVoicelineEnumAndString : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the enum to work with")]
	string m_sTargetEnum;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
	    if (!taskSystem)
	        return;

		array<SCR_Task> tasks = {};
		taskSystem.GetTasks(tasks);
		
		typename targetEnum = m_sTargetEnum.ToType();
		foreach (SCR_Task task : tasks)
		{
			SCR_ScenarioFrameworkTask frameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
			if (!frameworkTask)
				return;
			
			SCR_ScenarioFrameworkSlotTask slotTask = frameworkTask.GetSlotTask();
			if (slotTask)
				scenarioFrameworkSystem.ProcessVoicelineEnumAndString(targetEnum, slotTask.m_sTaskIntroVoiceline);
		}
	}
}