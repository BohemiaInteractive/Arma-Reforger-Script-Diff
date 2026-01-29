[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnResourceChanged : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate supply (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Actions that will be executed on compartment entered", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;
	
	[Attribute(defvalue: "0", desc: "Activation Percentage", params: "0 inf 0.01")]
	float m_iActivationPercentage;
	
	[Attribute(defvalue: "0", desc: "Operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_EScenarioFrameworkComparisonOperator)]
	SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;

	protected IEntity m_SupplyHolderEntity;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!ValidateInputEntity(object, m_Getter, m_SupplyHolderEntity))
			return;
		
		SCR_ResourceComponent resourceComponent;
		SCR_ResourceContainer supplyContainer = GetContainer(m_SupplyHolderEntity, resourceComponent);
		if (!supplyContainer)
		{
			if (object)
				PrintFormat("ScenarioFramework Action: No resource container found for Action %1 attached on %2.", this, object.GetName(), level: LogLevel.ERROR);
			else
				PrintFormat("ScenarioFramework Action: No resource container Managers found for Action %1.", this, level: LogLevel.ERROR);

			return;
		}
		
		supplyContainer.GetOnResourcesChanged().Insert(OnResourceChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnResourceChanged(SCR_ResourceContainer container, float previousValue)
	{
		float current = (container.GetResourceValue() / container.GetMaxResourceValue()) * 100;
		
		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (current < m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (current <= m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (current == m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (current >= m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (current > m_iActivationPercentage)) 
		)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			{
				action.Init(m_SupplyHolderEntity);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the resource container at given entity.
	//! \param[in] entity
	//! \param[out] resource component
	//! \return
	SCR_ResourceContainer GetContainer(notnull IEntity entity, out SCR_ResourceComponent component)
	{
		component = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer container;

		if (component)
		{
			container = component.GetContainer(EResourceType.SUPPLIES);
			if (container)
				return container;
		}

		IEntity iteratedEntity = entity.GetChildren();
		while (iteratedEntity)
		{
			component = SCR_ResourceComponent.Cast(iteratedEntity.FindComponent(SCR_ResourceComponent));
			if (component)
			{
				container = component.GetContainer(EResourceType.SUPPLIES);
				if (container)
					return container;
			}

			iteratedEntity = iteratedEntity.GetSibling();
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}