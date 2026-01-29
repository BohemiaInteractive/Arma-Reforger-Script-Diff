[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetCountEntitiesInTrigger : SCR_ScenarioFrameworkGetListEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);

		return new SCR_ScenarioFrameworkParam<int>(aEntities.Count());
	}
}