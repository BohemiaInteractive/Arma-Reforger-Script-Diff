[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetListEntitiesInTrigger : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sTriggerName;
	
	ref array<IEntity> m_aEntities = {};

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		GetEntitiesInTrigger(m_aEntities);

		return new SCR_ScenarioFrameworkParam<array<IEntity>>(m_aEntities);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] aEntities
	void GetEntitiesInTrigger(out notnull array<IEntity> aEntities)
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sTriggerName));
		if (!trigger)
			return;

		trigger.GetEntitiesInside(aEntities);
	}
}