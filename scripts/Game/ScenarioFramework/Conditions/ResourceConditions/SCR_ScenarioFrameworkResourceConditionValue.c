[BaseContainerProps()]
class SCR_ScenarioFrameworkResourceConditionValue : SCR_ScenarioFrameworkResourceConditionBase
{
	[Attribute(defvalue: "0", desc: "Activation amount", params: "0 inf")]
	float m_fTargetValue;
	
	[Attribute(defvalue: "0", desc: "Operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_EScenarioFrameworkComparisonOperator)]
	SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(notnull IEntity ent, notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType)
	{
		super.Init(ent, resourceComponent, resourceType);
		
		float current;
		SCR_ResourceSystemHelper.GetStoredResources(resourceComponent, current, resourceType);
		
		if (((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) && (current < m_fTargetValue))	||	((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 	&& (current <= m_fTargetValue)) ||	((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) && (current == m_fTargetValue)) || ((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) && (current >= m_fTargetValue)) ||((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) && (current > m_fTargetValue)))
			return true;
		
		return false;
	}
}