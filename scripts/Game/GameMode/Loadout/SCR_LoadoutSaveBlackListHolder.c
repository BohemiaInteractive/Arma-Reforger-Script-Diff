[BaseContainerProps(configRoot: true)]
class SCR_LoadoutSaveBlackListHolder
{
	[Attribute(desc: "List of blacklist entries. Each blacklist becomes a button in attributes which the GM can enable and/or disable and is a set of items/item types which cannot be saved in loadout if the blacklist is activate")]
	protected ref array<ref SCR_LoadoutSaveBlackList> m_LoadoutSaveBlackLists;
	
	[Attribute(desc: "List of Items that are black listed regardless of the other black list rules. Players with the item in their inventory can never save their loadout")]
	protected ref array<ref SCR_LoadoutSaveBlackListItem> m_aSaveBlackListItems;
	
	//------------------------------------------------------------------------------------------------
	//! Init black lists, this will make sure each blacklist has a list to items that are blacklisted
	void Init()
	{
		for (int i = m_LoadoutSaveBlackLists.Count() - 1; i >= 0; i--)
		{
			//~ Removed disabled lists
			if (!m_LoadoutSaveBlackLists[i].IsEnabled() || !m_LoadoutSaveBlackLists[i].Init())
			{
				m_LoadoutSaveBlackLists.RemoveOrdered(i);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get List of all black lists
	//! \param[out] loadoutSaveBlackLists List of black lists which contain lists of prefabs that can never be saved if the blacklist is enabled
	//! \return Count of blacklist lists
	int GetLoadoutSaveBlackLists(out notnull array<SCR_LoadoutSaveBlackList> loadoutSaveBlackLists)
	{
		loadoutSaveBlackLists.Clear();
		
		foreach (SCR_LoadoutSaveBlackList blackList : m_LoadoutSaveBlackLists)
		{
			loadoutSaveBlackLists.Insert(blackList);
		}
		
		return loadoutSaveBlackLists.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Count of blacklist lists
	int GetBlackListsCount()
	{
		return m_LoadoutSaveBlackLists.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if given prefab is in an enabled blacklist.
	//! \param[in] prefab Prefab to check
	//! \return True if prefab is blacklisted
	bool IsPrefabBlacklisted(ResourceName prefab)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefab))
			return true;
		
		//~ Check if the specific item is blacklisted
		foreach (SCR_LoadoutSaveBlackListItem blackListItem : m_aSaveBlackListItems)
		{
			if (blackListItem.m_bEnabled && blackListItem.m_sSaveBlackListItem == prefab)
				return true;
		}
		
		//~ Check if the item type is black listed
		foreach (SCR_LoadoutSaveBlackList blackList : m_LoadoutSaveBlackLists)
		{
			if (!blackList || !blackList.IsActive())
				continue;
			
			//~ Prefab in blacklist so not allowed to save
			if (blackList.DoesBlackListContaintPrefab(prefab))
				return true;
		}
		
		//~ Prefab not in an enabled blacklist
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if black list is active
	//! \param[in] index Index of black list
	//! \return True if active
	bool IsBlackListActive(int index)
	{
		if (!m_LoadoutSaveBlackLists.IsIndexValid(index))
		{
			Print("'SCR_LoadoutSaveBlackListHolder' function 'IsBlackListEnabled' given index is invalid", LogLevel.ERROR);
			return false;
		}
			
		return m_LoadoutSaveBlackLists[index].IsActive();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set black list active
	//! \param[in] index Index of black list
	//! \param[in] active True if setting blacklist active
	void SetBlackListActive(int index, bool active)
	{
		if (!m_LoadoutSaveBlackLists.IsIndexValid(index))
		{
			Print("'SCR_LoadoutSaveBlackListHolder' function 'SetBlackListEnabled' given index is invalid", LogLevel.ERROR);
			return;
		}
		
		m_LoadoutSaveBlackLists[index].SetActive(active);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get black lists active returning an ordered active array
	//! \param[in] orderedBlackListActive Ordered active array
	//! \return Count of ordered active array
	int GetOrderedBlackListsActive(out notnull array<bool> orderedBlackListActive)
	{
		orderedBlackListActive.Clear();
		
		array<SCR_LoadoutSaveBlackList> loadoutSaveBlackLists = {};
		GetLoadoutSaveBlackLists(loadoutSaveBlackLists);
		
		foreach (SCR_LoadoutSaveBlackList blackList : loadoutSaveBlackLists)
		{
			orderedBlackListActive.Insert(blackList.IsActive());
		}
		
		return orderedBlackListActive.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set black lists active using ordered active array
	//! \param[in] orderedBlackListActive Ordered active array
	void SetOrderedBlackListsActive(notnull array<bool> orderedBlackListActive)
	{
		for (int i = 0, count = orderedBlackListActive.Count(); i < count; i++)
		{
			SetBlackListActive(i, orderedBlackListActive[i]);
		}
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_UIInfo")]
class SCR_LoadoutSaveBlackList
{
	[Attribute("1", "If blacklist is enabled or not. Disabled blacklists are removed on init and is used for mods")]
	protected bool m_bEnabled;
	
	[Attribute("0", "If blacklist is active or not. If active than the items within the prefab list can never be saved in loadouts")]
	protected bool m_bBlackListActive;
	
	[Attribute(desc: "Ui info to display in editor to set blacklist saving or not")]
	protected ref SCR_UIInfo m_UIInfo;
	
	[Attribute(desc: "List of entities that are blacklisted and can never be saved when player saves their loadout. Is cleared on init and any prefabs are added to a faster set.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ref array<ResourceName> m_aBlackListedPrefabs;
	
	[Attribute(desc: "List of item type and mode that cannot be saved in loadout. This list is cleared on init and any prefabs that fit the set up are added to a faster set.")]
	protected ref array<ref SCR_LoadoutSaveBlackListItemType> m_BlackListedItemTypes;
	
	//~ List of resource names that are black listed
	protected ref set<ResourceName> m_BlackListedPrefabs = new set<ResourceName>();
	
	//------------------------------------------------------------------------------------------------
	//! Init black list, this will get all items that should be blacklisted and adds it to one unified set (Server only)
	//! \return Returns true if init is successfull, returns false if not
	bool Init()
	{
		foreach (ResourceName prefab : m_aBlackListedPrefabs)
		{
			m_BlackListedPrefabs.Insert(prefab);
		}
		
		//~ Delete the array list as it is no longer needed
		m_aBlackListedPrefabs = null;
		
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		
		if (!catalogManager && !m_BlackListedItemTypes.IsEmpty())
		{
			Debug.Error2("SCR_LoadoutSaveBlackList", "Black lists need the 'SCR_EntityCatalogManagerComponent' to set the the prefabs who are blacklisted. The blacklist will be disabled if the custom prefab blacklist is empty");
			m_BlackListedItemTypes = null;
			
			//~ No items in the black list so set disabled
			if (m_BlackListedPrefabs.IsEmpty())
			{
				m_bEnabled = false;
				return false;
			}
				
			return true;
		}
		
		array<SCR_ArsenalItem> filteredArsenalItems = {};
		
		//~ Find arsenal items for each filter (Intensive search)
		foreach (SCR_LoadoutSaveBlackListItemType blackList : m_BlackListedItemTypes)
		{
			//~ Get list of arsenal items either with or without mode
			if (blackList.m_bIgnoreItemMode)
				catalogManager.GetAllArsenalItems(filteredArsenalItems, blackList.m_eItemType);
			else 
				catalogManager.GetAllArsenalItems(filteredArsenalItems, blackList.m_eItemType, blackList.m_eItemMode);
			
			//~ Get the Prefab resourcename form arsenal item
			foreach (SCR_ArsenalItem arsenalItem : filteredArsenalItems)
			{
				m_BlackListedPrefabs.Insert(arsenalItem.GetItemResourceName());
			}
		}
	
		//~ Delete the array list as no longer needed
		m_BlackListedItemTypes = null;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get if black list is enabled. Disabled blacklists are removed on init
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get if black list is active
	bool IsActive()
	{
		return m_bBlackListActive;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set black list enabled
	//! \param[in] enabled True if setting blacklist enabled
	void SetActive(bool active)
	{
		m_bBlackListActive = active
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does black list contain prefab
	//! \param[in] prefab Prefab to check
	//! \return True if it contains, false if it does not
	bool DoesBlackListContaintPrefab(ResourceName prefab)
	{
		return m_BlackListedPrefabs.Contains(prefab);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get UIInfo
	SCR_UIInfo GetUIInfo()
	{
		if (!m_UIInfo)
			return new SCR_UIInfo();
		
		return m_UIInfo;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EArsenalItemType, "m_eItemType")]
class SCR_LoadoutSaveBlackListItemType
{
	[Attribute("2", UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	SCR_EArsenalItemType m_eItemType;
	
	[Attribute("-1", UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	SCR_EArsenalItemMode m_eItemMode;
	
	[Attribute("1", desc: "If true than the ItemMode will be ignored and any items that have the type regardless of the mode will be blacklisted", UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	bool m_bIgnoreItemMode;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EArsenalItemType, "m_eItemType")]
class SCR_LoadoutSaveBlackListItem
{
	[Attribute(desc: "Item prefabs that are not allowed to be saved in the players loadout", UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sSaveBlackListItem;
	
	[Attribute("1", desc: "If the item is actively blacklisted")]
	bool m_bEnabled;
}
