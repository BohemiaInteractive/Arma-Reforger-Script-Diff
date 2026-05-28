[BaseContainerProps()]
class SCR_ScenarioFrameworkInstigatorContextConditionVictimEntity : SCR_ScenarioFrameworkInstigatorContextConditionBase
{
	[Attribute(desc: "Entities to be checked for being killer")]
	ref SCR_ScenarioFrameworkGet m_VictimGetter;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(SCR_InstigatorContextData instigatorContext)
	{
		IEntity victimEntity = instigatorContext.GetVictimEntity();
		if (!victimEntity)
			return false;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_VictimGetter.Get());
		if (!entityWrapper)
		{
			SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_VictimGetter.Get());
			if (!entityArrayWrapper)
				return false;
				
			array<IEntity> entities = {};
			entities = entityArrayWrapper.GetValue();
				
			if (entities.IsEmpty())
				return false;
				
			foreach (IEntity entity : entities)
			{
				if (entity == victimEntity)
					return true;
			}
				
			return false;
		}
			
		return entityWrapper.GetValue() == victimEntity;
	}
}