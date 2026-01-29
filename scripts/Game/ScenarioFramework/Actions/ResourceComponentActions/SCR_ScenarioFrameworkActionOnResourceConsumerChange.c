[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnResourceConsumerChange : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate supply (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Actions that will be executed on compartment entered", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;
	
	[Attribute(defvalue: "0", desc: "Activation amount", params: "0 inf 0.01")]
	int m_iActivationAmount;
	
	[Attribute(defvalue: "0", desc: "Operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_EScenarioFrameworkComparisonOperator)]
	SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;

	IEntity m_SupplyHolderEntity;
	ref SCR_ResourceSystemSubscriptionHandleBase m_Handle;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!ValidateInputEntity(object, m_Getter, m_SupplyHolderEntity))
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(m_SupplyHolderEntity.FindComponent(SCR_ResourceComponent));
		if (!resourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		if (!consumer)
		{
			if (object)
				Print(string.Format("ScenarioFramework Action: No resource consumer found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: No resource consumer Managers found for Action %1.", this), LogLevel.ERROR);

			return;
		}
		
		consumer.GetOnResourcesChanged().Insert(OnResourceChanged);
		m_Handle = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(consumer, Replication.FindId(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
	}
	
	//------------------------------------------------------------------------------------------------
	void OnResourceChanged(SCR_ResourceInteractor interactor, float previousValue)
	{
		SCR_ResourceConsumer consumer = SCR_ResourceConsumer.Cast(interactor);
		if (!consumer)
			return;
		
		float current = consumer.GetAggregatedResourceValue();
		
		if (((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) && (current < m_iActivationAmount))	||	((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 	&& (current <= m_iActivationAmount)) ||	((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) && (current == m_iActivationAmount)) || ((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) && (current >= m_iActivationAmount)) ||((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) && (current > m_iActivationAmount)))
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			{
				action.Init(m_SupplyHolderEntity);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the resource consumer at given entity.
	//! \param[in] entity
	//! \param[out] resource component
	//! \return
	SCR_ResourceConsumer GetContainer(notnull IEntity entity, out SCR_ResourceComponent component)
	{
		component = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		SCR_ResourceConsumer consumer;

		if (component)
		{
			consumer = component.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			if (consumer)
				return consumer;
		}

		IEntity iteratedEntity = entity.GetChildren();
		while (iteratedEntity)
		{
			component = SCR_ResourceComponent.Cast(iteratedEntity.FindComponent(SCR_ResourceComponent));
			if (component)
			{
				consumer = component.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (consumer)
					return consumer;
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