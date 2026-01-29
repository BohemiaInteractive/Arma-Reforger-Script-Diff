class SCR_ResourceNameUtils
{
	//------------------------------------------------------------------------------------------------
	//! Gets the prefab the entity uses
	//! \param[in] entity Instance of which to get the prefab name
	//! \return the resource name of the prefab or empty string if no prefab was used or entity is invalid
	static ResourceName GetPrefabName(IEntity entity)
	{
		if (!entity || !entity.GetPrefabData())
			return ResourceName.Empty;

		return SCR_BaseContainerTools.GetPrefabResourceName(entity.GetPrefabData().GetPrefab());
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the GUID of a prefab
	static string GetPrefabGUID(ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return ResourceName.Empty;

		return prefab.Substring(1, 16);
	}
}
