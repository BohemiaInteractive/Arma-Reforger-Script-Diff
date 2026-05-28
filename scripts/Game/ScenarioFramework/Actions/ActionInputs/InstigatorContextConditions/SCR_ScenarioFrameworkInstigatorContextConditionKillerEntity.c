[BaseContainerProps()]
class SCR_ScenarioFrameworkInstigatorContextConditionKillerEntity : SCR_ScenarioFrameworkInstigatorContextConditionBase
{
	[Attribute(desc: "Entities to be checked for being killer")]
	ref SCR_ScenarioFrameworkGet m_KillerGetter;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(SCR_InstigatorContextData instigatorContext)
	{
		IEntity killerEntity = instigatorContext.GetKillerEntity();
		if (!killerEntity)
			return false;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_KillerGetter.Get());
		if (!entityWrapper)
		{
			SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_KillerGetter.Get());
			if (!entityArrayWrapper)
				return false;
				
			array<IEntity> entities = {};
			entities = entityArrayWrapper.GetValue();
				
			foreach (IEntity entity : entities)
			{
				if (entity == killerEntity)
					return true;
			}
				
			return false;
		}
			
		return entityWrapper.GetValue() == killerEntity;
	}
}