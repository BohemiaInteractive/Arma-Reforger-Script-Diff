[BaseContainerProps()]
class SCR_BasePlayerLoadout
{
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetLoadoutResource()
	{
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetLoadoutName()
	{
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetLoadoutImageResource()
	{
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \return
	bool IsLoadoutAvailable(int playerId)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsLoadoutAvailableClient()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pOwner
	//! \param[in] playerId
	void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] loadout
	//! \param[in] playerId needed only for arsenal player loadout
	//! \return required rank of saved player loadout or catalog entry loadout
	static SCR_ECharacterRank GetLoadoutRequiredRank(notnull SCR_BasePlayerLoadout loadout, int playerId)
	{
		SCR_PlayerArsenalLoadout arsenalLoadout = SCR_PlayerArsenalLoadout.Cast(loadout);
		if (arsenalLoadout && playerId > 0)
			return arsenalLoadout.GetRequiredRank(playerId);

		ResourceName loadoutResource = loadout.GetLoadoutResource();
		if (!loadoutResource)
			return SCR_ECharacterRank.PRIVATE;

		Resource resource = Resource.Load(loadoutResource);
		if (!resource)
			return SCR_ECharacterRank.PRIVATE;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return SCR_ECharacterRank.PRIVATE;

		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName());
		if (!entry)
			return SCR_ECharacterRank.PRIVATE;

		SCR_EntityCatalogLoadoutData data = SCR_EntityCatalogLoadoutData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogLoadoutData));
		if (!data)
			return SCR_ECharacterRank.PRIVATE;

		return data.GetRequiredRank();
	}
}
