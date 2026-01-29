[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCountInventoryItemsAndExecuteAction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	[Attribute(UIWidgets.Auto, desc: "If conditions from Prefab Filter are true, it will execute these actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsToExecute;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		array<IEntity> items = {};
		inventoryComponent.GetItems(items);
		if (items.IsEmpty())
			return;

		bool countCondition;
		Resource resource;
		BaseContainer prefabContainer;
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilter : m_aPrefabFilter)
		{
			resource = Resource.Load(prefabFilter.m_sSpecificPrefabName);
			if (!resource.IsValid())
				continue;

			prefabContainer = resource.GetResource().ToBaseContainer();
			if (!prefabContainer)
				continue;

			int count;
			int targetCount = prefabFilter.m_iPrefabCount;
			bool includeInheritance = prefabFilter.m_bIncludeChildren;
			countCondition = false;

			foreach (IEntity item : items)
			{
				if (!item)
					continue;

				EntityPrefabData prefabData = item.GetPrefabData();
				if (!prefabData)
					continue;

				BaseContainer container = prefabData.GetPrefab();
				if (!container)
					continue;

				if (!includeInheritance)
				{
					if (container != prefabContainer)
						continue;

					count++;
					if (count == targetCount)
					{
						countCondition = true;
						break;
					}
				}
				else
				{
					while (container)
					{
						if (container == prefabContainer)
						{
							count++;
							break;
						}

						container = container.GetAncestor();
					}
				}

				if (count == targetCount)
				{
					countCondition = true;
					break;
				}
			}

			//If just one prefab filter is false, we don't count any further and return
			if (!countCondition)
				return;
		}

		if (countCondition)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsToExecute)
			{
				action.OnActivate(object);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActionsToExecute;
	}
}