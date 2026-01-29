[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionResetCounter : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Counter to reset (Optional if this action is attached on Counter)")]
	string	m_sCounterName;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkLogicCounter logicCounter;
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sCounterName))
		{
			if (!object)
			{
				PrintFormat("ScenarioFramework Action: Logic Counter not found for Action %1", this, LogLevel.ERROR);
				return;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(object);
			if (!logicCounter)
			{
				PrintFormat("ScenarioFramework Action: Logic Counter %1 not found for Action %2", object.GetName(), this, LogLevel.ERROR);
				return;
			}
		}
		else
		{
			IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
			if (!entity)
			{
				PrintFormat("ScenarioFramework Action: Could not find %1 for Action %2", m_sCounterName, this, LogLevel.ERROR);
				return;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
			if (!logicCounter)
			{
				PrintFormat("ScenarioFramework Action: Logic Counter %1 not found for Action %2", m_sCounterName, this, LogLevel.ERROR);
				return;
			}
		}
			
		logicCounter.Reset();
	}
}