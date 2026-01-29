class SCR_ScenarioFrameworkLogicDistanceClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicDistance : SCR_ScenarioFrameworkLogic
{	
	[Attribute(defvalue: "0", desc: "Checked distance between entities", params: "0 inf 0.1", precision: 1)]
	float m_fDistance;
	
	[Attribute(desc: "Entity A")]
	ref SCR_ScenarioFrameworkGet m_GetterA;

	[Attribute(desc: "Entity B")]
	ref SCR_ScenarioFrameworkGet m_GetterB;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", enumType: SCR_EScenarioFrameworkComparisonOperator)]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	[Attribute(defvalue: "1", desc: "Delay between checks. Lower values are more precise, but has bigger performance impact", params: "0 inf")]
	protected int m_iDelay;
	
	protected IEntity m_EntityA, m_EntityB;
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkDistanceLogic.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!m_GetterA || !m_GetterB)
			return;
			
		SCR_ScenarioFrameworkParam<IEntity> entityWrapperA = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_GetterA.Get());
		if (!entityWrapperA)
		{
			Print(string.Format("SCR_ScenarioFrameworkDistanceLogic: Entity A not found for logic %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapperB = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_GetterB.Get());
		if (!entityWrapperB)
		{
			Print(string.Format("SCR_ScenarioFrameworkDistanceLogic: Entity B not found for logic %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_EntityA = entityWrapperA.GetValue();
		if (!m_EntityA)
		{
			Print(string.Format("SCR_ScenarioFrameworkDistanceLogic: Entity A not found for logic %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_EntityB = entityWrapperB.GetValue();
		if (!m_EntityB)
		{
			Print(string.Format("SCR_ScenarioFrameworkDistanceLogic: Entity B not found for logic %1.", this), LogLevel.ERROR);
			return;
		}
		
		super.Init();
		
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(EvaluateDistance, m_iDelay, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluateDistance()
	{
		float distance = vector.Distance(m_EntityA.GetOrigin(), m_EntityB.GetOrigin());
		
		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (distance < m_fDistance)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (distance <= m_fDistance)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (distance == m_fDistance)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (distance >= m_fDistance)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (distance > m_fDistance)) 
		)
			OnActivate(null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void RestoreToDefault()
	{
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(EvaluateDistance);
	}
}
