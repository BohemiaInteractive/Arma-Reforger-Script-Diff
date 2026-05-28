[BaseContainerProps()]
class SCR_ScenarioFrameworkInstigatorContextConditionRelation : SCR_ScenarioFrameworkInstigatorContextConditionBase
{

	[Attribute(desc: "Checked Victim-Killer Relation", uiwidget: UIWidgets.ComboBox, enumType: SCR_ECharacterDeathStatusRelations)]
	ref array<SCR_ECharacterDeathStatusRelations> m_aVictimKillerRelations;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(SCR_InstigatorContextData instigatorContext)
	{	
		if (!m_aVictimKillerRelations || m_aVictimKillerRelations.IsEmpty())
			return false;
		
		foreach (SCR_ECharacterDeathStatusRelations relation : m_aVictimKillerRelations)
		{
			if (relation == instigatorContext.GetVictimKillerRelation())
				return true;
		}
		
		return false;
	}
}