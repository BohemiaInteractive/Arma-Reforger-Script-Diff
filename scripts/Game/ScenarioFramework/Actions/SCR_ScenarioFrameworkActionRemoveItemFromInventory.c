[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRemoveItemFromInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		//Due to how invokers are setup, deletion is sometimes triggered before said item is actually in said inventory
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(OnActivateCalledLater, 1000, false, object);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	void OnActivateCalledLater(IEntity object)
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

		Resource resource;
		BaseContainer prefabContainer;
		array<IEntity> itemsToRemove = {};
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

					itemsToRemove.Insert(item);
					count++;
					if (count == targetCount)
						break;
				}
				else
				{
					while (container)
					{
						if (container == prefabContainer)
						{
							itemsToRemove.Insert(item);
							count++;
							break;
						}

						container = container.GetAncestor();
					}
				}

				if (count == targetCount)
					break;
			}

			for (int i = itemsToRemove.Count() - 1; i >= 0; i--)
			{
				inventoryComponent.TryDeleteItem(itemsToRemove[i]);
			}
		}
	}
}