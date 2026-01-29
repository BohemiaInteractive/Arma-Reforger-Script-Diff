/**
Entity Entry within the SCR_EntityCatalog. This is meant for NON-EDITABLE ENTITIES ONLY! For editable entities use SCR_EntityCatalogEntry!
*/
//[BaseContainerProps(), BaseContainerCustomDoubleCheckIntResourceNameTitleField("m_bEnabled", "m_sEntityPrefab", 1, "(Item) %1", "DISABLED - (Item) %1")]
[BaseContainerProps(), SCR_BaseContainerCustomInventoryCatalogEntry("m_sEntityPrefab", "m_aEntityDataList", "m_bEnabled")]
class SCR_EntityCatalogInventoryItem : SCR_EntityCatalogEntryNonEditable
{
	//--------------------------------- Class Specific Init ---------------------------------\\
	protected override void ClassSpecificInit()
	{
		if (m_EditableEntityUiInfo != null)
			return;
		
		Resource resource = Resource.Load(GetPrefab());
		if (!resource || !resource.IsValid())
			return;
		
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
			return;
		
		UIInfo uiInfo;
		
		//~ Get UI info from weapon
		IEntityComponentSource weaponEntitySource = SCR_ComponentHelper.GetWeaponComponentSource(entitySource);
		if (weaponEntitySource)
		{
			uiInfo = SCR_ComponentHelper.GetWeaponComponentInfo(weaponEntitySource);
			
			if (uiInfo)
			{
				m_EditableEntityUiInfo = SCR_UIInfo.CreateInfo(uiInfo);
				return;
			}
		}
		
		//~ Get UI info from item
		IEntityComponentSource inventoryEntitySource = SCR_ComponentHelper.GetInventoryItemComponentSource(entitySource);
		if (inventoryEntitySource)
		{
			SCR_ItemAttributeCollection inventoryEntityUiInfo = SCR_ComponentHelper.GetInventoryItemInfo(inventoryEntitySource);
			if (inventoryEntityUiInfo)
			{
				uiInfo = inventoryEntityUiInfo.GetUIInfo();
				
				if (uiInfo)
				{
					m_EditableEntityUiInfo = SCR_UIInfo.CreateInfo(uiInfo);
					return;
				}
			}
		}
	
		//~ No UI info found so create new invalid
		if (!m_EditableEntityUiInfo)
		{
			Print("Catalog Entry: '" + GetPrefab() + "' could not create UIinfo for inventory item. Non was found for weapon nor Inventory item data", LogLevel.WARNING);
			m_EditableEntityUiInfo = new SCR_UIInfo();
			m_EditableEntityUiInfo.CreateInfo("MISSING");
		}
	}
};

//~ Custom title to show Type of inventory
class SCR_BaseContainerCustomInventoryCatalogEntry : BaseContainerCustomTitle
{
	protected string m_sPrefabName;
	protected string m_sDataListName;
	protected string m_sEnabledName;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomInventoryCatalogEntry(string prefabName, string dataListName, string enabledName)
	{
		m_sPrefabName = prefabName;
		m_sDataListName = dataListName;
		m_sEnabledName = enabledName;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)	
	{	
		bool enabled;
		if (!source.Get(m_sEnabledName, enabled))
			return false;
		
		ResourceName path;
		if (!source.Get(m_sPrefabName, path))
			return false;
		
		if (!path.IsEmpty())
		{
			title = FilePath.StripPath(path.GetPath());
		}
		else 
		{
			title = "NO PREFAB"
		}
		
		array<ref SCR_BaseEntityCatalogData> entityDataList;
		if (!source.Get(m_sDataListName, entityDataList))
			return false; 
		
		string typeTitle, modeTitle;
		SCR_ArsenalItem arsenalItem;
		
		//~ Show specific type info if ArsenalItem
		foreach (SCR_BaseEntityCatalogData data: entityDataList)
		{
			arsenalItem = SCR_ArsenalItem.Cast(data);
			if (arsenalItem)
			{
				//~ Show type
				typeTitle = typename.EnumToString(SCR_EArsenalItemType, arsenalItem.GetItemType());
				title = title + "    ~    " + typeTitle;
				
				//~ Only show mode if not default
				if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.DEFAULT)
				{
					modeTitle = typename.EnumToString(SCR_EArsenalItemMode, arsenalItem.GetItemMode());
					title = title + " - " + modeTitle
				}
			}
		}
		
		//~ Disabled so do not show types
		if (!enabled)
			title = "DISABLED - " + title;
		
		return true; 
	}
};