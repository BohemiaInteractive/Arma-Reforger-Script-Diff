[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextConditionInsigatorEntity : SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute(desc: "Checked instigators")]
	ref array <ref SCR_ScenarioFrameworkGet> m_aGetters;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(BaseDamageContext damageContext)
	{
		if (!damageContext || !damageContext.instigator)
			return false;
		
		array<IEntity> possibleInstigators = {};
		if (GetEntities(possibleInstigators) == 0)
			return false;
		
		IEntity instigatorEntity = damageContext.instigator.GetInstigatorEntity();
		if (!instigatorEntity)
			return false;
		
		foreach (IEntity possibleInstigator : possibleInstigators)
		{
			if (instigatorEntity == possibleInstigator)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetEntities(out array<IEntity> entities)
	{	
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper;
		foreach (SCR_ScenarioFrameworkGet getter : m_aGetters)
		{
			entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
			if (!entityWrapper)
			{
				PrintFormat("ScenarioFramework Action: Issue with Getter %1 detected for Action %2.", getter, this, level: LogLevel.ERROR);
				continue;
			}
			
			entities.Insert(entityWrapper.GetValue());
		}
		
		return entities.Count();
	}
}