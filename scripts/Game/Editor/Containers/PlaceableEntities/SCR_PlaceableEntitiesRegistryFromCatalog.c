[BaseContainerProps(configRoot: true), BaseContainerCatalogPlacableEntitiesTitleField("m_eEditorMode", "m_eCatalogFactionType", "m_CatalogTypes")]
/** @ingroup Editor_Containers_Root
*/

/*!
Allows Content browser to take entities from the catalog. It fills in the Prefab list from the catalog
*/
class SCR_PlaceableEntitiesRegistryFromCatalog : SCR_PlaceableEntitiesRegistry
{		
	[Attribute("0", desc: "The editor mode that the catalog entry has assigned", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditorMode))]
	protected EEditorMode m_eEditorMode;
	
	[Attribute("0", desc: "If the entries will be from factions, factionless or both", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ECatalogFactionType))]
	protected SCR_ECatalogFactionType m_eCatalogFactionType;
	
	[Attribute("0", UIWidgets.SearchComboBox, "", enums: ParamEnumArray.FromEnum(EEntityCatalogType))]
	protected ref array<ref EEntityCatalogType> m_CatalogTypes;
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (m_CatalogTypes.IsEmpty())
			return;		
		
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
		{
			Print("SCR_PlaceableEntitiesRegistryFromCatalog cannot find SCR_EntityCatalogManagerComponent so can not initialize entries within Catalogs!", LogLevel.WARNING);
			return;
		}

		foreach (EEntityCatalogType catalogType: m_CatalogTypes)
		{
			ProcessCatalog(catalogType, catalogManager);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessCatalog(EEntityCatalogType catalogType, notnull SCR_EntityCatalogManagerComponent catalogManager)
	{
		array<ResourceName> prefabs = {};
		
		//~ Get correct filtered entities
		if (m_eCatalogFactionType == SCR_ECatalogFactionType.FACTION_AND_FACTIONLESS)
			catalogManager.GetFilteredEditorPrefabsOfAllFactions(catalogType, m_eEditorMode, prefabs, getFactionLessPrefabs: true);
		else if (m_eCatalogFactionType == SCR_ECatalogFactionType.FACTIONS_ONLY)
			catalogManager.GetFilteredEditorPrefabsOfAllFactions(catalogType, m_eEditorMode, prefabs, getFactionLessPrefabs: false);
		else if (m_eCatalogFactionType == SCR_ECatalogFactionType.FACTIONLESS_ONLY)
			catalogManager.GetFilteredEditorPrefabs(catalogType, m_eEditorMode, null, prefabs);
		
		AddPrefabs(prefabs);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlaceableEntitiesRegistryFromCatalog()
	{		
		if (SCR_Global.IsEditMode()) 
			return;
		
		Init();
	}
};

enum SCR_ECatalogFactionType
{
	FACTION_AND_FACTIONLESS = 0, //!< Get entries from faction and factionless catalogs
	FACTIONS_ONLY, //!< Get entries from factions only
	FACTIONLESS_ONLY, //!< Get entries from factionless Only
}

class BaseContainerCatalogPlacableEntitiesTitleField : BaseContainerCustomTitle
{
	protected string m_sEditorModeVar;
	protected string m_sFactionTypeVar;
	protected string m_sCatalogTypesListVar;
	
	//------------------------------------------------------------------------------------------------
	void BaseContainerCatalogPlacableEntitiesTitleField(string editorModeVar, string factionTypeVar, string catalogTypesListVar)
	{
		m_sEditorModeVar = editorModeVar;
		m_sFactionTypeVar = factionTypeVar;
		m_sCatalogTypesListVar = catalogTypesListVar;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		EEditorMode editorMode;
		
		if (!source.Get(m_sEditorModeVar, editorMode))
			return false;
		
		SCR_ECatalogFactionType factionType;
		
		if (!source.Get(m_sFactionTypeVar, factionType))
			return false;
		
		array<EEntityCatalogType> catalogTypes = {};
		
		if (!source.Get(m_sCatalogTypesListVar, catalogTypes))
			return false;
		
		title = "Catalog Getter (" + typename.EnumToString(EEditorMode, editorMode) + ")";
		
		string categories = "MISSING CATALOGS!";
		
		if (!catalogTypes.IsEmpty())
		{
			categories = string.Empty;
			
			for (int i = 0, count = catalogTypes.Count(); i < count; i++)
			{
				if (i > 0)
					categories += " & ";
				
				categories += typename.EnumToString(EEntityCatalogType, catalogTypes[i]);
			}
		}
		
		title += " - " + categories + " - " + typename.EnumToString(SCR_ECatalogFactionType, factionType);
		
		return true;
	}
}
