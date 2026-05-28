#ifdef WORKBENCH
class SCR_BIKIGearHelper
{
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Characters's gear
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! \param[in] characterContainer
	//! \return
	static array<ResourceName> GetWeapons(notnull BaseContainer characterContainer)
	{
		array<IEntityComponentSource> entityComponentSources = {};
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(characterContainer, CharacterWeaponSlotComponent, false, entityComponentSources) < 1)
			return {};

		array<ResourceName> result = {};

		ResourceName resourceName;
		foreach (IEntityComponentSource entityComponentSource : entityComponentSources)
		{
			if (entityComponentSource.Get("WeaponTemplate", resourceName) && resourceName) // !IsEmpty()
				result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] characterContainer
	//! \return
	static array<ResourceName> GetItems(notnull BaseContainer characterContainer)
	{
		IEntityComponentSource entityComponentSource = SCR_BaseContainerTools.FindComponentSource(characterContainer, SCR_InventoryStorageManagerComponent);
		if (!entityComponentSource)
			return {};

		array<ResourceName> result = {};

		BaseContainerList inventoryItemConfigs = entityComponentSource.GetObjectArray("InitialInventoryItems");
		array<ResourceName> prefabsToSpawn;
		for (int i, count = inventoryItemConfigs.Count(); i < count; ++i)
		{
			if (inventoryItemConfigs.Get(i).Get("PrefabsToSpawn", prefabsToSpawn))
			{
				foreach (ResourceName prefabToSpawn : prefabsToSpawn)
				{
					result.Insert(prefabToSpawn);
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] characterContainer
	//! \return
	static array<ResourceName> GetGear(notnull BaseContainer characterContainer)
	{
		IEntityComponentSource entityComponentSource = SCR_BaseContainerTools.FindComponentSource(characterContainer, BaseLoadoutManagerComponent);
		if (!entityComponentSource)
			return {};

		array<ResourceName> result = {};

		ResourceName resourceName;
		BaseContainerList gearSlots = entityComponentSource.GetObjectArray("Slots");
		for (int i, count = gearSlots.Count(); i < count; ++i)
		{
			if (gearSlots.Get(i).Get("Prefab", resourceName) && resourceName) // !IsEmpty()
				result.Insert(resourceName);
		}

		return result;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Gear itself
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif // WORKBENCH
