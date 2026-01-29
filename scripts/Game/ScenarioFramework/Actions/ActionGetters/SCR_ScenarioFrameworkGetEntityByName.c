[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetEntityByName : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sEntityName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		return new SCR_ScenarioFrameworkParam<IEntity>(GetGame().GetWorld().FindEntityByName(m_sEntityName));
	}
}