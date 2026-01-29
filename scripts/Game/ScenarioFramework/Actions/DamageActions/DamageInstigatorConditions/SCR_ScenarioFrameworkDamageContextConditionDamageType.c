[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextConditionDamageType : SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute("Accepted damage types")];
	ref array<ref SCR_ScenarioFrameworkDamageContextEnumType> m_aAcceptedDamageTypes;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(BaseDamageContext damageContext)
	{	
		foreach (SCR_ScenarioFrameworkDamageContextEnumType damType : m_aAcceptedDamageTypes)
		{
			if (damageContext.damageType == damType.GetDamageType())
				return true;
		}
	
		return false;
	}
}