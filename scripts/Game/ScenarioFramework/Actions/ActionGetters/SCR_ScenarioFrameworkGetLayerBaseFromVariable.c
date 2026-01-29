[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLayerBaseFromVariable : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sVariableName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return null;
		
		string value;
		if (!scenarioFrameworkSystem.GetVariable(m_sVariableName, value))
			return null;
				
		IEntity entity = FindEntityByName(value);
		if (!entity)
			return null;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
}