[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArrayOfEntities : SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Array of Getters that should return entity or array of entities")]
	ref array<ref SCR_ScenarioFrameworkGet> m_aGetters;
	
	protected ref array<IEntity> m_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		m_aEntities.Clear();
		ProcessGetters(m_aEntities);

		return new SCR_ScenarioFrameworkParam<array<IEntity>>(m_aEntities);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] aEntities
	void ProcessGetters(out notnull array<IEntity> aEntities)
	{
		IEntity entity;
		array<IEntity> entities = {};
		foreach (SCR_ScenarioFrameworkGet getter : m_aGetters)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
			if (entityWrapper)
			{
				entity = entityWrapper.GetValue();
				if (entity)
					aEntities.Insert(entity);
			}
			else
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(getter.Get());
				if (!entityArrayWrapper)
					continue;
				
				entities = entityArrayWrapper.GetValue();
				if (entities && !entities.IsEmpty())
					aEntities.InsertAll(entities);
			}
		}
	}
}