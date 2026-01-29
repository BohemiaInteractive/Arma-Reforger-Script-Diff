[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCompareCounterAndExecute : SCR_ScenarioFrameworkActionBase
{

	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;

	[Attribute(desc: "Value")]
	int	m_iValue;

	[Attribute(desc: "Counter to compare (Optional if this action is attached on Counter)")]
	string	m_sCounterName;

	[Attribute(desc: "What to do once counter is reached", category: "OnActivate")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkLogicCounter logicCounter;
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sCounterName))
		{
			if (!object)
			{
				PrintFormat("ScenarioFramework Action: Logic Counter not found for Action %1", this, LogLevel.ERROR);
				return;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(object);
			if (!logicCounter)
			{
				PrintFormat("ScenarioFramework Action: Logic Counter %1 not found for Action %2", object.GetName(), this, LogLevel.ERROR);
				return;
			}
		}
		else
		{
			IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
			if (!entity)
			{
				PrintFormat("ScenarioFramework Action: Could not find %1 for Action %2", m_sCounterName, this, LogLevel.ERROR);
				return;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
			if (!logicCounter)
			{
				PrintFormat("ScenarioFramework Action: Logic Counter %1 not found for Action %2", m_sCounterName, this, LogLevel.ERROR);
				return;
			}
		}

		int counterValue = logicCounter.m_iCnt;

		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (counterValue < m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (counterValue <= m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (counterValue == m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 		&& (counterValue >= m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 			&& (counterValue > m_iValue))
		)
		{
			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(object);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}