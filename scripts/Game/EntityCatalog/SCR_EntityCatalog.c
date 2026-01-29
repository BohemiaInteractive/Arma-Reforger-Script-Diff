	/*
	//~ USE CASE EXAMPLE
	protected void Example()
	{
		//-------- Get from manager --------\\

		//~ The SCR_EntityCatalogManagerComponent on the GameMode can get general catalogs as well as faction catalogs
			// It also allows to get Entry from Prefab with extensive functions
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return;
		
		SCR_EntityCatalog entityCatalog = entityCatalogManager.GetEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		if (!entityCatalog)
			return;

		//~ Can also get it from faction (Though getting it directly from faction is slighty cheaper)
		SCR_EntityCatalog entityCatalog = entityCatalogManager.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE, "US");
		if (!entityCatalog)
			return;

		//~ There area also many direct search functions if you have the prefab. (Note that it can be quite performance intensive so use when needed)
		//~ See below for more info about SCR_EntityCatalogEntry
		//~ Faction can be given if you first want to check a specific faction as it is more likly it is in there (To save an more extensive search)
		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.VEHICLE, ResourceName prefabToFind, SCR_Faction);

		//-------

		//-------- Get directly from faction --------\\

		//~ You can get the catalog from faction
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey("US"));
		if (!faction)
			return;
		
		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		if (!entityCatalog)
			return;

		//-------

		//-------- Get list of entity data --------\\

		//~ You can simply get all entries regardless if they have any specific data class
			array<SCR_EntityCatalogEntry> entityList = {};
			int count = entityCatalog.GetEntityList(entityList);
			Print(count);

		//-------

		//-------- Filtering Entries list - Specific Labels --------\\
		The system is compatible with Editable Entities and it is possible to get the name of the entity as well as lables
		(For non-editable entities you can set the same variables using the SCR_EntityCatalogEntryNonEditable class)


		//~ Get a list of all entries within catalog that have the given Label
			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetEntityListWithLabel(EEditableEntityLabel.FACTION_US, filteredSpawnerEntityList);
			Print(count);

		//~ Get a list of all entries within catalog that do not have the given Label
			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetEntityListExcludingLabel(EEditableEntityLabel.FACTION_USSR, filteredSpawnerEntityList);
			Print(count);

		//~You can also do a full filter. This will get you all the enties that have all/any of the include labels and none of the exclude labels. 
			array<EEditableEntityLabel> includedLabels = {}; //~ Note: Inclusive can be null if exclusive is filled
			includedLabels.Insert(EEditableEntityLabel.FACTION_US);
			
			array<EEditableEntityLabel> excludedLabels = {}; //~ Note: exclusive can be null if Inclusive is filled
			excludedLabels.Insert(EEditableEntityLabel.FACTION_USSR);

			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetFullFilteredEntityListWithLabels(filteredSpawnerEntityList, includedLabels, excludedLabels);
			Print(count);

		//-------

		//-------- Filtering Entries list - Specific Data types --------\\
		The system is also able to filter entries on specific data type in the data array (Data needs to be inherented from SCR_BaseEntityCatalogData)

		//~ Get a list of all entries within catalog that have the given data class (In this cause it is the SCR_EntityCatalogSpawnerData class)
			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetEntityListWithData(SCR_EntityCatalogSpawnerData, filteredSpawnerEntityList);
			Print(count);

		//~ You can get a list that excludes a specific data class
			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetEntityListExcludingLabel(SCR_EntityCatalogSpawnerData, filteredSpawnerEntityList);
			Print(count);

		//~You can also do a full filter. This will get you all the enties that have all/any of the include data type and none of the exclude Data type. 
			array<typename> includedDataClasses = {}; //~ Note: Inclusive can be null if exclusive is filled
			includedDataClasses.Insert(SCR_EntityCatalogSpawnerData);
			
			array<typename> excludedDataClasses = {}; //~ Note: exclusive can be null if Inclusive is filled
			excludedDataClasses.Insert(SCR_EntityCatalogLoadoutData);

			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetFullFilteredEntityListWithData(filteredSpawnerEntityList, includedDataClasses, excludedDataClasses);
			Print(count);
			

		//-------

		//-------- Filtering Entries list - Labels and data --------\\
		Lastly you can do an ultimate filter with both labels and data.

			array<EEditableEntityLabel> includedLabels = {}; //~ Note: Inclusive can be null if exclusive is filled
			includedLabels.Insert(EEditableEntityLabel.FACTION_US);
			
			array<EEditableEntityLabel> excludedLabels = {}; //~ Note: exclusive can be null if Inclusive is filled
			excludedLabels.Insert(EEditableEntityLabel.FACTION_USSR);

			array<typename> includedDataClasses = {}; //~ Note: Inclusive can be null if exclusive is filled
			includedDataClasses.Insert(SCR_EntityCatalogSpawnerData);
			
			array<typename> excludedDataClasses = {}; //~ Note: exclusive can be null if Inclusive is filled
			excludedDataClasses.Insert(SCR_EntityCatalogLoadoutData);

			array<SCR_EntityCatalogEntry> filteredSpawnerEntityList = {};
			int count = entityCatalog.GetFullFilteredEntityList(filteredSpawnerEntityList, includedLabels, excludedLabels, includedDataClasses, excludedDataClasses);
			Print(count);

		//-------


		//-------- Entry Entity Functions --------\\
		In the example we simply get the index from the list above. With it we can quicly obtain the information we want
		
		//~ Getting the Catalog Index from filtered entity
			int catalogIndex = filteredSpawnerEntityList[someIndex].GetCatalogIndex();
		
		//~ You now got the entry directly from the catalog
			SCR_EntityCatalogEntry catalogEntry = entityCatalog.GetCatalogEntry(catalogIndex);

		//~ This allows you to Get the prefab
			ResourceName prefab = catalogEntry.GetPrefab();
			Print(prefab);
		
		//~ Get Data 
			SCR_EntityCatalogSpawnerData entitySpawnerData = SCR_EntityCatalogSpawnerData.Cast(catalogEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData)); //~ Cast to Desired Type
			Print(entitySpawnerData);

		//~ Get Entity UI Info
			SCR_UIInfo uiInfo = catalogEntry.GetEntityUiInfo();
			Print(uiInfo);
		
		//~ Get Entity Name
			LocalizedString name = catalogEntry.GetEntityName();
			Print(name);

		//~ Get Entity Labels
			array<EEditableEntityLabel> labels = {};
			int count = catalogEntry.GetEditableEntityLabels(labels);
		
		//~ Do various label functions
			 catalogEntry.HasEditableEntityLabel();
			 catalogEntry.HasAnyEditableEntityLabels();
			 catalogEntry.HasAllEditableEntityLabels();

		//~ Do various Data functions
			 catalogEntry.GetEntityDataOfType();
			 catalogEntry.HasEntityDataOfType();
			 catalogEntry.HasAllEntityDataOfTypes();
			 catalogEntry.HasAnyEntityDataOfTypes();
		
		//-------
	*/


/**
Catalog that holds faction entity lists of a specific entity type
*/
[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomEntityCatalogCatalog(EEntityCatalogType, "m_eEntityCatalogType", "m_aEntityEntryList", "m_aMultiLists")]
class SCR_EntityCatalog
{
	[Attribute("0", desc: "Type of the Catalog", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEntityCatalogType))]
	protected EEntityCatalogType m_eEntityCatalogType;

	[Attribute(desc: "List of all entities of the given type (Note that this list can still be used if using the 'SCR_EntityCatalogMultiList' Class. All entries in the multi lists will be merged into this one on init including the entries already in it)")]
	protected ref array<ref SCR_EntityCatalogEntry> m_aEntityEntryList;
	
	//~ A map of all prefabs with the linked index for quicker getting the prefab data
	protected ref map<ResourceName, int> m_mPrefabIndexes = new map<ResourceName, int>();
	
	//======================================== TYPE ========================================\\
	/*!
	Get Data type
	\return Returns Catalog type of holder
	*/
	EEntityCatalogType GetCatalogType()
	{
		return m_eEntityCatalogType;
	}
	
	//======================================== ENTITY ENTRY GETTER ========================================\\
	//--------------------------------- Get Entity List ---------------------------------\\
	/*!
	Get list of entities witin Catalog
	Ignores Disabled Entries
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param[out] entityList Array of enabled entity list
	\return List size
	*/
	int GetEntityList(notnull out array<SCR_EntityCatalogEntry> entityList)
	{
		//~ Clear Given list
		entityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			entityList.Insert(entityEntry);
		}
			
		return entityList.Count();
	}
	
	/*!
	Return entry with specific prefab
	Ignores Disabled Entries
	\param prefabToFind Prefab the entry has that you are looking for
	\return Found Entry, can be null if not found
	*/
	SCR_EntityCatalogEntry GetEntryWithPrefab(ResourceName prefabToFind)
	{
		int index;
		
		//~ Entry not found
		if (!m_mPrefabIndexes.Find(prefabToFind, index))
			return null;
		
		//~ Somehow an invalid index
		if (!m_aEntityEntryList.IsIndexValid(index))
			return null;
		
		//~ Return entry
		return m_aEntityEntryList[index];
	}
	
		
	//======================================== DIRECT ENTRY GETTERS WITH INDEX ========================================\\
	//--------------------------------- Get Catalog Entry from Index ---------------------------------\\
	/*!
	Get Catalog Entry of index.
	Can ignores disabled entries
	\param Index of entry within list
	\return Catalog Entry. Null if entry is disabled
	*/
	SCR_EntityCatalogEntry GetCatalogEntry(int index)
	{
		//~ Invalid index
		if (!m_aEntityEntryList.IsIndexValid(index))
		{
			Print(string.Format("'SCR_EntityCatalog' Function: 'GetCatalogEntry()'. index: '%1' is invalid for catalog type '%2'.", index, typename.EnumToString(EEntityCatalogType, m_eEntityCatalogType)), LogLevel.ERROR); 			
			return null;
		}
		
		//~ Return Entry
		return m_aEntityEntryList[index];
	}
	
	//======================================== ENTITY ENTRY GETTER WITH LABELS ========================================\\
	//--------------------------------- Get Entity List with Label ---------------------------------\\
	/*!
	Get list of entities witin Catalog which have a specific Label
	Ignores Disabled Entries and disabled Data types
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param label Label you want the entry to have
	\param[out] filteredEntityList Array of enabled entity list with the specific label
	\return List size
	*/
	int GetEntityListWithLabel(EEditableEntityLabel label, notnull out array<SCR_EntityCatalogEntry> filteredEntityList)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			//~ Does not have label
			if (!entityEntry.HasEditableEntityLabel(label))
				continue;
			
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//--------------------------------- Get Entity List WITHOUT Fata type ---------------------------------\\
	/*!
	Get list of entities witin Catalog EXCLUDING those with specific label
	Ignores Disabled Entries as well
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param excludinglabel Label you want the entry NOT have
	\param[out] filteredEntityList Array of enabled entity list without the specific label
	\return List size
	*/
	int GetEntityListExcludingLabel(EEditableEntityLabel excludinglabel, notnull out array<SCR_EntityCatalogEntry> filteredEntityList)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			//~ HAS the label so do not add it to the list
			if (entityEntry.HasEditableEntityLabel(excludinglabel))
				continue;
			
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//--------------------------------- Get Full Filtered Entity List with Labels ---------------------------------\\
	/*!
	Get list of entities that all contain all/any the included labels and NONE exclude labels
	Ignores Disabled Entries as well
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param[out] filteredEntityList Filltered array of enabled entity
	\param includedLabels A list of labels the entity needs all/any to have. Can be null if exclusive is filled
	\param excludedLabels A list of labels the entity CANNOT have ANY of. Can be null if inclusive is filled
	\param needsAllIncluded If true included List all needs to be true, if false any needs to be true
	\return List size
	*/
	int GetFullFilteredEntityListWithLabels(notnull out array<SCR_EntityCatalogEntry> filteredEntityList, array<EEditableEntityLabel> includedLabels = null, array<EEditableEntityLabel> excludedLabels = null, bool needsAllIncluded = true)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			//~ If entity has any of the given exclude continue to next
			if (excludedLabels != null && !excludedLabels.IsEmpty() && entityEntry.HasAnyEditableEntityLabels(excludedLabels))
				continue;
			
			//~ Check included labels
			if (includedLabels != null && !includedLabels.IsEmpty())
			{
				//~ Needs all included
				if (needsAllIncluded)
				{
					if (!entityEntry.HasAllEditableEntityLabels(includedLabels))
						continue;
				}
				//~ Needs any included
				else 
				{
					if (!entityEntry.HasAnyEditableEntityLabels(includedLabels))
						continue;
				}
			}
				
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//======================================== ENTITY ENTRY GETTER WITH DATA TYPES ========================================\\
	//--------------------------------- Get Entity List with Data type ---------------------------------\\
	/*!
	Get list of entities witin Catalog which have a specific Data type (Needs to be inherent from SCR_BaseEntityCatalogData)
	Ignores Disabled Entries and disabled Data types
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param dataClass class of Data type you want the data to have in order to be added to the list (Needs to be inherent from SCR_BaseEntityCatalogData)
	\param[out] filteredEntityList Array of enabled entity list with specific Data type
	\param[out] dataList optionally you can directly get the data list if array given
	\return List size
	*/
	int GetEntityListWithData(typename dataClass, notnull out array<SCR_EntityCatalogEntry> filteredEntityList, out array<SCR_BaseEntityCatalogData> dataList = null)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ If returning datalist also clear
		if (dataList)
			dataList.Clear();
		
		SCR_BaseEntityCatalogData data;
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			data = entityEntry.GetEntityDataOfType(dataClass);

			if (!data)
				continue;
			
			if (dataList)
				dataList.Insert(data);
			
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//--------------------------------- Get Entity List WITHOUT Data type ---------------------------------\\
	/*!
	Get list of entities witin Catalog EXCLUDING those with specific Data type (Needs to be inherent from SCR_BaseEntityCatalogData)
	Ignores Disabled Entries as well
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param excludingDataClass class of Data type you DON'T want the data from (Needs to be inherent from SCR_BaseEntityCatalogData)
	\param[out] filteredEntityList Array of enabled entity list without the specific Data type
	\return List size
	*/
	int GetEntityListExcludingData(typename excludingDataClass, notnull out array<SCR_EntityCatalogEntry> filteredEntityList)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			//~ HAS the data type so do not add it to the list
			if (entityEntry.HasEntityDataOfType(excludingDataClass))
				continue;
			
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//--------------------------------- Get Full Filtered Entity List with Data Type ---------------------------------\\
	/*!
	Get list of entities that all contain all/any the included Data classes and NONE exclude data classes
	Ignores Disabled Entries as well
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param[out] filteredEntityList Filltered array of enabled entity
	\param includedDataClasses  A list of classes the entity needs all/any to have. Can be null if exclusive is filled (Needs to be inherent from SCR_BaseEntityCatalogData)
	\param excludedDataClasses A list of classes the entity CANNOT have ANY of. Can be null if inclusive is filled (Needs to be inherent from SCR_BaseEntityCatalogData)
	\param needsAllIncluded If true included List all needs to be true, if false any needs to be true
	\return List size
	*/
	int GetFullFilteredEntityListWithData(notnull out array<SCR_EntityCatalogEntry> filteredEntityList, array<typename> includedDataClasses = null, array<typename> excludedDataClasses = null, bool needsAllIncluded = true)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			//~ If entity has any of the given exclude continue to next
			if (excludedDataClasses != null && !excludedDataClasses.IsEmpty() && entityEntry.HasAnyEntityDataOfTypes(excludedDataClasses))
				continue;
			
			//~ Check included labels
			if (includedDataClasses != null && !includedDataClasses.IsEmpty())
			{
				//~ Needs all included
				if (needsAllIncluded)
				{
					if (!entityEntry.HasAllEntityDataOfTypes(includedDataClasses))
						continue;
				}
				//~ Needs any included
				else 
				{
					if (!entityEntry.HasAnyEntityDataOfTypes(includedDataClasses))
						continue;
				}
			}
			
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//======================================== Get full filtered list ========================================\\
	//--------------------------------- Get Full Filtered Entity List ---------------------------------\\
	/*!
	Get list of entities that all contain all/any the included labels and/or Data classes and NONE exclude labels and/or data classes
	Ignores Disabled Entries as well
	Sometimes it is quicker to get the index (entityList[i].GetCatalogIndex()) and use the GetCatalogEntry() directly as it saves looping through the list and you don't need to save a ref to the list you got
	\param[out] filteredEntityList Filltered array of enabled entity
	\param includedLabels A list of labels the entity needs all/any to have. Can be null if any of the other arrays are filled
	\param excludedLabels A list of labels the entity CANNOT have ANY of. Can be null if any of the other arrays are filled
	\param includedDataClasses A list of classes the entity ALL needs to have. Can be null if any of the other arrays are filled (Needs to be inherent from SCR_BaseEntityCatalogData)
	\param excludedDataClasses A list of classes the entity CANNOT have ANY of. Can be null if any of the other arrays are filled (Needs to be inherent from SCR_BaseEntityCatalogData)
	\param needsAllIncludedLabels If true included List all needs to be true, if false any needs to be true
	\param needsAllIncludedClasses If true included List all needs to be true, if false any needs to be true
	\return List size
	*/
	int GetFullFilteredEntityList(notnull out array<SCR_EntityCatalogEntry> filteredEntityList, array<EEditableEntityLabel> includedLabels = null, array<EEditableEntityLabel> excludedLabels = null, array<typename> includedDataClasses = null, array<typename> excludedDataClasses = null, bool needsAllIncludedLabels = true, bool needsAllIncludedClasses = true)
	{
		//~ Clear Given list
		filteredEntityList.Clear();
		
		//~ Copy list
		foreach (SCR_EntityCatalogEntry entityEntry: m_aEntityEntryList)
		{
			//~ If entity has any of the given exclude labels continue to next
			if (excludedLabels != null && !excludedLabels.IsEmpty() && entityEntry.HasAnyEditableEntityLabels(excludedLabels))
				continue;
			
			//~ If entity has any of the given exclude classes continue to next
			if (excludedDataClasses != null && !excludedDataClasses.IsEmpty() && entityEntry.HasAnyEntityDataOfTypes(excludedDataClasses))
				continue;
			
			//~ Check included labels
			if (includedLabels != null && !includedLabels.IsEmpty())
			{
				//~ Needs all included
				if (needsAllIncludedLabels)
				{
					if (!entityEntry.HasAllEditableEntityLabels(includedLabels))
						continue;
				}
				//~ Needs any indluded
				else 
				{
					if (!entityEntry.HasAnyEditableEntityLabels(includedLabels))
						continue;
				}
			}
			
			//~ If entity has none of the given include continue to next
			if (includedDataClasses != null && !includedDataClasses.IsEmpty())
			{
				if (needsAllIncludedClasses)
				{
					if (!entityEntry.HasAllEntityDataOfTypes(includedDataClasses))
						continue;
				}
				else 
				{
					if (!entityEntry.HasAnyEntityDataOfTypes(includedDataClasses))
						continue;
				}
			}
			
			//~ Add to list
			filteredEntityList.Insert(entityEntry);
		}
			
		return filteredEntityList.Count();
	}
	
	//======================================== MERGE CATALOGS ========================================\\
	/*!
	Merge the given catalog into this catalog.
	Used on init to create one coherent list of each catalog type
	\param catalogToMerge Given catalog to merge into this one
	*/
	void MergeCatalogs(notnull SCR_EntityCatalog catalogToMerge)
	{
		array<SCR_EntityCatalogEntry> entityList = {};
		catalogToMerge.GetEntityList(entityList);
		
		//~ Add the entry
		foreach(SCR_EntityCatalogEntry entry : entityList)
		{
			m_aEntityEntryList.Insert(entry);
		}
		
		//~ Merge multi lists
		SCR_EntityCatalogMultiList multiListClass = SCR_EntityCatalogMultiList.Cast(catalogToMerge);
		if (multiListClass)
		{
			array <SCR_EntityCatalogMultiListEntry> multiLists = {};
			multiListClass.GetMultiList(multiLists);
			
			foreach (SCR_EntityCatalogMultiListEntry multiList : multiLists)
			{
				foreach (SCR_EntityCatalogEntry entry : multiList.m_aEntities)
				{
					m_aEntityEntryList.Insert(entry);
				}
			}
		}
		
		catalogToMerge.ClearCatalogOnMerge();
	}
	
	//---- REFACTOR NOTE START: Has a call later as it needs to initialize all other lists need to be initialized before the post init should be called ----
	//======================================== INIT ========================================\\
	void InitCatalog()
	{		
		//~ Init data for entries		
		for (int i = m_aEntityEntryList.Count() - 1; i >= 0; i--)
		{
			//~ Remove disabled entries on init
			if (!m_aEntityEntryList[i].IsEnabled())
			{
				m_aEntityEntryList.RemoveOrdered(i);
				continue;
			}
			
			//~ Call init
			m_aEntityEntryList[i].InitEntry(this, i);
		}
		
		foreach (int idx, SCR_EntityCatalogEntry entry : m_aEntityEntryList)
		{
			m_mPrefabIndexes.Insert(entry.GetPrefab(), idx);
		}
		
		//~ Call post init one frame later for all entries to allow all Catalogs to be initialized before the post init is called on entries
		GetGame().GetCallqueue().CallLater(PostInitCatalog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PostInitCatalog()
	{
		//~ Post Init data for entries	
		foreach (SCR_EntityCatalogEntry entry : m_aEntityEntryList)
		{
			entry.PostInitEntry(this);
		}
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! Clear the catalog only executed on merge
	void ClearCatalogOnMerge()
	{
		m_aEntityEntryList.Clear();
	}
};


class SCR_BaseContainerCustomEntityCatalogCatalog : BaseContainerCustomTitle
{
	private typename m_EnumType;
	private string m_PropertyName;
	protected string m_sEntityListName;
	protected string m_sEntityMultiListName;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomEntityCatalogCatalog(typename enumType, string propertyName, string entityListName, string multiListName)
	{
		m_EnumType = enumType;
		m_PropertyName = propertyName;
		m_sEntityListName = entityListName;
		m_sEntityMultiListName = multiListName;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		bool hasError = false;
		
		int enumValue;
		if (!source.Get(m_PropertyName, enumValue))
		{
			return false;
		}
		
		int totalCount = 0;
		int enabledCount = 0;
		
		//~ Get entry count
		array<ref SCR_EntityCatalogEntry> entityList;
		if (!source.Get(m_sEntityListName, entityList))
			return false; 
		
		foreach (SCR_EntityCatalogEntry entry: entityList)
		{
			if (entry.IsEnabled())
			{
				enabledCount++;
				
				//~ Is Empty show error warning in title
				if (entry.GetPrefab().IsEmpty())
					hasError = true;
			}
				
			totalCount++;
		}
		
		array<ref SCR_EntityCatalogMultiListEntry> multiLists;
		if (source.Get(m_sEntityMultiListName, multiLists))
		{
			foreach (SCR_EntityCatalogMultiListEntry multiList: multiLists)
			{
				foreach (SCR_EntityCatalogEntry entry : multiList.m_aEntities)
				{
					if (entry.IsEnabled())
					{
						enabledCount++;
						
						//~ Is Empty show error warning in title
						if (entry.GetPrefab().IsEmpty())
							hasError = true;
					}

					totalCount++;
				}
			}
		}
		
		string format;
		//~ Does not have disabled entries
		if (enabledCount == totalCount)
			format = "%1 (%2)";
		//~ Has disabled entries
		else 
			format = "%1 (%3 of %2)";
		
		//~ Has empty prefabs
		if (hasError)
			format += " !!";

		title = string.Format(format, typename.EnumToString(m_EnumType, enumValue), totalCount, enabledCount);
		return true;
	}
};