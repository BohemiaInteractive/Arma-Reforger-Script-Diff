//! Manager for non-faction specific entity catalogs as well as getters for faction specific catalogs
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Manager for holding non-faction enities and to getter functions for entities from factions")]
class SCR_EntityCatalogManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_EntityCatalogManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "List of non-faction related Entity catalogs. Each holds a list of entity Prefab and data of a given type. Catalogs of the same type are merged into one. Note this array is moved to a map on init and set to null")]
	protected ref array<ref SCR_EntityCatalog> m_aEntityCatalogs;

	//~ Catalog map for quicker obtaining the catalog using EEntityCatalogType
	protected ref map<EEntityCatalogType, ref SCR_EntityCatalog> m_mEntityCatalogs = new map<EEntityCatalogType, ref SCR_EntityCatalog>();

	//~ Faction manager ref
	protected SCR_FactionManager m_FactionManager;

	//~ Instance
	protected static SCR_EntityCatalogManagerComponent s_Instance;

	protected static ref ScriptInvokerVoid s_OnEntityCatalogInitialized;
	
	protected bool m_bInitDone;

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_EntityCatalogManagerComponent GetInstance()
	{		
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerVoid GetOnEntityCatalogInitialized()
	{
		if (!s_OnEntityCatalogInitialized)
			s_OnEntityCatalogInitialized = new ScriptInvokerVoid();

		return s_OnEntityCatalogInitialized;
	}

	//======================================== GET CATALOG ========================================\\

	//------------------------------------------------------------------------------------------------
	//! Get Entity catalog of specific type not part of a faction
	//! The catalog contains all entities part of the specific type not part of a faction
	//! \param[in] catalogType Type to get catalog of
	//! \param[in] printNotFound True will print a warning if the given category was not found
	//! \return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	SCR_EntityCatalog GetEntityCatalogOfType(EEntityCatalogType catalogType, bool printNotFound = true)
	{
		//~ Init not yet called so initialize the factionless catalogs
		if (!m_bInitDone)
			Init();
		
		//~ Get catalog
		SCR_EntityCatalog entityCatalog = m_mEntityCatalogs.Get(catalogType);

		if (entityCatalog)
			return entityCatalog;

		//~ No data found
		if (printNotFound)
			Print(string.Format("'SCR_EntityCatalogManagerComponent' trying to get entity list of type '%1' but there is no catalog with that type.", typename.EnumToString(EEntityCatalogType, catalogType)), LogLevel.WARNING);
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Entity catalog of specific type linked to given faction
	//! The catalog contains all entities part of the faction of that specific type
	//! \param[in] catalogType Type to get catalog of
	//! \param[in] factionKey FactionKey of faction
	//! \param[in] printNotFound True will print a warning if the given category was not found
	//! \return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	SCR_EntityCatalog GetFactionEntityCatalogOfType(EEntityCatalogType catalogType, FactionKey factionKey, bool printNotFound = true)
	{
		if (!m_FactionManager)
			return null;

		SCR_Faction faction = SCR_Faction.Cast(m_FactionManager.GetFactionByKey(factionKey));
		if (!faction)
		{
			Print(string.Format("'SCR_EntityCatalogManagerComponent', GetFactionEntityCatalogOfType could not get Catalog as faction '%1' is invalid or does not exist in current game", factionKey), LogLevel.ERROR);
			return null;
		}

		return GetFactionEntityCatalogOfType(catalogType, faction, printNotFound);
	}

	//------------------------------------------------------------------------------------------------
	//! Get Entity catalog of specific type linked to given faction
	//! The catalog contains all entities part of the faction of that specific type
	//! \param[in] catalogType Type to get catalog of
	//! \param[in] faction Faction to find
	//! \param[in] printNotFound True will print a warning if the given category was not found
	//! \return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	SCR_EntityCatalog GetFactionEntityCatalogOfType(EEntityCatalogType catalogType, notnull SCR_Faction faction, bool printNotFound = true)
	{
		return faction.GetFactionEntityCatalogOfType(catalogType, printNotFound);
	}

	//======================================== GET ALL CATALOGS ========================================\\

	//------------------------------------------------------------------------------------------------
	//! Get all entity catalogs not part of a faction
	//! The catalogs contain all entities not part of a faction
	//! \param[out] List of all catalogs within the faction
	//! \return List size
	int GetAllEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs)
	{
		outEntityCatalogs.Clear();
		foreach (SCR_EntityCatalog entityCatalog : m_mEntityCatalogs)
		{
			outEntityCatalogs.Insert(entityCatalog);
		}

		return outEntityCatalogs.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get all entity catalogs within the faction
	//! The catalogs contain all entities part of the faction
	//! \param[out] List of all catalogs within the faction
	//! \param[in] factionKey key of faction to get all catalogs from
	//! \return List size
	//!
	int GetAllFactionEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs, FactionKey factionKey)
	{
		if (!m_FactionManager)
			return 0;

		SCR_Faction faction = SCR_Faction.Cast(m_FactionManager.GetFactionByKey(factionKey));
		if (!faction)
		{
			Print(string.Format("'SCR_EntityCatalogManagerComponent', GetAllFactionEntityCatalogs could not get Catalog as faction '%1' is invalid or does not exist in current game", factionKey), LogLevel.ERROR);
			return 0;
		}

		return GetAllFactionEntityCatalogs(outEntityCatalogs, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Get all entity catalogs within the faction
	//! The catalogs contain all entities part of the faction
	//! \param[out] List of all catalogs within the faction
	//! \param[in] faction faction to get all catalogs from
	//! \return List size
	int GetAllFactionEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs, notnull SCR_Faction faction)
	{
		return faction.GetAllFactionEntityCatalogs(outEntityCatalogs);
	}

	//======================================== GET SPECIFIC ENTRY WITH PREFAB ========================================\\

	//------------------------------------------------------------------------------------------------
	//! Return entry with specific prefab within general catalog
	//! Ignores Disabled Entries
	//! Note this search can be somewhat extensive
	//! \param[in] catalogType Catalog type to get entry in
	//! \param[in] prefabToFind Prefab the entry has that you are looking for
	//! \return Found Entry, can be null if not found
	SCR_EntityCatalogEntry GetEntryWithPrefabFromCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind)
	{
		//~ No prefab given
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		//~ Get catalog
		SCR_EntityCatalog catalog = GetEntityCatalogOfType(catalogType);
		if (!catalog)
			return null;

		//~ Try and find prefab
		return catalog.GetEntryWithPrefab(prefabToFind);
	}

	//------------------------------------------------------------------------------------------------
	//! Return entry with specific prefab within faction catalog
	//! Ignores Disabled Entries
	//! Note this search can be somewhat extensive
	//! \param[in] catalogType Catalog type to get entry in
	//! \param[in] prefabToFind Prefab the entry has that you are looking for
	//! \param[in] faction faction to find entry in
	//! \return Found Entry, can be null if not found
	SCR_EntityCatalogEntry GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind, notnull SCR_Faction faction)
	{
		//~ No prefab given
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		//~ Get catalog
		SCR_EntityCatalog catalog = GetFactionEntityCatalogOfType(catalogType, faction);
		if (!catalog)
			return null;

		//~ Try and find prefab
		return catalog.GetEntryWithPrefab(prefabToFind);
	}

	//------------------------------------------------------------------------------------------------
	//! Return entry with specific prefab within general or faction catalog
	//! Ignores Disabled Entries
	//! Note this search can be somewhat extensive
	//! \param[in] catalogType Catalog type to get entry in
	//! \param[in] prefabToFind Prefab the entry has that you are looking for
	//! \param[in] prioritizeGeneralCatalog If true will search in general catalog then faction catalog. False to search the other way around
	//! \param[in] faction faction to find entry in along with general catalog
	//! \return Found Entry, can be null if not found
	SCR_EntityCatalogEntry GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind, notnull SCR_Faction faction, bool prioritizeGeneralCatalog = false)
	{
		//~ No prefab given
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		SCR_EntityCatalogEntry foundEntry;

		//~ Search in faction first
		if (!prioritizeGeneralCatalog)
		{
			//~ Search in faction
			foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, faction);
			if (foundEntry)
				return foundEntry;

			//~ Search in general
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;
		}
		//~ Search in general catalog first
		else
		{
			//~ Search in faction
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;

			//~ Search in general
			foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, faction);
			if (foundEntry)
				return foundEntry;
		}

		//~ Was not found
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return entry with specific prefab within general or ANY faction catalog
	//! Note this search can be quite extensive!
	//! Ignores Disabled Entries
	//! \param[in] catalogType Catalog type to get entry in
	//! \param[in] prefabToFind Prefab the entry has that you are looking for
	//! \param[in] prioritizeGeneralOfPriorityFaction If true will search in general catalog then faction catalogs. False it will first search priorityFaction (if any) than General than any other faction
	//! \param[in] priorityFaction If given it will first try and get the entry from this faction before searching all the other factions
	//! \return Found Entry, can be null if not found
	SCR_EntityCatalogEntry GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind, SCR_Faction priorityFaction = null, bool prioritizeGeneralOfPriorityFaction = false)
	{
		SCR_EntityCatalogEntry foundEntry;

		if (!prioritizeGeneralOfPriorityFaction)
		{
			//~ Search in priority faction first
			if (priorityFaction)
			{
				foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, priorityFaction);
				if (foundEntry)
					return foundEntry;
			}

			//~ Search in general
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;
		}
		else
		{
			//~ Search in general first
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;

			//~ Search in priority faction
			if (priorityFaction)
			{
				foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, priorityFaction);
				if (foundEntry)
					return foundEntry;
			}
		}

		if (!m_FactionManager)
			return null;

		//~ Search all faction (ignoring priorityFaction) and try to find the prefab
		array<Faction> factions = {};
		m_FactionManager.GetFactionsList(factions);

		foreach (Faction faction : factions)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);

			//~ SCR_Faction not found or already searched
			if (!scrFaction || scrFaction == priorityFaction)
				continue;

			//~ Try and find the prefab
			foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, scrFaction);
			if (foundEntry)
				return foundEntry;
		}

		return null;
	}
		
	//======================================== SPAWNER ========================================\\
		//--------------------------------- Get List of valid prefabs for spawner ---------------------------------\\
	/*!
	Get all prefabs that have the spawner data, the given labels and are valid in the editor mode for all factions
	\param catalogType Type to catalog to get prefabs from
	\param editorMode Editor mode to get valid entries from
	\param[out] filteredPrefabsList Filltered array of valid prefabs
	\param includedLabels A list of labels the entity needs all/any to have. Can be null if any of the other arrays are filled
	\param excludedLabels A list of labels the entity CANNOT have ANY of. Can be null if any of the other arrays are filled
	\param needsAllIncludedLabels If true included List all needs to be true, if false any needs to be true
	\param getFactionLessPrefabs If true than it will also get the prefabs not assigned to any faction
	\return Size of found prefabs
	*/
	int GetFilteredEditorPrefabsOfAllFactions(EEntityCatalogType catalogType, EEditorMode editorMode, notnull out array<ResourceName> filteredPrefabsList, array<EEditableEntityLabel> includedLabels = null, array<EEditableEntityLabel> excludedLabels = null, bool needsAllIncludedLabels = true, bool getFactionLessPrefabs = false)
	{
		filteredPrefabsList.Clear();
		
		array<Faction> factions = {};
		
		//~ Get sorted factions
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (delegateFactionManager)
		{
			SCR_SortedArray<SCR_EditableFactionComponent> sortedDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
			int count = delegateFactionManager.GetSortedFactionDelegates(sortedDelegates);
			
			//~ Get sorted factions
			for(int i = 0; i < count; ++i)
			{
				factions.Insert(sortedDelegates.Get(i).GetFaction());
			}
			
		}
		//~ No delegate found so get factions from faction manager
		else if (m_FactionManager)
		{
			m_FactionManager.GetFactionsList(factions);
		}
		//~ Could not find faction nor faction manager
		else 
		{
			return 0;
		}
	
		array<ResourceName> filteredPrefabsOfFaction = {};
		SCR_Faction scrFaction;
		
		//~ Get filtered prefabs of each faction
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			GetFilteredEditorPrefabs(catalogType, editorMode, scrFaction, filteredPrefabsOfFaction, includedLabels, excludedLabels, needsAllIncludedLabels);
			filteredPrefabsList.InsertAll(filteredPrefabsOfFaction);
		}
		
		//~ Get filtered prefabs of factionless entries
		if (getFactionLessPrefabs)
		{
			GetFilteredEditorPrefabs(catalogType, editorMode, null, filteredPrefabsOfFaction, includedLabels, excludedLabels, needsAllIncludedLabels);
			filteredPrefabsList.InsertAll(filteredPrefabsOfFaction);
		}
		
		return filteredPrefabsList.Count();
	}
	
	//--------------------------------- Get List of valid prefabs for spawner ---------------------------------\\
	/*!
	Get all prefabs that have the spawner data, the given labels and are valid in the editor mode
	\param catalogType Type to catalog to get prefabs from
	\param editorMode Editor mode to get valid entries from
	\param faction Faction (Optional) to get valid prefabs from
	\param[out] filteredPrefabsList Filltered array of valid prefabs
	\param includedLabels A list of labels the entity needs all/any to have. Can be null if any of the other arrays are filled
	\param excludedLabels A list of labels the entity CANNOT have ANY of. Can be null if any of the other arrays are filled
	\param needsAllIncludedLabels If true included List all needs to be true, if false any needs to be true
	\return Size of found prefabs
	*/
	int GetFilteredEditorPrefabs(EEntityCatalogType catalogType, EEditorMode editorMode, SCR_Faction faction, notnull out array<ResourceName> filteredPrefabsList, array<EEditableEntityLabel> includedLabels = null, array<EEditableEntityLabel> excludedLabels = null, bool needsAllIncludedLabels = true)
	{
		SCR_EntityCatalog catalog;
		filteredPrefabsList.Clear();
		
		//~ Get the catalog from faction or factionless
		if (faction)
			catalog = GetFactionEntityCatalogOfType(catalogType, faction);
		else 
			catalog = GetEntityCatalogOfType(catalogType);
		
		//~ No catalog found
		if (!catalog)
			return 0;
		
		array<SCR_EntityCatalogEntry> filteredEntityList = {};
		array<typename> filterClassArray = {};
		filterClassArray.Insert(SCR_EntityCatalogEditorData);
		
		catalog.GetFullFilteredEntityList(filteredEntityList, includedLabels, excludedLabels, filterClassArray, needsAllIncludedLabels: needsAllIncludedLabels);
		
		SCR_EntityCatalogEditorData editorData;
		
		//~ Get all valid prefabs for mode
		foreach (SCR_EntityCatalogEntry entry : filteredEntityList)
		{
			editorData = SCR_EntityCatalogEditorData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogEditorData));
			
			//~ Ignore entries not valid in current editor mode
			if (!editorData.IsValidInEditorMode(editorMode))
				continue;
		
			//~ Add the prefab to the array
			filteredPrefabsList.Insert(entry.GetPrefab());
		}
		
		return filteredPrefabsList.Count();
	}
	
	//======================================== ARSENAL ========================================\\	

	//------------------------------------------------------------------------------------------------
	//! Get all arsenal items configured on the faction
	//! Values taken from General Catalog ITEM
	//! \param[out] arsenalItems output array
	//! \param[in] typeFilter filter for Types (-1 is ignore filter)
	//! \param[in] modeFilter filter for Modes (-1 is ignore filter)
	//! \param[in] arsenalGameModeType The Arsenal Game mode type which dictates which items are available (-1 is unrestricted)
	//! \param[in] requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	//! \return False if no config is set and when 0 items are configured
	bool GetArsenalItems(out array<SCR_ArsenalItem> arsenalItems, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, SCR_EArsenalGameModeType arsenalGameModeType = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		arsenalItems.Clear();
		
		SCR_EntityCatalog itemCatalog = GetEntityCatalogOfType(EEntityCatalogType.ITEM);
		if (!itemCatalog)
			return false;
		
		return GetArsenalItems(arsenalItems, itemCatalog, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all arsenal items configured on the faction
	//! Values taken from Faction Catalog ITEM
	//! \param[out] arsenalItems output array
	//! \param[in] faction faction to get items from
	//! \param[in] typeFilter filter for Types (-1 is ignore filter)
	//! \param[in] modeFilter filter for Modes (-1 is ignore filter)
	//! \param[in] arsenalGameModeType The Arsenal Game mode type which dictates which items are available (-1 is unrestricted)
	//! \param[in] requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	//! \return bool False if no config is set and when 0 items are configured
	bool GetFactionArsenalItems(out array<SCR_ArsenalItem> arsenalItems, SCR_Faction faction, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, SCR_EArsenalGameModeType arsenalGameModeType = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{		
		arsenalItems.Clear();
		
		SCR_EntityCatalog itemCatalog = GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM, faction);
		if (!itemCatalog)
			return false;
		
		return GetArsenalItems(arsenalItems, itemCatalog, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets a filtered list of arsenal items
	//! \param[out] arsenalItems output array
	//! \param[in] faction faction to get items from
	//! \param[in] typeFilter filter for Types (-1 is ignore filter)
	//! \param[in] modeFilter filter for Modes (-1 is ignore filter)
	//! \param[in] arsenalGameModeType The Arsenal Game mode type which dictates which items are available (-1 is unrestricted)
	//! \param[in] requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	//! \return bool False if no config is set and when 0 items are configured
	protected bool GetArsenalItems(out array<SCR_ArsenalItem> arsenalItems, notnull SCR_EntityCatalog itemCatalog, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, SCR_EArsenalGameModeType arsenalGameModeType = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		array<SCR_EntityCatalogEntry> arsenalEntries = {};
		array<SCR_BaseEntityCatalogData> arsenalDataList = {};
		itemCatalog.GetEntityListWithData(SCR_ArsenalItem, arsenalEntries, arsenalDataList);
		
		SCR_ArsenalItem arsenalItem;
		
		foreach (SCR_BaseEntityCatalogData arsenalData: arsenalDataList)
		{
			//~ Get Arsenal data
			arsenalItem = SCR_ArsenalItem.Cast(arsenalData);
			
			//~ Does not have type so skip
			if (typeFilter != -1 && !SCR_Enum.HasPartialFlag(arsenalItem.GetItemType(), typeFilter))
				continue;
			
			//~ Does not have mode so skip
			if (modeFilter != -1 && !SCR_Enum.HasPartialFlag(arsenalItem.GetItemMode(), modeFilter))
				continue;
			
			//~ Get if has display data, ignore if it doesn't
			if (requiresDisplayType != -1 && !arsenalItem.GetDisplayDataOfType(requiresDisplayType))
				continue;

			//~ Only get items that have the given arsenal game mode type flag. (-1 means no restrictions)
			if (arsenalGameModeType > 0 && !SCR_Enum.HasFlag(arsenalItem.GetArsenalGameModeTypes(), arsenalGameModeType))
				continue;
			
			arsenalItems.Insert(arsenalItem);
		}
		
		return !arsenalItems.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all arsenal items configured in any faction and the non-faction catalogs. Note this is a very intensive seach
	//! Values taken from Faction Catalog ITEM
	//! \param[out] allArsenalItems output array
	//! \param[in] typeFilter filter for Types (-1 is ignore filter)
	//! \param[in] modeFilter filter for Modes (-1 is ignore filter)
	//! \param[in] arsenalGameModeType The Arsenal Game mode type which dictates which items are available (-1 is unrestricted)
	//! \param[in] requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	//! \return int count of items found
	int GetAllArsenalItems(out array<SCR_ArsenalItem> allArsenalItems, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, SCR_EArsenalGameModeType arsenalGameModeType = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		allArsenalItems.Clear();
		
		array<Faction> factions = {};
		m_FactionManager.GetFactionsList(factions);
		SCR_Faction scrFaction;
		array<SCR_ArsenalItem> arsenalItems = {};
		
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			GetFactionArsenalItems(arsenalItems, scrFaction, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
			allArsenalItems.InsertAll(arsenalItems);
		}
		
		GetArsenalItems(arsenalItems, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
		allArsenalItems.InsertAll(arsenalItems);
		
		return allArsenalItems.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get arsenal items filtered by SCR_EArsenalItemType filter, caches values
	//! \param[in] typeFilter Combined flags for available items for this faction (RIFLE, MAGAZINE, EQUIPMENT, RADIOBACKPACK etc.)
	//! \param[in] modeFilter Things like AMMO and CONSUMABLE
	//! \param[in] faction If empty will take non-faction data else will take the data from the faction
	//! \param[in] arsenalGameModeType The Arsenal Game mode type which dictates which items are available (-1 is unrestricted)
	//! \param[in] requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	//! \return array with availabe arsenal items of give filter types
	array<SCR_ArsenalItem> GetFilteredArsenalItems(SCR_EArsenalItemType typeFilter, SCR_EArsenalItemMode modeFilter, SCR_EArsenalGameModeType arsenalGameModeType, SCR_Faction faction = null, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		array<SCR_ArsenalItem> refFilteredItems = {};
		array<SCR_ArsenalItem> filteredItems = {};
		
		if (faction)
			GetFactionArsenalItems(refFilteredItems, faction, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
		else 
			GetArsenalItems(refFilteredItems, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
		
		foreach (SCR_ArsenalItem item : refFilteredItems)
		{
			filteredItems.Insert(item);
		}
			
		return filteredItems;
	}
	
	//======================================== IDENTITY ITEM ========================================\\
	//------------------------------------------------------------------------------------------------
	//! \return Identity item for given character. Will take faction affiliation comp and try to find a valid identity item for the character
	ResourceName GetIdentityItemForCharacter(ChimeraCharacter character)
	{
		if (!character)
			return GetDefaultIdentityItem();
		
		FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComp)
			return GetDefaultIdentityItem();
		
		SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
		if (!faction)
			return GetDefaultIdentityItem();
		
		SCR_EntityCatalog catalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
		if (!catalog)
			return GetDefaultIdentityItem();
		
		array<SCR_EntityCatalogEntry> filteredEntityList = {};
		
		catalog.GetEntityListWithData(SCR_EntityCatalogIdentityItemData, filteredEntityList);
		if (filteredEntityList.IsEmpty())
			return string.Empty;
		
		if (filteredEntityList.Count() > 1)
			Print("SCR_EntityCatalogManagerComponent: GetIdentityItemForCharacter Multiple Identity Items found for faction " + faction.GetFactionKey() + "! Only the first found will be used");
		
		return filteredEntityList[0].GetPrefab();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Default Identity item which is not tied to any faction
	ResourceName GetDefaultIdentityItem()
	{
		SCR_EntityCatalog catalog = GetEntityCatalogOfType(EEntityCatalogType.ITEM);
		if (!catalog)
			return string.Empty;
		
		array<SCR_EntityCatalogEntry> filteredEntityList = {};
		
		catalog.GetEntityListWithData(SCR_EntityCatalogIdentityItemData, filteredEntityList);
		if (filteredEntityList.IsEmpty())
			return string.Empty;
		
		if (filteredEntityList.Count() > 0)
			Print("SCR_EntityCatalogManagerComponent: GetIdentityItemForCharacter Multiple Identity Items found for factionless list! Only the first found will be used");
		
		return filteredEntityList[0].GetPrefab();
	}

	//======================================== INIT ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Init Catalog lists to move the arrays into a map for faster processing
	//! \param[in] entityCatalogArray Array of entity catalog to move into the map
	//! \param[in] entityCatalogMap the map to move the catalogs to. Catalogs with the same type will be merged
	static void InitCatalogs(notnull array<ref SCR_EntityCatalog> entityCatalogArray, notnull map<EEntityCatalogType, ref SCR_EntityCatalog> entityCatalogMap)
	{
		SCR_EntityCatalog foundCatalog;
		
		//~ Process all catalogs
		foreach (SCR_EntityCatalog entityCatalog : entityCatalogArray)
		{			
			//~ Catalog not part of map so add it
			if (!entityCatalogMap.Find(entityCatalog.GetCatalogType(), foundCatalog))
				entityCatalogMap.Insert(entityCatalog.GetCatalogType(), entityCatalog);
			//~ Catalog is part of map so merge them
			else 
				foundCatalog.MergeCatalogs(entityCatalog);
		}
		
		//~ Init the entries after merge
		foreach (SCR_EntityCatalog entityCatalog : entityCatalogArray)
		{		
			//~ Init the catalog
			entityCatalog.InitCatalog();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_bInitDone)
			Init();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		m_bInitDone = true;
		
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!m_FactionManager)
			Debug.Error2("SCR_EntityCatalogManagerComponent", "Could not find SCR_FactionManager, this is required for many the Getter Functions!");

		//~ Init the catalog
		InitCatalogs(m_aEntityCatalogs, m_mEntityCatalogs);
		
		//~ Clear array as no longer needed
		m_aEntityCatalogs = null;

		if (s_OnEntityCatalogInitialized)
			s_OnEntityCatalogInitialized.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		if (s_Instance)
		{
			Print("More than one 'SCR_EntityCatalogManagerComponent' excist in the world! Make sure there is only 1!", LogLevel.ERROR);
			return;
		}

		//~ Set instance
		s_Instance = this;

		SetEventMask(owner, EntityEvent.INIT);
	}
}
