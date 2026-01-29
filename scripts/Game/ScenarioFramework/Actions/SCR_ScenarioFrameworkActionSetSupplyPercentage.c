[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetSupplyPercentage : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate supply (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "75", desc: "Percentage of a supply to be set.", UIWidgets.Graph, "0 100 1")]
	int m_iSupplyPercentage;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		SCR_ResourceComponent resourceComponent;
		SCR_ResourceContainer supplyContainer = GetContainer(entity, resourceComponent);
		if (!supplyContainer)
		{
			if (object)
				Print(string.Format("ScenarioFramework Action: No resource container found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: No resource container Managers found for Action %1.", this), LogLevel.ERROR);

			return;
		}
		
		supplyContainer.SetResourceValue(supplyContainer.GetMaxResourceValue() * (m_iSupplyPercentage * 0.01));		
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
}