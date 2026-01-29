[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputCheckEntitiesInTrigger : SCR_ScenarioFrameworkActionInputBase
{
	[Attribute(desc: "Trigger")];
	protected ref SCR_ScenarioFrameworkGet					m_Getter;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator			m_eComparisonOperator;
	
	[Attribute(desc: "Value")];
	protected int							m_iValue;
	
	protected SCR_ScenarioFrameworkTriggerEntity	m_Trigger;
		
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework: Selected getter %1 is not suitable for this operation", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			PrintFormat("ScenarioFramework: Selected getter entity is null", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(layer.GetSpawnedEntity());
		if (!trigger)
			return;		//TODO: add a universal method for informing user about errors

		m_Trigger = trigger;
		//We want to give trigger enough time to be properly set up and not to get OnChange called prematurely
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(RegisterOnChange, 5000);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RegisterOnChange()
	{
		if (m_Trigger)
			m_Trigger.GetOnChange().Insert(OnActivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] param
	void OnActivate(SCR_ScenarioFrameworkParam<IEntity> param)
	{
		if (!m_Trigger)
			return;
		
		array<IEntity> aEntities = {};
		int iNrOfEnts = m_Trigger.GetCountEntitiesInside();
		
		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (iNrOfEnts < m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (iNrOfEnts <= m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (iNrOfEnts == m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (iNrOfEnts >= m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (iNrOfEnts > m_iValue)) 
		)
		{
			m_Input.OnActivate(true, m_Trigger);
		}
	}
	
}