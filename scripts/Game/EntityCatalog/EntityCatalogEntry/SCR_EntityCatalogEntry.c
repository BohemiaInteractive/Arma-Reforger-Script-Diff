/**
Entity Entry within the SCR_EntityCatalog. Includes the Prefab and list of Data for each entry
*/
[BaseContainerProps(), BaseContainerCustomDoubleCheckIntResourceNameTitleField("m_bEnabled", "m_sEntityPrefab", 1, "%1", "DISABLED - %1")]
class SCR_EntityCatalogEntry
{
	[Attribute(desc: "Prefab of entity", UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sEntityPrefab;
	
	[Attribute("1", desc: "Allows to disable the Entity data. It will be removed from the array on init if it is disabled and can never be obtained. Used for specific gamemodes and modding.")]
	protected bool m_bEnabled;
	
	[Attribute(desc: "List of specific Data for the entity. Each element should be an unique class (Unless all but one are disabled). This list is static and should not be changed in runtime.")]
	protected ref array<ref SCR_BaseEntityCatalogData> m_aEntityDataList;
	
	//~ Index within catalog. Set by Catalog
	protected int m_iCatalogIndex = -1;
	
	//~ UI Info that is obtained from the Prefab data.
	protected ref SCR_UIInfo m_EditableEntityUiInfo;
	
	//~ Reference to parent catalog this entry is part of
	protected SCR_EntityCatalog m_CatalogParent;
	
	//======================================== GETTERS AND SETTERS ========================================\\
	//--------------------------------- Is Enabled ---------------------------------\\
	/*!
	If is Enabled
	\return If enabled or disabled
	*/
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//--------------------------------- Get Index ---------------------------------\\
	/*!
	Get Index of entry within Catalog
	\return index
	*/
	int GetCatalogIndex()
	{
		return m_iCatalogIndex;
	}
	
	//--------------------------------- Get Prefab ---------------------------------\\
	/*!
	Get Prefab data
	\return Prefab data
	*/
	ResourceName GetPrefab()
	{
		return m_sEntityPrefab;
	}
	
	//--------------------------------- Get Catalog Parent ---------------------------------\\
	/*!
	Get Catalog parent the entity entry is in
	\return Catalog parent
	*/
	SCR_EntityCatalog GetCatalogParent()
	{
		return m_CatalogParent;
	}
	
	//======================================== UI INFO ========================================\\
	//--------------------------------- Get UI Info ---------------------------------\\
	/*!
	Get UI info
	Prefab needs to be an editable entity or have a custom UI info logic
	Note that if UIInfo is NULL then the autotest will fail
	\return Ui info
	*/
	SCR_UIInfo GetEntityUiInfo()
	{
		return m_EditableEntityUiInfo;
	}
	
	//--------------------------------- Get Name ---------------------------------\\
	/*!
	Get Name of entity
	Prefab needs to be an editable entity
	\return Name, empty string if no uiinfo was found
	*/
	LocalizedString GetEntityName()
	{
		if (!GetEntityUiInfo())
			return string.Empty;
		
		return GetEntityUiInfo().GetName();
	}
	
	//======================================== LABELS ========================================\\
	//--------------------------------- Get all Labels ---------------------------------\\
	/*!
	Get a list of all editable entity lables
	Prefab needs to be an editable entity
	\param[out] List of all labels on entity
	\return length of lable array
	*/
	int GetEditableEntityLabels(notnull out array<EEditableEntityLabel> editableEntityLables)
	{
		editableEntityLables.Clear();
		
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(GetEntityUiInfo());
		if (!editableUiInfo)
			return 0;
		
		return editableUiInfo.GetEntityLabels(editableEntityLables);
	}
	
	//--------------------------------- Has Label ---------------------------------\\
	/*!
	Check if entity has given label
	Prefab needs to be an editable entity
	\param editableEntityLabel Label to check if entity has it
	\return True if entity has the label
	*/
	bool HasEditableEntityLabel(EEditableEntityLabel editableEntityLabel)
	{
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(GetEntityUiInfo());
		if (!editableUiInfo)
			return false;
		
		return editableUiInfo.HasEntityLabel(editableEntityLabel);
	}
	
	//--------------------------------- Has Any label ---------------------------------\\
	/*!
	Check if entity has any one of the given labels
	Prefab needs to be an editable entity
	\param editableEntityLables Labels to check if entity has any of them
	\return True if entity has any of the label
	*/
	bool HasAnyEditableEntityLabels(notnull array<EEditableEntityLabel> editableEntityLables)
	{
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(GetEntityUiInfo());
		if (!editableUiInfo)
			return false;
	
		foreach (EEditableEntityLabel label: editableEntityLables)
		{
			if (editableUiInfo.HasEntityLabel(label))
				return true;
		}
		
		return false;
	}
	
	//--------------------------------- Has all labels ---------------------------------\\
	/*!
	Check if entity has ALL of the given labels
	Prefab needs to be an editable entity
	\param editableEntityLables Labels to check if entity has ALL of them
	\return True if entity has ALL of the label
	*/
	bool HasAllEditableEntityLabels(notnull array<EEditableEntityLabel> editableEntityLables)
	{
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(GetEntityUiInfo());
		if (!editableUiInfo)
			return false;
		
		foreach (EEditableEntityLabel label: editableEntityLables)
		{
			if (!editableUiInfo.HasEntityLabel(label))
				return false;
		}
		
		return true;
	}
	
	//======================================== DATA LISTS ========================================\\
	//--------------------------------- Get Entity Data from type ---------------------------------\\
	/*!
	Get prefab entity Data of type.
	Ignores disabled Data's
	\param dataType class of Data type you with to obtain
	\return Entity Data of given type. Null if not found
	*/
	SCR_BaseEntityCatalogData GetEntityDataOfType(typename dataType)
	{
		foreach (SCR_BaseEntityCatalogData entityData: m_aEntityDataList)
		{
			//~ Check if correct type and not disabled
			if (entityData.IsEnabled() && entityData.Type().IsInherited(dataType))
				return entityData;
		}
		
		return null;
	}
	
	//--------------------------------- Get Entity Data list ---------------------------------\\
	/*!
	Get list of entity Data of all types
	Ignores Disabled Elements
	\param[out] Array of enabled entity Data list
	\return List size
	*/
	int GetEntityDataList(notnull out array<SCR_BaseEntityCatalogData> entityDataList)
	{
		//~ Clear Given list
		entityDataList.Clear();
		
		//~ Copy list
		foreach (SCR_BaseEntityCatalogData entityData: m_aEntityDataList)
		{
			//~ Ignore if disabled
			if (entityData.IsEnabled())
				entityDataList.Insert(entityData);
		}
			
		return entityDataList.Count();
	}
	
	//--------------------------------- Has Entity Data from type ---------------------------------\\
	/*!
	Check if has the entity data of the given type
	Ignores disabled Data's
	\param dataType class of Data type you want to check
	\return True if the entity has an enabled data of given type
	*/
	bool HasEntityDataOfType(typename dataType)
	{
		foreach (SCR_BaseEntityCatalogData entityData: m_aEntityDataList)
		{
			//~ Check if correct type and not disabled
			if (entityData.IsEnabled() && entityData.Type().IsInherited(dataType))
				return true;
		}
		
		return false;
	}
	
	/*!
	Check if has ALL the entity datas of the given types
	Ignores disabled Data's
	\param dataTypes classes of Data types you want to check
	\return True if the entity has ALL the enabled data of given types
	*/
	bool HasAllEntityDataOfTypes(array<typename> dataTypes)
	{
		//~ Check for each data type if type exist. Return false if it does not
		foreach (typename dataType: dataTypes)
		{
			if (!HasEntityDataOfType(dataType))
				return false;
		}
		
		return true;
	}
	
	/*!
	Check if has ANY the entity datas of the given types
	Ignores disabled Data's
	\param dataTypes classes of Data types you want to check
	\return True if the entity has ANY the enabled data of given types
	*/
	bool HasAnyEntityDataOfTypes(array<typename> dataTypes)
	{
		//~ Check for each data type if type exist. Return true if it has any in the given list
		foreach (typename dataType: dataTypes)
		{
			if (HasEntityDataOfType(dataType))
				return true;
		}
		
		return false;
	}
	
	//======================================== INIT ========================================\\
	//--------------------------------- Class Specific Init ---------------------------------\\
	/*!
	Init for general entry. Setting index and called class specific init
	Only set by Catalog parent on init. Should not be overwritten
	\param catalog Catalog entry is in
	\param index Index within catalog
	*/
	void InitEntry(notnull SCR_EntityCatalog catalog, int catalogIndex)
	{
		//~ Set idexes for easy getting the entry from the catalog
		if (GetCatalogIndex() == -1)
			m_iCatalogIndex = catalogIndex;
		
		//~ Set Catalog Reference
		m_CatalogParent = catalog;
		
		//~ Class Specific Init
		ClassSpecificInit();
			
		//~ Init the data found in the list (Do last)
		array<SCR_BaseEntityCatalogData> entityDataList = {};
		GetEntityDataList(entityDataList);
		foreach (SCR_BaseEntityCatalogData data: entityDataList)
		{
			data.InitData(this);
		}
	}
	
	//--------------------------------- Class Specific Post Init ---------------------------------\\
	/*!
	Post Init for general entry. Called one frame after init is called
	Only set by Catalog parent on post init. Should not be overwritten
	\param[i] catalog Catalog entry is in
	*/
	void PostInitEntry(SCR_EntityCatalog catalog)
	{
		array<SCR_BaseEntityCatalogData> entityDataList = {};
		GetEntityDataList(entityDataList);
		foreach (SCR_BaseEntityCatalogData data: entityDataList)
		{
			data.PostInitData(this);
		}
	}
	
	//--------------------------------- Class Specific Init ---------------------------------\\
	//~ This is a class specific init. Override and call any specific init functionality that should only be executed in the specific class
	//~ Gets UIInfo from editable entity in base class
	protected void ClassSpecificInit()
	{
		//~ Already has Init called
		if (m_EditableEntityUiInfo != null)
			return;
		
		//~ Load Prefab
		Resource entityPrefab = Resource.Load(GetPrefab());
		if (!entityPrefab)
			return;
		
		//~ Get entity source
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		if (!entitySource)
			return;
		
		//~ Get EditableEntity component
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
		if (!editableEntitySource)
			return;
		
		//~ Save UI Info found on Editable Enity
		m_EditableEntityUiInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
	}
};
