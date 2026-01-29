[BaseContainerProps(configRoot:true)]
class SCR_ArsenalItemListConfig
{
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_ArsenalItemStandalone> m_aArsenalItems;
	
	protected ref map<SCR_EArsenalItemType, ref array<SCR_ArsenalItem>> m_mArsenalItemsByType = new map<SCR_EArsenalItemType, ref array<SCR_ArsenalItem>>();
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] arsenalItems
	//! \return
	bool GetArsenalItems(out array<ref SCR_ArsenalItemStandalone> arsenalItems)
	{
		arsenalItems = m_aArsenalItems;
		return arsenalItems != null && !arsenalItems.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] prefab
	//! \param[out] itemType
	//! \param[out] itemMode
	//! \return
	bool GetItemTypeAndModeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemType, out SCR_EArsenalItemMode itemMode)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefab))
			return false;
		
		for (int i = 0, count = m_aArsenalItems.Count(); i < count; i++)
		{
			SCR_ArsenalItem item = m_aArsenalItems[i];
			if (item.GetItemResourceName() == prefab)
			{
				itemType = item.GetItemType();
				itemMode =  item.GetItemMode();
				return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get arsenal items filtered by SCR_EArsenalItemType filter, caches values
	//! \param[in] filter Combined flags for available items for this faction (RIFLE, MAGAZINE, EQUIPMENT, RADIOBACKPACK etc.)
	//! \param[in] typeFilter
	//! \param[in] modeFilter
	//! \param[in] requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	//! \param[in] checkFaction If a faction is given it will check if the item is part of the faction and if not will filter out the item
	//! \return array with availabe arsenal items of give filter types
	array<SCR_ArsenalItem> GetFilteredArsenalItems(SCR_EArsenalItemType typeFilter, SCR_EArsenalItemMode modeFilter, EArsenalItemDisplayType requiresDisplayType = -1, SCR_Faction checkFaction = null)
	{
		array<SCR_ArsenalItem> filteredItems = new array<SCR_ArsenalItem>();
		
		array<SCR_ArsenalItem> itemsByType = m_mArsenalItemsByType.Get(typeFilter);
		if (!itemsByType)
		{
			itemsByType = new array<SCR_ArsenalItem>();
			array<ref SCR_ArsenalItemStandalone> availableArsenalItems;
			if (GetArsenalItems(availableArsenalItems))
			{				
				for (int i = 0, count = availableArsenalItems.Count(); i < count; i++)
				{
					//~ Ignore if no required display data				
					if (requiresDisplayType != -1 && !availableArsenalItems[i].GetDisplayDataOfType(requiresDisplayType))
						continue;
					
					if (availableArsenalItems[i].GetItemType() & typeFilter)
					{
						itemsByType.Insert(availableArsenalItems[i]);
					}
				}
				
				m_mArsenalItemsByType.Insert(typeFilter, itemsByType);
			}
		}
		
		//~ Get the catalog if a faction is given
		SCR_EntityCatalog catalog;
		if (checkFaction)
			catalog = checkFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM, false);
		
		foreach	(SCR_ArsenalItem item : itemsByType)
		{
			if (item.GetItemMode() & modeFilter)
			{
				//~ Check if item is part of faction
				if (catalog && !catalog.GetEntryWithPrefab(item.GetItemResourceName()))
					continue;

				filteredItems.Insert(item);
			}
		}

		return filteredItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] prefab
	//! \param[out] itemMode
	//! \return
	bool GetItemModeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemMode)
	{
		for (int i = 0, count = m_aArsenalItems.Count(); i < count; i++)
		{
			SCR_ArsenalItem item = m_aArsenalItems[i];
			if (item.GetItemResourceName() == prefab)
			{
				itemMode = item.GetItemMode();
				return true;
			}
		}
		return false;
	}
}
