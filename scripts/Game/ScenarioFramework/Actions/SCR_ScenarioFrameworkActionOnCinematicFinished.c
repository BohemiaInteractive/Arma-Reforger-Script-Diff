[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnCinematicFinished : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Getter of cinematic entity.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions that will be triggered once the cinematic finishes playing")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		CinematicEntity cinematic = CinematicEntity.Cast(entity);
		if (!cinematic)
		{
			PrintFormat(string.Format("ScenarioFramework Action: Entity %1 is not CinematicEntity.", cinematic), LogLevel.ERROR);
			return;
		}

		cinematic.SetOnFinishedCallback(OnCinematicFinished);
	}

	//------------------------------------------------------------------------------------------------
	void OnCinematicFinished()
	{
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}
