[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextConditionValue : SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute(defvalue: "0", desc: "Activation Value", params: "0 inf 0.01")]
	float m_fActivationValue;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(BaseDamageContext damageContext)
	{	
		return (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (damageContext.damageValue < m_fActivationValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (damageContext.damageValue <= m_fActivationValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (damageContext.damageValue == m_fActivationValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (damageContext.damageValue >= m_fActivationValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (damageContext.damageValue > m_fActivationValue)) 
			);
	}
}