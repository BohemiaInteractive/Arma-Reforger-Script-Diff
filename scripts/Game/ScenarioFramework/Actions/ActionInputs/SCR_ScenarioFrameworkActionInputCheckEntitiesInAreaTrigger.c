[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputCheckEntitiesInAreaTrigger : SCR_ScenarioFrameworkActionInputCheckEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework: Selected getter %1 is not suitable for this operation", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			PrintFormat("ScenarioFramework: Selected getter entity is null", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;
		
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(layer);
		if (!area)
			return;

		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(area.GetTrigger());
		if (!trigger)
			return;

		m_Trigger = trigger;
		//We want to give trigger enough time to be properly set up and not to get OnChange called prematurely
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(RegisterOnChange, 5000);
	}	
}