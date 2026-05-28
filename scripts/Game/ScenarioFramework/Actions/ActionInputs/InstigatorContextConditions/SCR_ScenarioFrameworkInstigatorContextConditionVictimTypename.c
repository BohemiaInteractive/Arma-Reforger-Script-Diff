[BaseContainerProps()]
class SCR_ScenarioFrameworkInstigatorContextConditionVictimTypename : SCR_ScenarioFrameworkInstigatorContextConditionBase
{
	[Attribute(desc: "Typenames To Check")]
	ref array<string> m_aTypenamesToCheck;
	
	[Attribute(defvalue: "1")]
	bool m_bAllowInherited;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(SCR_InstigatorContextData instigatorContext)
	{	
		if (!m_aTypenamesToCheck || m_aTypenamesToCheck.IsEmpty())
			return false;
		
		IEntity victimEntity = instigatorContext.GetVictimEntity();
		if (!victimEntity)
			return false;
		
		typename typeName;
		foreach (string type : m_aTypenamesToCheck)
		{
			typeName = type.ToType();
			
			if ((m_bAllowInherited && victimEntity.IsInherited(typeName)) || (victimEntity.Type() == typeName))
				return true;
		}

		return false;
	}
}