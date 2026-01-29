[BaseContainerProps()]
class SCR_ScenarioFrameworkNotorietyCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Notoriety Value to check", params: "0 inf 0.01")]
	float m_fNotorietyValue;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		SCR_ScenarioFrameworkNotorietyComponent notorietyComponent = SCR_ScenarioFrameworkNotorietyComponent.GetInstance();
		if (!notorietyComponent)
			return true;
		
		float notorietyValue = notorietyComponent.GetNotorietyValue();
		
		return 	((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (notorietyValue < m_fNotorietyValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (notorietyValue <= m_fNotorietyValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (notorietyValue == m_fNotorietyValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (notorietyValue >= m_fNotorietyValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (notorietyValue > m_fNotorietyValue));
	}
}