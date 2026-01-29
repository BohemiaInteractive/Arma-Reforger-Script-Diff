/*
TODO:
-Set default class flags on root/base prefabs (currently setting only when IsVariableSetDirectly is true)
*/
//#define WE_LOGGING_ENABLED

[BaseContainerProps(configRoot: true)]
class WorldExporterConfig
{
	[Attribute(defvalue: "c:/Reforger/Data/Arland", desc: "Folder of the map to export")]
	string m_sExportMapDir;

	[Attribute(defvalue: "Assets/_SharedData", desc: "Relative paths to shared data to be copied over")]
	ref array<string> m_aCopyPathsMixed;

	[Attribute(defvalue: "edds;emat;gamemat", desc: "Shared data extensions, only these will be copied over")]
	string m_sCopyPathsMixedExtensions;

	[Attribute(defvalue: "", desc: "Paths to copy .xob files from")]
	ref TStringArray m_aModelPaths;

	[Attribute(defvalue: "Tree::StaticModelEntity", desc: "Classname to Classname conversion mapping during the export process. First class will be converted to the second class, the separator is ::")]
	ref TStringArray m_aClassMappings;

	[Attribute(defvalue: "RoadGeneratorEntity", desc: "Entities/Prefabs inheriting classes in this list are not converted, even if they match any other criteria")]
	ref TStringArray m_aEntityClassWhitelist;

	[Attribute(defvalue: "", desc: "Entities/Prefabs inheriting classes containing keywords from this list are converted to their corresponding classes")]
	ref TStringArray m_aEntityClassBlacklistKeywords;

	[Attribute(defvalue: "MeshObject", desc: "Components inheriting these classes in this list are not converted, even if they match any other criteria")]
	ref TStringArray m_aComponentClassWhitelist;

	[Attribute(defvalue: "export-cache", desc: "Path prefix added to the resources that are converted during the export process, typically prefabs")]
	string m_sExportCachePrefix;

	[Attribute(defvalue: "true", desc: "After export is finished, automatically delete temporary files created during the export")]
	bool m_bDeleteTempFiles;

	[Attribute(defvalue: "", desc: "Resources with these GUIDs will not be copied over")]
	ref TResourceNameArray m_aResourceExcludeGUIDs;

	[Attribute(defvalue: "", desc: "When cloning prefabs, ignore the ones coming from PrefabLibrary and alter prefab inheritance accordingly")]
	bool m_bSkipPrefabLibrary;
	
	[Attribute(defvalue: "false", desc: "Export source files along with the resource files. (will export FBX, TIF etc.. which are not used at run time)")]
	bool m_bExportSourceFiles;

	[Attribute(defvalue: "true", desc: "Export files belonging to map (terrain tiles, etc.)")]
	bool m_ExportMapData;

	[Attribute(defvalue: "$ArmaReforger", desc: "FIlesystem of the project we are exporting world from.")]
	string m_sFilesystem;

	[Attribute(desc: "Root resources to start reference crawling from.")]
	ref TResourceNameArray m_RootResources;

	bool IsValid()
	{
		if (m_aModelPaths == null || m_aModelPaths.Count() == 0)
			return false;
	
		if (m_aCopyPathsMixed == null || m_aCopyPathsMixed.Count() == 0)
			return false;
	
		if (m_sCopyPathsMixedExtensions.IsEmpty())
			return false;
	
		if (m_aClassMappings == null || m_aClassMappings.Count() == 0)
			return false;
	
		if (m_aEntityClassWhitelist == null || m_aEntityClassWhitelist.Count() == 0)
			return false;

		if (m_aEntityClassBlacklistKeywords == null || m_aEntityClassBlacklistKeywords.Count() == 0)
			return false;
	
		if (m_aComponentClassWhitelist == null || m_aComponentClassWhitelist.Count() == 0)
			return false;
	
		if (m_aResourceExcludeGUIDs == null || m_aResourceExcludeGUIDs.Count() == 0)
			return false;
		
		return true;
	}

}


#ifdef WORKBENCH
enum ExporterAssetType
{
	PREFAB,
	XOB,
	MAT,
	GAMEMAT,
	TXT,
	MISC,
}

#ifdef WE_LOGGING_ENABLED
class WorldExporterLogItem
{
	string messageA, messageB;
	int StartTime, EndTime;
	
	int GetTime()
	{
		return EndTime - StartTime;
	}
}
#endif

class ResourceExportCrawler
{
	ResourceManager m_ResourceManager = Workbench.GetModule(ResourceManager);
	ref set<ResourceName> m_Visited = new set<ResourceName>();

	bool InsertFirstTime(ResourceName resourceName)
	{
		if (resourceName.IsEmpty() || resourceName.GetPath().IsEmpty())
			return false;

		return m_Visited.Insert(resourceName);
	}

	void Crawl(ResourceName resourceName)
	{
		if (resourceName.IsEmpty() || resourceName.GetPath().IsEmpty())
			return;

		if (resourceName.EndsWith(".gamemat"))
			CrawlGamemat(resourceName);
		else if (resourceName.EndsWith(".physmat"))
			CrawlPhysmat(resourceName);
		// else if (resourceName.EndsWith(".et"))
		// 	m_sPrefabsToMigrate.Insert(resourceName);
		else if (resourceName.EndsWith(".xob"))
			CrawlXob(resourceName);
		else if (resourceName.EndsWith(".emat"))
			CrawlEmat(resourceName);
		else if (resourceName.EndsWith(".edds"))
			CrawlEdds(resourceName);
		else if (resourceName.EndsWith(".conf"))
			CrawlConf(resourceName);
		else if (resourceName.EndsWith(".vhcsurf"))
			CrawlVhcsurf(resourceName);
		else
			PrintFormat("Unrecognized resource suffix: %1", resourceName);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlGamemat(ResourceName resourceName, BaseContainer container = null)
	{
		bool isFirstTime = InsertFirstTime(resourceName);
		if (!isFirstTime)
			return;

		Resource resource;
		if (!container)
		{
			resource = BaseContainerTools.LoadContainer(resourceName);
			container = resource.GetResource().ToBaseContainer();
		}

		ResourceName physicsMaterial;
		container.Get("Physics material", physicsMaterial);
		if (physicsMaterial && physicsMaterial.GetPath())
			CrawlPhysmat(physicsMaterial);

		ResourceName vhcsurf;
		container.Get("Vehicle contact surface", vhcsurf);
		if (vhcsurf && vhcsurf.GetPath())
			CrawlVhcsurf(vhcsurf);

		BaseContainer baseGamemat = container.GetAncestor();
		if (baseGamemat)
			CrawlGamemat(baseGamemat.GetResourceName(), baseGamemat);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlPhysmat(ResourceName resourceName)
	{
		InsertFirstTime(resourceName);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlVhcsurf(ResourceName resourceName)
	{
		InsertFirstTime(resourceName);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlXob(ResourceName resourceName)
	{
		bool isFirstTime = InsertFirstTime(resourceName);
		if (!isFirstTime)
			return;

		MetaFile meta = m_ResourceManager.GetMetaFile(resourceName.GetPath());
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		for (int cfgIdx = 0, cfgCount = configurations.Count(); cfgIdx < cfgCount; cfgIdx++)
		{
			BaseContainer cfg = configurations.Get(cfgIdx);

			string materialAssigns;
			array<string> pairs = new array<string>;
			cfg.Get("MaterialAssigns", materialAssigns);
			materialAssigns.Split(";", pairs, true);

			foreach (string pair : pairs)
			{
				array<string> keyValue = new array<string>;
				pair.Split(",", keyValue, true);
				CrawlEmat(keyValue[1]);
			}

			BaseContainerList geometryParams = cfg.GetObjectArray("GeometryParams");
			for (int i = 0, iCount = geometryParams.Count(); i < iCount; i++)
			{
				BaseContainer geometryParam = geometryParams.Get(i);
				array<ResourceName> surfaceProperties;
				geometryParam.Get("SurfaceProperties", surfaceProperties);
				foreach(ResourceName gamematOrPhysmat : surfaceProperties)
				{
					Crawl(gamematOrPhysmat);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlEmat(ResourceName resourceName)
	{
		bool isFirstTime = InsertFirstTime(resourceName);
		if (!isFirstTime)
			return;

		Resource resource = BaseContainerTools.LoadContainer(resourceName);
		BaseContainer container = resource.GetResource().ToBaseContainer();
		for (int i = 0, count = container.GetNumVars(); i < count; i++)
		{
			if (container.GetDataVarType(i) != DataVarType.TEXTURE)
				continue;

			string varName = container.GetVarName(i);
			ResourceName texture;
			container.Get(varName, texture);

			array<string> textures = new array<string>;
			texture.Split(" ", textures, true);
			if (textures.Count() > 1)
			{
				foreach (string txt : textures)
				{
					if (txt.Get(0) == "{")
					{
						CrawlEdds(txt);
					}
				}
			}
			else
			{
				CrawlEdds(texture);
			}
		}

		ResourceName surfaceProperties;
		container.Get("SurfaceProperties", surfaceProperties);
		if (surfaceProperties && surfaceProperties.GetPath())
			CrawlGamemat(surfaceProperties);

		ResourceName plantMat;
		container.Get("PlantMat", plantMat);
		if (plantMat && plantMat.GetPath())
			CrawlEmat(plantMat);

		ResourceName clutterConfig;
		container.Get("ClutterConfig", clutterConfig);
		if (clutterConfig && clutterConfig.GetPath())
			CrawlConf(clutterConfig);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlEdds(ResourceName resourceName)
	{
		InsertFirstTime(resourceName);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlConf(ResourceName resourceName, BaseContainer container = null)
	{
		bool isFirstTime = InsertFirstTime(resourceName);
		if (!isFirstTime)
			return;

		Resource resource;
		if (!container)
		{
			resource = BaseContainerTools.LoadContainer(resourceName);
			container = resource.GetResource().ToBaseContainer();
		}

		CrawlConfObject(container);
	}

	//-----------------------------------------------------------------------------------------------
	void CrawlConfObject(BaseContainer container)
	{
		BaseContainer ancestor = container.GetAncestor();
		if (ancestor)
		{
			ResourceName myName = container.GetResourceName();
			ResourceName ancestorName = ancestor.GetResourceName();
			// If names differ, this container is object instantiated based on another
			// container which is independent resource.
			// TODO(kroslakmar): Fakes native function "IsExternal()" on resource name.
			if (myName != ancestorName)
				CrawlConf(ancestorName, ancestor);
		}

		for (int i = 0, iCount = container.GetNumVars(); i < iCount; i++)
		{
			string varName = container.GetVarName(i);
			switch (container.GetDataVarType(i))
			{
			case DataVarType.OBJECT:
				BaseContainer obj = container.GetObject(varName);
				if (obj)
					CrawlConfObject(obj);
				break;
			case DataVarType.RESOURCE_NAME:
				ResourceName varVal;
				container.Get(varName, varVal);
				Crawl(varVal);
				break;
			case DataVarType.OBJECT_ARRAY:
				BaseContainerList objects = container.GetObjectArray(varName);
				for (int j = 0, jCount = objects.Count(); j < jCount; j++)
				{
					BaseContainer obj = objects.Get(j);
					if (obj)
						CrawlConfObject(obj);
				}
				break;
			case DataVarType.RESOURCE_NAME_ARRAY:
				auto varVal = new array<ResourceName>();
				container.Get(varName, varVal);
				foreach(ResourceName entry : varVal)
					Crawl(entry);
				break;
			}
		}
	}
};

// Separate class from WorldExporterPlugin, since modding plugin class directly doesn't work well.
class WorldExportScript
{
	void OnBeforeEntityProcessed(WorldExporterPlugin plugin, IEntitySource source)
	{
	}

	void OnAfterEntityProcessed(WorldExporterPlugin plugin, IEntitySource source)
	{
	}

	void OnAfterEntityCloned(WorldExporterPlugin plugin, IEntitySource oldEntSrc, IEntitySource newEntSrc)
	{
	}

	void OnAfterEntityReparented(WorldExporterPlugin plugin, IEntitySource source, IEntitySource newParent)
	{
	}
}

[WorkbenchPluginAttribute(name: "Map Exporter", wbModules: { "WorldEditor" }, shortcut: "Ctrl+`", awesomeFontCode: 0xF338)] // 0xF338 = ↨
class WorldExporterPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "", desc: "'WorldExporterConfig' type config", params: "conf")]
	ResourceName m_ConfigResource;
	
	[Attribute(defvalue: "c:/Work/MapExport/", desc: "Destination folder for the export")]
	string m_ExportDestination;
	
	[Attribute(defvalue: "true", desc: "Mostly for debug purposes, enables/disables copying of files identified during export, for normal export, leave enabled")]
	bool m_AllowResourceCopy;

	[Attribute(desc: "Export using root resources.")]
	bool m_CrawlFromRootResources;

	ref WorldExporterConfig 						m_Config;
	WorldEditor 									m_WorldEditor;
	ref map<string, string>						m_mEntityClassBlacklist;
	ref map<ResourceName,ResourceName> 			m_mCreatedPrefabs;
	ref TStringArray 							m_aCreatedFiles;
	ref set<BaseContainer>						m_sDiscoveredPrefabs;
	ref map<ResourceName,bool> 					m_mPrefabsNeedingReplace;//when prefab is inside this container, that means it's been processed, 'true' value means it needs to be replaced, 'false' it does not
	
	ref set<ResourceName>							m_sPrefabsToMigrate;
	ref set<ResourceName>							m_sModelsToMigrate;
	ref set<ResourceName>							m_sMaterialsToMigrate;
	ref set<ResourceName>							m_sTexturesToMigrate;
	ref set<ResourceName>							m_sMiscToMigrate;
	ref set<ResourceName>							m_sResourceExcludeGUIDs;//for faster ref
	ref set<ResourceName>							m_sGamematsToMigrate;
	#ifdef WE_LOGGING_ENABLED
	ref array<ref WorldExporterLogItem>			m_aLogItems;
	#endif
	
	
	ref WorldExportScript MapSpecificScript;
	WorldEditorAPI m_WEapi;
	ResourceManager resourceManager;
	bool m_PerformExport;
	string m_sFilesystem;
	
	
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run Export", true)]
	protected bool ButtonExport()
	{
		// if(!IsExportTargetFolderEmpty())
		// {
		// 	Print(string.Format("Export target folder'%1' not empty, clear before export", m_ExportDestination), LogLevel.ERROR);
		// 	return true;
		// }
		m_PerformExport = true;
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	protected bool Init()
	{
		m_Config = GetConfig(m_ConfigResource);
		if (!m_Config || !m_Config.IsValid())
		{
			Print("Config load failed", LogLevel.ERROR);
			return false;
		}
		
		m_sFilesystem = m_Config.m_sFilesystem + ":";
		
		m_WorldEditor = Workbench.GetModule(WorldEditor);
		m_WEapi = m_WorldEditor.GetApi();
		resourceManager = Workbench.GetModule(ResourceManager);
		
		m_mEntityClassBlacklist 	= new map<string, string>();
		m_sResourceExcludeGUIDs	= new set<ResourceName>	();
		
		foreach(string turnInto:m_Config.m_aClassMappings)
		{
			TStringArray output = {};
			turnInto.Split("::",output, true);
			m_mEntityClassBlacklist.Insert(output[0], output[1]);
		}
		
		foreach(ResourceName resName: m_Config.m_aResourceExcludeGUIDs)
		{
			m_sResourceExcludeGUIDs.Insert(resName);
		}

		MapSpecificScript = new WorldExportScript();
		return true;
	}
	
	protected void InitVars()
	{
		m_PerformExport					= false;
		m_sDiscoveredPrefabs				= new set<BaseContainer>;
		m_mCreatedPrefabs 				= new map<ResourceName, ResourceName>;
		m_aCreatedFiles					= {};
		m_mPrefabsNeedingReplace 			= new map<ResourceName,bool>;
		m_sPrefabsToMigrate				= new set<ResourceName>;
		m_sModelsToMigrate				= new set<ResourceName>;
		m_sMaterialsToMigrate			= new set<ResourceName>;
		m_sTexturesToMigrate			= new set<ResourceName>;
		m_sMiscToMigrate				= new set<ResourceName>;
		m_sGamematsToMigrate = new set<ResourceName>;
		#ifdef WE_LOGGING_ENABLED
		m_aLogItems						= new array<ref WorldExporterLogItem>;
		#endif
	

		
	}
	//-----------------------------------------------------------------------------------------------
	override void Run()
	{
		InitVars();

		bool reload = true;
		while(reload)
		{
			reload = Workbench.ScriptDialog("Map Exporter", "This tool coverts the assets used on a map as well as the map \nentities based on the predefined rules, many of which can be adjusted in the config file.\nThe result is a folder containing the converted assets, as well as map data.\nIt's meant to strip the map of any gameplay specific features,\nleaving only the bare minimum to aproach a visual parity with the original map.\n\nAfter loading the map in the project the export is meant for, it's recommended to run the Re-save plugin which cleans the data by\ngetting rid of some properties of uknown types,\nreducing the number of errors displayed when loading the map.\n\n!!! This Tool expects to be provided a configuration file, as well as external script. \nThe script needs to be placed in the same folder as the config file !!!", this);
		}

		if(m_PerformExport)
		{
			if (!Init())//we need access to the config, which gets set in the ScriptDialog
			{
				Print("Init failed", LogLevel.ERROR);
				return;
			}
			if (m_CrawlFromRootResources)
			{
				auto crawler = new ResourceExportCrawler();
				foreach(ResourceName resourceName : m_Config.m_RootResources)
				{
					crawler.Crawl(resourceName);
				}

				ExportFiles(crawler.m_Visited);
			}
			else
			{
				PerformExport();
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void PerformExport()
	{
		if (m_WEapi)
		{
			array<IEntitySource> entitiesToProcess = {};

			int selectedEntCount = m_WEapi.GetSelectedEntitiesCount();
			if(selectedEntCount > 0)
			{
				for (int i = 0; i < selectedEntCount; i++)
				{
					IEntitySource entitySource = m_WEapi.GetSelectedEntity(i);
					if(!entitySource.GetParent())
						entitiesToProcess.Insert(entitySource);	
				}
			}
			else
			{
				int countAll = m_WEapi.GetEditorEntityCount();
			
				Print("Entities overall:"+countAll);
				for (int i = 0; i < countAll; i++)
				{
					IEntitySource entitySource = m_WEapi.GetEditorEntity(i);
					if(!entitySource.GetParent())
						entitiesToProcess.Insert(entitySource);	
				}
			}

			Print("walk entities array count: "+entitiesToProcess.Count());
			m_WEapi.BeginEntityAction("MAP_EXPORT");
			WBProgressDialog progress = new WBProgressDialog("Exporting map...", m_WorldEditor);
			foreach(int x, IEntitySource entSource:entitiesToProcess)
			{
				if(x % 100 == 0)
				{
					Print("Entities processed: " + x + "/"+entitiesToProcess.Count());
					progress.SetProgress((x / entitiesToProcess.Count()) * 0.9);
				}
				
				IEntitySource parent = entSource.GetParent();
				if(!parent)
				{
					ProcessEntityRecur(entSource,null,false);
				}
			}
			m_WEapi.EndEntityAction("MAP_EXPORT");
			
			if(m_AllowResourceCopy)
				PerformResourceExport(progress);
			if(m_Config.m_bDeleteTempFiles)
			ClearExportSourceData();
			#ifdef WE_LOGGING_ENABLED
				PrintLogs();
			#endif
		}
	}
	//-----------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Map Exporter plugin", "", this);
	}
	
	//-----------------------------------------------------------------------------------------------
	WorldExporterConfig GetConfig(ResourceName configPath)
	{
		Resource configResource = Resource.Load(configPath);
		if (!configResource.IsValid())
		{
			Print(string.Format("Cannot load config '%1'!", configPath), LogLevel.ERROR);
			return null;
		}

		BaseResourceObject configContainer = configResource.GetResource();
		if (!configContainer)
			return null;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return null;

		if (configBase.GetClassName() != "WorldExporterConfig")
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'WorldExporterConfig'!", configPath, configBase.GetClassName()), LogLevel.ERROR);
			return null;
		}

		return WorldExporterConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool AddAssetToMigrate(ExporterAssetType type, ResourceName resource, ResourceName resSource = string.Empty)
	{
		if (!resource)
			return false;

		if (!resource.GetPath())
		{
			return false;
		}
	
		bool registered = false;
	
		switch (type)
		{
			case ExporterAssetType.PREFAB:
				registered = m_sPrefabsToMigrate.Insert(resource);
				break;
			case ExporterAssetType.XOB:
				registered = m_sModelsToMigrate.Insert(resource);
				break;
			case ExporterAssetType.MAT:
				registered = m_sMaterialsToMigrate.Insert(resource);
				break;
			case ExporterAssetType.GAMEMAT:
				registered = m_sGamematsToMigrate.Insert(resource);
				break;
			case ExporterAssetType.TXT:
				registered = m_sTexturesToMigrate.Insert(resource);
				break;
			case ExporterAssetType.MISC:
				registered = m_sMiscToMigrate.Insert(resource);
				break;
			default:
				return false;
		}
	
		return registered;
	}
	//-----------------------------------------------------------------------------------------------
	bool IsEntityClassBlacklisted(IEntitySource entSource)
	{
		string entClassName = entSource.GetClassName();
		
		if(m_Config.m_aEntityClassWhitelist.Contains(entClassName))//first check whitelist, as some checks later on can be quite broad
			return false;
		if(m_mEntityClassBlacklist.Contains(entClassName))
			return true;
		
		foreach(string keyword:m_Config.m_aEntityClassBlacklistKeywords)
			if(entClassName.Contains(keyword))
				return true;
	
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	string GetAlternateClassName(string currentName)
	{
		if(m_mEntityClassBlacklist.Contains(currentName))
			return m_mEntityClassBlacklist.Get(currentName);
		return "GenericEntity";
	}
	//-----------------------------------------------------------------------------------------------
	bool IsComponentClassBlacklisted(string className)
	{
		return !m_Config.m_aComponentClassWhitelist.Contains(className);
	}

	//-----------------------------------------------------------------------------------------------
	bool IsAnyComponentIncompatible(notnull IEntitySource entSource)
	{
		int componentsCount = entSource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entSource.GetComponent(i);
			if(IsComponentClassBlacklisted(componentSource.GetClassName()))
				return true;

		}
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	bool DoesEntityNeedReplacing(IEntitySource entSource)
	{
		if(IsEntityClassBlacklisted(entSource))
			return true;
		if(IsAnyComponentIncompatible(entSource))
			return true;
		IEntitySource prefab = entSource.GetAncestor();
		if(prefab)
			return DoesPrefabNeedReplacingFast(prefab);
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	
	//cached version, use this as it's faster
	bool DoesPrefabNeedReplacingFast(BaseContainer prefab)
	{
		ResourceName resource = prefab.GetResourceName();
		if(m_mPrefabsNeedingReplace.Contains(resource))
		{
			return m_mPrefabsNeedingReplace.Get(resource);
		}
		else
		{
			bool result = DoesPrefabNeedReplacing(prefab);
			m_mPrefabsNeedingReplace.Insert(resource, result);
			return result;
		}
	}
	//-----------------------------------------------------------------------------------------------
	//uncached version, only use from inside the cached version
	bool DoesPrefabNeedReplacing(BaseContainer prefab)
	{
		//first check the class, that's the cheapest disqualifier
		if(IsEntityClassBlacklisted(prefab))
		{
			return true;
		}
		
		// check components
		if(IsAnyComponentIncompatible(prefab))
			return true;
		
		//now check immediate children
		int childCount = prefab.GetNumChildren();
		
		for(int i = 0; i < childCount; i++)
		{
			BaseContainer child = prefab.GetChild(i);
			if(DoesEntityNeedReplacing(child))
			{
				return true;
			}
		}
		
		RunPrefabDiscovery(prefab);//there may be a more optimal position for this call
		return false;
	}
	
	void RegisterXob(BaseContainer meshObjectComp)
	{
		ResourceName resourceName;
		meshObjectComp.Get("Object", resourceName);
		
		if(resourceName)
		{
			AddAssetToMigrate(ExporterAssetType.XOB, resourceName);
		}
		
		DiscoverMeshObjectMaterialOverrides(meshObjectComp, resourceName);
	}
	//-----------------------------------------------------------------------------------------------
	//takes in an entity or prefab source, saves the xob and checks for any material overrides
	//TODO:ideally should only do anything if the xob or the materials are overriden on the specific container, if it's at all possible check that
	void RegisterPrefabXob(IEntitySource entSource)
	{
		BaseContainer container = FindComponentSource(entSource, "MeshObject");
		//-------------------------- MeshObject ---------------------------
		if (container)
		{
			BaseContainer ancestor = entSource.GetAncestor();
		
			if(ancestor)
			{
				BaseContainer componentSourceAnc = FindComponentSource(ancestor, "MeshObject");
				if(container == componentSourceAnc)
					return;//no override
			}

			RegisterXob(container);
		}
	}
	//-----------------------------------------------------------------------------------------------
	void DiscoverMeshObjectMaterialOverrides(BaseContainer meshContainer, ResourceName resourceName)
	{
		BaseContainerList materials = meshContainer.GetObjectArray("Materials");
		if (materials)
		{
			for (int i = 0; i < materials.Count(); i++)
			{
				BaseContainer materialData = materials.Get(i);
				ResourceName matTgt;
				materialData.Get("AssignedMaterial", matTgt);
				if(matTgt)
				{
					RegisterMaterial(matTgt);
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void GetMaterialsFromXobs()
	{
		foreach(ResourceName res:m_sModelsToMigrate)
		{
			MetaFile meta = resourceManager.GetMetaFile(res.GetPath());
			if(!meta)
			{
				return;
			}
			BaseContainerList configurations = meta.GetObjectArray("Configurations");
			for (int cfgIdx = 0, cfgCount = configurations.Count(); cfgIdx < cfgCount; cfgIdx++)
			{
				BaseContainer cfg = configurations.Get(cfgIdx);

				string materialAssigns;
				array<string> pairs = new array<string>;
				cfg.Get("MaterialAssigns", materialAssigns);
				materialAssigns.Split(";", pairs, true);

				foreach (string pair : pairs)
				{
					array<string> keyValue = new array<string>;
					pair.Split(",", keyValue, true);
					ResourceName resName = keyValue[1];
					RegisterMaterial(resName);
				}

				BaseContainerList geometryParams = cfg.GetObjectArray("GeometryParams");
				for (int i = 0, count = geometryParams.Count(); i < count; i++)
				{
					BaseContainer geometryParam = geometryParams.Get(i);
					array<ResourceName> surfaceProperties;
					geometryParam.Get("SurfaceProperties", surfaceProperties);
					foreach(ResourceName gamemat : surfaceProperties)
					{
						RegisterGamemat(gamemat);
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	void GetTexturesFromMaterials()
	{
		for(int j = 0; j < m_sMaterialsToMigrate.Count();j++)
		{
			ResourceName resName = m_sMaterialsToMigrate.Get(j);
			Resource resource = BaseContainerTools.LoadContainer(resName);
			
			if(!resource || !resource.IsValid())
				return;
			
			BaseResourceObject bro = resource.GetResource();

			BaseContainer container = bro.ToBaseContainer();
			if(!container)
				return;
			int varCount = container.GetNumVars();
			
			for(int i = 0; i < varCount; i++)
			{
				if(container.GetDataVarType(i) == DataVarType.TEXTURE)
				{
					string varName = container.GetVarName(i);
					ResourceName texture;
					container.Get(varName,texture);
					
					array<string> textures = new array<string>;
					texture.Split(" ", textures, true);
					if(textures.Count() > 1)
					{
						foreach(string txt:textures)
						{
							if(txt.Get(0) == "{")
							{
								AddAssetToMigrate(ExporterAssetType.TXT,txt, resName);
							}
						}
					}
					else
					{
						AddAssetToMigrate(ExporterAssetType.TXT,texture, resName);
					}
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void RegisterMaterial(ResourceName resName)
	{
		if(m_sMaterialsToMigrate.Contains(resName))
			return;
		AddAssetToMigrate(ExporterAssetType.MAT,resName);
				
		Resource res = BaseContainerTools.LoadContainer(resName);
		if(!res)
			return;
		BaseResourceObject bro = res.GetResource();
		if(!bro)
			return;
		BaseContainer containerMaterial = bro.ToBaseContainer();
		if(!containerMaterial)
			return;
		BaseContainer anc = containerMaterial.GetAncestor();
		if(anc)
			RegisterMaterial(containerMaterial.GetAncestor().GetResourceName());
		
		int varCount = containerMaterial.GetNumVars();
		
		for(int i = 0; i < varCount; i++)
		{
			if(containerMaterial.GetDataVarType(i) == DataVarType.RESOURCE_NAME)
			{
				string varName = containerMaterial.GetVarName(i);
				ResourceName material;
				containerMaterial.Get(varName,material);
				if(!material.GetPath() || !material.GetPath().Contains(".emat"))
					continue;
				RegisterMaterial(material);
				
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	void RegisterGamemat(ResourceName resName)
	{
		bool registered = AddAssetToMigrate(ExporterAssetType.GAMEMAT, resName);
		if (!registered)
			return;

		Resource res = BaseContainerTools.LoadContainer(resName);
		if (!res)
			return;

		BaseResourceObject bro = res.GetResource();
		if (!bro)
			return;

		BaseContainer container = bro.ToBaseContainer();
		if (!container)
			return;

		BaseContainer anc = container.GetAncestor();
		if (anc)
			RegisterGamemat(anc.GetResourceName());
	}

	//-----------------------------------------------------------------------------------------------
	//this method tries to identify(discover) prefabs that have not been discovered yet
	void RunPrefabDiscovery(IEntitySource prefab)
	{
		#ifdef WE_LOGGING_ENABLED
		int indx = LogTimedStart("RunPrefabDiscovery:"+prefab);
		#endif
		
		RegisterPrefabXob(prefab);//this is for prefab instance, inside another prefab, which can have model override
		
		if(m_sDiscoveredPrefabs.Contains(prefab))
		{
			return;
		}
		ResourceName resName = prefab.GetResourceName();
		if(resName && resName.GetPath())
		{
			m_sDiscoveredPrefabs.Insert(prefab);
		}
		
		int childCount = prefab.GetNumChildren();
		for(int i = 0; i < childCount; i++)
		{
			IEntitySource child = prefab.GetChild(i);
			RunPrefabDiscovery(child);
		}
		
		IEntitySource ancestor = prefab.GetAncestor();
		if(ancestor)
			RunPrefabDiscovery(ancestor);
		#ifdef WE_LOGGING_ENABLED
		LogTimedEnd(indx);
		#endif
		
	}
	//-----------------------------------------------------------------------------------------------		
	
	void ProcessEntityRecur(IEntitySource entSource, IEntitySource parent, bool parentToBeDeleted, array<IEntitySource> toReparent = null)
	{
		int childCount = entSource.GetNumChildren();
		
		IEntitySource newEntitySrc = entSource;
		OnBeforeEntityProcessed(entSource);
		bool isPrefabChild = IsPartOfPrefab(entSource);
		bool toBeDeleted;
		RegisterPrefabXob(entSource);//this is here because an entity placed in the world can have either xob override and/or material override
		if(!isPrefabChild)
		{
			if(DoesEntityNeedReplacing(entSource))
			{
				newEntitySrc = CloneEntityFull(entSource, parent);
				if(!parentToBeDeleted)
					toBeDeleted = true;
			}
			else 
			{
				if (entSource.GetParent() != parent)
				{
					IEntity entParent = m_WEapi.SourceToEntity(parent);
					IEntity entChild = m_WEapi.SourceToEntity(entSource);
					if(entParent && entChild)
					{
						toReparent.Insert(entSource);
					}
				}
			}
		}
		
		array<IEntitySource> children = new array<IEntitySource>();
		for(int x = 0; x < childCount; x++)
		{
			IEntitySource childSource = entSource.GetChild(x);
			children.Insert(childSource);
		}
		
		if(!children.IsEmpty())
		{
			array<IEntitySource> entitiesToReparent = {};
			foreach(IEntitySource child:children)
			{
				ProcessEntityRecur(child, newEntitySrc, toBeDeleted, entitiesToReparent);
			}
			#ifdef WE_LOGGING_ENABLED
			int indx = LogTimedStart("Reparenting:"+children.Count());
			#endif
			if(!entitiesToReparent.IsEmpty())
			{
				m_WEapi.ParentEntities(newEntitySrc, entitiesToReparent, false);
				foreach(IEntitySource ent: entitiesToReparent)
					OnAfterEntityReparented(ent,newEntitySrc);
			}
			
			#ifdef WE_LOGGING_ENABLED
			LogTimedEnd(indx);
			#endif
		}
		if(toBeDeleted)
		{
			string name = entSource.GetName();
			m_WEapi.DeleteEntity(entSource);
			if(name)
			{
				newEntitySrc.SetName(name);
			}
			OnAfterEntityProcessed(newEntitySrc);
		}
		else if(!parentToBeDeleted)
		{
			OnAfterEntityProcessed(entSource);
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	IEntitySource CloneEntityFull(IEntitySource entSource, IEntitySource parent)
	{
		string className = entSource.GetClassName();
		
		if(IsEntityClassBlacklisted(entSource))
		{
			className = GetAlternateClassName(className);
		}
		
		BaseContainer prefab = entSource.GetAncestor();
		if(prefab)
		{
			if(DoesPrefabNeedReplacingFast(prefab))
			{
				ResourceName newPrefab = ClonePrefabFast(prefab);
			
				if(newPrefab)
				{
					//string fileName = FilePath.StripPath(newPrefab.GetPath());
					className = newPrefab.GetPath();
				}
				else
					Print("Something went wrong with prefab replace:" + prefab.GetResourceName(), LogLevel.ERROR);
			}
		}

		IEntitySource newEntSrc = CloneSingleEntity(className, entSource, parent, entSource.GetLayerID());
		
		CopyComponents(entSource, newEntSrc);
		return newEntSrc;
	}
	//-----------------------------------------------------------------------------------------------
	protected ResourceName ClonePrefabFast(BaseContainer prefab)
	{
		ResourceName newPrefab;
		ResourceName prefabResName = prefab.GetResourceName();
		
		if(!m_mCreatedPrefabs.Contains(prefabResName))
		{
			newPrefab = ClonePrefabSlow(prefab);
		}
		else
		{
			newPrefab = m_mCreatedPrefabs.Get(prefabResName);
		}
		return newPrefab;
	}
	//-----------------------------------------------------------------------------------------------
	bool IsFromPrefabLibrary(ResourceName prefab)
	{
		return prefab.GetPath().Contains("PrefabLibrary");
	}
	
	//-----------------------------------------------------------------------------------------------
	protected ResourceName ClonePrefabSlow(notnull BaseContainer originalPrefab)
	{
		ResourceName originalPrefabName = originalPrefab.GetResourceName();
		#ifdef WE_LOGGING_ENABLED
		int indx = LogTimedStart("ClonePrefabSlow:"+originalPrefabName);
		#endif
		BaseContainer ancestorPrefab = originalPrefab.GetAncestor();//prefab container this prefab container is inheriting from
		
		ResourceName ancestorPrefabName;
		if(ancestorPrefab)
		{
			if(DoesPrefabNeedReplacingFast(ancestorPrefab))
			{
				ancestorPrefabName = ClonePrefabFast(ancestorPrefab);
			}
			else
			{
				ancestorPrefabName = ancestorPrefab.GetResourceName();
			}
		}
		
		//this skips over the prefabs from PrefabLibrary in the inheritence of the newly created prefabs
		//...by returning the ancestor prefab as the cloned prefab, instead of the prefab from the PrefabLibrary
		if(m_Config.m_bSkipPrefabLibrary && IsFromPrefabLibrary(originalPrefabName) && ancestorPrefabName)
		{
			#ifdef WE_LOGGING_ENABLED
			LogTimedEnd(indx);
			#endif
			RegisterNewPrefab(originalPrefabName,ancestorPrefabName);
			return ancestorPrefabName;
		}

		string originalPrefabPath = originalPrefabName.GetPath();
		string newPrefabPath = FilePath.Concat(m_Config.m_sExportCachePrefix, originalPrefabPath);
		string newPrefabPathAbs;
		Workbench.GetAbsolutePath(newPrefabPath, newPrefabPathAbs, false);
		IEntitySource entSrc;
		if(newPrefabPathAbs)
		{
			PrintFormat("New prefab alternative not found for %1, lets create it", originalPrefabName);

			entSrc = DeepPrefabEntityCloneRecur(originalPrefab, null, ancestorPrefabName);

			FileIO.MakeDirectory(FilePath.StripFileName(newPrefabPathAbs));
			m_WEapi.CreateEntityTemplate(entSrc, newPrefabPathAbs);//<------------ CREATE PREFAB
			resourceManager.WaitForFile(m_sFilesystem + newPrefabPath, 10000);
			ResourceName newPrefabName = Workbench.GetResourceName(newPrefabPath);//variable re-use

			AddAssetToMigrate(ExporterAssetType.PREFAB, newPrefabName);
			RegisterNewPrefab(originalPrefabName, newPrefabName);

			PrintFormat("new prefab ready for:%1, new resource name:%2", originalPrefabName, newPrefabName);

			// Delete ent.
			if(entSrc)
				m_WEapi.DeleteEntity(entSrc);
			#ifdef WE_LOGGING_ENABLED
			LogTimedEnd(indx);
			#endif
			return newPrefabName;
		}
		return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	IEntitySource DeepPrefabEntityCloneRecur(IEntitySource prefab, IEntitySource parent, ResourceName ancestor)
	{
		IEntitySource newEntSrc;
		if(!IsPartOfPrefab(prefab))
		{
			string className = prefab.GetClassName();
			
			if(IsEntityClassBlacklisted(prefab))
			{
				className = GetAlternateClassName(className);
			}
				
			BaseContainer anc = GetPrefab(prefab);//okno_old_prefab
			if(anc)
			{
				if(DoesPrefabNeedReplacingFast(anc))
				{
					ResourceName newPrefab = ClonePrefabFast(anc);//okno_new_prefab
			
					if(newPrefab)
						className = newPrefab;
					else
						Print("Something went wrong with prefab replace:" + anc.GetResourceName(), LogLevel.ERROR);
				}
				else
				{
					className = GetPrefabName(prefab);
				}
			}

			newEntSrc = CloneSingleEntity(className, prefab, parent);//okno_new_prefab_instance
			if(ancestor && ancestor.GetPath())
			{
				newEntSrc.SetAncestor(ancestor);
				
			}
			CopyComponents(prefab, newEntSrc);
		}

		int childCount = prefab.GetNumChildren();
	
		for(int i = 0; i < childCount;i++)
		{
			IEntitySource child = prefab.GetChild(i);
			if(!newEntSrc)
			{
				newEntSrc = parent.GetChild(i);
			}
			
			DeepPrefabEntityCloneRecur(child,newEntSrc,string.Empty);
		}
		return newEntSrc;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void RegisterNewPrefab(ResourceName resNameOriginal, ResourceName resNameNew)
	{
		m_mCreatedPrefabs.Insert(resNameOriginal, resNameNew);
		m_aCreatedFiles.Insert(resNameNew.GetPath());
	}
	
	//-----------------------------------------------------------------------------------------------
	bool IsPartOfPrefab(IEntitySource entSource)
	{
		BaseContainer thisEntPrefabRoot;
		if(entSource.GetAncestor() && entSource.GetAncestor().GetParent())
			thisEntPrefabRoot = entSource.GetAncestor().GetParent();
		
		IEntitySource parentSource = entSource.GetParent();
		
		BaseContainer parentPrefab;
		if(parentSource && parentSource.GetAncestor())
			parentPrefab = parentSource.GetAncestor();

		return (thisEntPrefabRoot && parentPrefab && parentPrefab == thisEntPrefabRoot);
	}
	//-----------------------------------------------------------------------------------------------
	static BaseContainer GetPrefab(BaseContainer source)
	{
		BaseContainer container = source.GetAncestor();
		BaseContainer prefab;
		while(container)
		{
			if(container.GetResourceName() != ResourceName.Empty)
			{
				prefab = container;
				break;
			}
			container = container.GetAncestor();
		}
		return prefab;
	}
	//-----------------------------------------------------------------------------------------------
	static ResourceName GetPrefabName(BaseContainer source)
	{
		BaseContainer container = source.GetAncestor();
		ResourceName prefabRes;
		while(container)
		{
			if(container.GetResourceName() != ResourceName.Empty)
			{
				prefabRes = container.GetResourceName();
				break;
			}
			container = container.GetAncestor();
		}
		return prefabRes;
	}

	//-----------------------------------------------------------------------------------------------
	void CopyContainerProperties(BaseContainer sourceContainer, BaseContainer targetContainer, BaseContainer targetEntity, array<ref ContainerIdPathEntry> path)
	{
		string varName;
		for (int v = 0, varCount = sourceContainer.GetNumVars(); v < varCount; v++)
		{
			varName = sourceContainer.GetVarName(v);
			if (!sourceContainer.IsVariableSetDirectly(varName))
				continue;
			switch (sourceContainer.GetDataVarType(v))
			{
				case DataVarType.SCALAR_ARRAY:
				{
					break;
				}
				
				case DataVarType.RESOURCE_NAME:
				{
					ResourceName valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc);
					
					break;
				}

				case DataVarType.STRING:
				{
					string valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc);
					
					break;
				}
				case DataVarType.INTEGER:
				{
					int valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);

					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					break;
				}
				case DataVarType.SCALAR:
				{
					float valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);

					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					break;
				}
				case DataVarType.VECTOR3:
				{
					vector valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);

					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString(false));
					break;
				}
				case DataVarType.BOOLEAN:
				{
					int valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);
					
					if(valueSrc != valueTrg)
					{
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					}
					break;
				}
				case DataVarType.FLAGS:
				{
					int valueSrc, valueTrg;
					sourceContainer.Get(varName, valueSrc);
					targetContainer.Get(varName, valueTrg);

					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					break;
				}
				default:
				{
					Print(string.Format("Cannot copy variable '%1' from template, it has unsupported type %2!", varName, typename.EnumToString(DataVarType, sourceContainer.GetDataVarType(v))), LogLevel.ERROR);
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void CopyComponents(IEntitySource source, IEntitySource target)
	{
		array<string> componentsToCopy = {"MeshObject", "Hierarchy", "RigidBody"};
		foreach (string componentName : componentsToCopy)
		{
			CopyComponent(componentName, source, target);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	void CopyComponent(string componentName, IEntitySource source, IEntitySource target)
	{
		BaseContainer componentSource = FindComponentSource(source, componentName);
		BaseContainer ancestor = source.GetAncestor();
		
		if(ancestor)
		{
			BaseContainer componentSourceAnc = FindComponentSource(ancestor, componentName);
			if(componentSource == componentSourceAnc)
				return;//no override
		}
		
		if (componentSource)
		{
			array<ref ContainerIdPathEntry> containerPath = {new ContainerIdPathEntry(componentName)};
			BaseContainer componentTarget = FindComponentSource(target, componentName);
	
			if (!componentTarget)
			{
				componentTarget = m_WEapi.CreateComponent(target, componentName);
			}
	
			CopyContainerProperties(componentSource, componentTarget, target, containerPath);
			CopyContainerPropertiesSpecial(componentName, componentSource, componentTarget, target, containerPath);
		}
	}
	//-----------------------------------------------------------------------------------------------
	void CopyContainerPropertiesSpecial(string componentName, BaseContainer sourceComponent, BaseContainer targetComponent, BaseContainer targetEntity, array<ref ContainerIdPathEntry> path)
	{
		if(componentName == "MeshObject")
		{
			RegisterXob(sourceComponent);
			if(!sourceComponent.IsVariableSetDirectly("Materials"))
				return;
			
			BaseContainerList materialsSource = sourceComponent.GetObjectArray("Materials");
			BaseContainerList materialsTarget = targetComponent.GetObjectArray("Materials");

			if (materialsSource)
			{
				for (int i = 0; i < materialsSource.Count(); i++)
				{
					BaseContainer materialDataSource = materialsSource.Get(i);
		
					ResourceName matSrc, nameSrc;
					materialDataSource.Get("AssignedMaterial", matSrc);
					materialDataSource.Get("SourceMaterial", nameSrc);
					
					m_WEapi.CreateObjectArrayVariableMember(targetEntity, path, "Materials", "MaterialAssignClass", i);
					
					array<ref ContainerIdPathEntry> containerPath = {};
					foreach(ContainerIdPathEntry entry: path)
						containerPath.Insert(entry);
					containerPath.Insert(new ContainerIdPathEntry("Materials", i)); // Take the first point
					
					m_WEapi.SetVariableValue(targetEntity, containerPath, "SourceMaterial", nameSrc);
					m_WEapi.SetVariableValue(targetEntity, containerPath, "AssignedMaterial", matSrc);
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void ExportFiles(set<ResourceName> resourceNames)
	{
		foreach(ResourceName name:resourceNames)
		{
			if(m_sResourceExcludeGUIDs.Contains(name))
				continue;//skip resource when excluded
			string filePath = name.GetPath();
			ExportFile(filePath);
		}
	}
	//-----------------------------------------------------------------------------------------------
	void ExportFile(string srcFilePath)
	{
		string dstFilePath = srcFilePath;
		if (dstFilePath.StartsWith(m_Config.m_sExportCachePrefix))
		{
			int prefixLen = m_Config.m_sExportCachePrefix.Length() + 1; // +1 for path separator
			dstFilePath = dstFilePath.Substring(prefixLen, dstFilePath.Length() - prefixLen);
		}
		dstFilePath = FilePath.Concat(m_ExportDestination, dstFilePath);

		FileIO.MakeDirectory(FilePath.StripFileName(dstFilePath));

		string srcMetaPath = srcFilePath + ".meta";
		string dstMetaPath = dstFilePath + ".meta";

		if (FileIO.FileExists(srcFilePath))
			FileIO.CopyFile(srcFilePath, dstFilePath);

		if (FileIO.FileExists(srcMetaPath))
			FileIO.CopyFile(srcMetaPath, dstMetaPath);

		if (m_Config.m_bExportSourceFiles)
		{
			MetaFile meta = resourceManager.GetMetaFile(srcFilePath);
			if (meta)
			{
				string sourceFilePath = meta.GetSourceFilePath();
				string fsName = FilePath.FileSystemNameFromFileName(sourceFilePath);
				string fsComplete = "$"+fsName+":";
				sourceFilePath.Replace(fsComplete,"");
				string sourceFilename = FilePath.StripPath(sourceFilePath);
				if(!sourceFilename.IsEmpty())
				{
					string destination3 = m_ExportDestination+sourceFilePath;
					if (FileIO.FileExists(sourceFilePath))
						FileIO.CopyFile(sourceFilePath, destination3);
				}
				meta.Release();
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void ExportMapData()
	{
		TStringArray files = {};
		FileIO.FindFiles(files.Insert,m_Config.m_sExportMapDir,string.Empty);
		foreach(string filePath:files)
			ExportFile(filePath);
	}
	//-----------------------------------------------------------------------------------------------
	void ExportSharedData()
	{
		set<ResourceName> foundResources = new set<ResourceName>();
	
		array<string> extensions = {};
		m_Config.m_sCopyPathsMixedExtensions.Split(";",extensions, true);
		
		foreach(string path: m_Config.m_aCopyPathsMixed)
		{
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.rootPath = m_sFilesystem + path;
			filter.fileExtensions = extensions;
			
			ResourceDatabase.SearchResources(filter, foundResources.Insert);
		}
		
		foreach(string path: m_Config.m_aModelPaths)
		{
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.rootPath = m_sFilesystem + path;
			filter.fileExtensions = {"xob"};
			
			ResourceDatabase.SearchResources(filter, foundResources.Insert);
		}
		
		ExportFiles(foundResources);
	}
	//-----------------------------------------------------------------------------------------------
	#ifdef WE_LOGGING_ENABLED
	int LogTimedStart(string message)
	{
		WorldExporterLogItem item = new WorldExporterLogItem();
		if(item)
		{
			if(message)
				item.messageA = message;
			item.StartTime = System.GetTickCount();
		}
		return m_aLogItems.Insert(item);
	}
	//-----------------------------------------------------------------------------------------------
	void LogTimedEnd(int index, string message = string.Empty)
	{
		if(m_aLogItems.IsIndexValid(index))
		{
			WorldExporterLogItem item = m_aLogItems.Get(index);
			
			if(item)
			{
				if(message)
					item.messageB = message;
				item.EndTime = System.GetTickCount();
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void PrintLogs()
	{
		foreach(WorldExporterLogItem item: m_aLogItems)
		{
			Print(string.Format("Timed:,%1,%2,took(ms):,%3,", item.messageA, item.messageB, item.GetTime().ToString()));
		}
	}
	
	#endif
	//-----------------------------------------------------------------------------------------------
	IEntitySource CloneSingleEntity(string className, IEntitySource oldEntSrc,  IEntitySource parent, int layerID = 0)
	{
		#ifdef WE_LOGGING_ENABLED
		int indx = LogTimedStart("CloneSingleEntity:"+className);
		#endif

		vector angles;
		
		oldEntSrc.Get("angles", angles);
		
		IEntitySource newEntSrc = m_WEapi.CreateEntityExt(className, string.Empty, layerID, parent, vector.Zero, angles, 0);
		if(newEntSrc)
			OnAfterEntityCloned(oldEntSrc, newEntSrc);
		else
		{
			//cloning failed TODO: add print
		}
		#ifdef WE_LOGGING_ENABLED
		LogTimedEnd(indx);
		#endif
		return newEntSrc;
	}
	//-----------------------------------------------------------------------------------------------
	void OnBeforeEntityProcessed(IEntitySource source)
	{
		MapSpecificScript.OnBeforeEntityProcessed(this, source);
	}
	//-----------------------------------------------------------------------------------------------
	void OnAfterEntityProcessed(IEntitySource source)
	{
		MapSpecificScript.OnAfterEntityProcessed(this, source);
	}
	//-----------------------------------------------------------------------------------------------
	void OnAfterEntityCloned(IEntitySource oldEntSrc, IEntitySource newEntSrc)
	{
		CopyEntityProperties(oldEntSrc, newEntSrc);
		MapSpecificScript.OnAfterEntityCloned(this, oldEntSrc, newEntSrc);
	}
	//-----------------------------------------------------------------------------------------------
	void OnAfterEntityReparented(IEntitySource source, IEntitySource newParent)
	{
		MapSpecificScript.OnAfterEntityReparented(this, source, newParent);
	}
	//-----------------------------------------------------------------------------------------------
	void CopyEntityProperties( IEntitySource oldEntSrc,  IEntitySource newEntSrc)
	{
		if(oldEntSrc.IsVariableSetDirectly("Flags"))
		{
			EntityFlags flags;
			oldEntSrc.Get("Flags", flags);
			m_WEapi.SetVariableValue(newEntSrc, {}, "Flags", flags.ToString()); // Set Name to given value
		}
		if(oldEntSrc.IsVariableSetDirectly("scale"))
		{
			float scale;
			oldEntSrc.Get("scale", scale);
			m_WEapi.SetVariableValue(newEntSrc, {}, "scale", scale.ToString()); // Set Name to given value
		}
		if(oldEntSrc.IsVariableSetDirectly("coords"))
		{
			vector coords;
			oldEntSrc.Get("coords", coords);
			m_WEapi.SetVariableValue(newEntSrc, {}, "coords", coords.ToString(false)); // Set Name to given value
		}
		if(oldEntSrc.IsVariableSetDirectly("placement"))
		{
			string placement;
			oldEntSrc.Get("placement", placement);
			m_WEapi.SetVariableValue(newEntSrc, {}, "placement", placement); // Set Name to given value
		}
	}

	//-----------------------------------------------------------------------------------------------
	// this iterates through all prefabs seen on the map
	void ProcessDiscoveredPrefabs()
	{
		foreach(BaseContainer prefab:m_sDiscoveredPrefabs)
		{
			RegisterPrefabXob(prefab);
			ResourceName resName = prefab.GetResourceName();
			if(!m_mCreatedPrefabs.Contains(resName))//exclude prefabs that are being replaced by a new prefab
			{
				AddAssetToMigrate(ExporterAssetType.PREFAB,resName);
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void PerformResourceExport(WBProgressDialog dialog)
	{
		ProcessDiscoveredPrefabs();
		GetMaterialsFromXobs();
		GetTexturesFromMaterials();

		auto crawler = new ResourceExportCrawler();

		ExportFiles(m_sPrefabsToMigrate);
		dialog.SetProgress(0.91);
		ExportFiles(m_sModelsToMigrate);
		dialog.SetProgress(0.92);
		// ExportFiles(m_sMaterialsToMigrate);
		// dialog.SetProgress(0.93);
		ExportFiles(m_sTexturesToMigrate);
		dialog.SetProgress(0.94);
		// ExportFiles(m_sMiscToMigrate);
		// dialog.SetProgress(0.95);

		foreach (ResourceName resourceName : m_sMiscToMigrate) crawler.Crawl(resourceName);
		foreach (ResourceName resourceName : m_sMaterialsToMigrate) crawler.Crawl(resourceName);
		foreach (ResourceName resourceName : m_sGamematsToMigrate) crawler.Crawl(resourceName);
		foreach (ResourceName resourceName : m_Config.m_RootResources) crawler.Crawl(resourceName);

		ExportFiles(crawler.m_Visited);
		dialog.SetProgress(0.96);

		#ifdef WE_LOGGING_ENABLED
		Print("---------------------------Models------------------------------");
		foreach(ResourceName name:m_sModelsToMigrate)
			Print(name);

		Print("---------------------------Materials------------------------------");
		foreach(ResourceName name:m_sMaterialsToMigrate)
			Print(name);
		
		Print("---------------------------Textures------------------------------");
		foreach(ResourceName name:m_sTexturesToMigrate)
			Print(name);
		
		Print("-------------------------Prefabs --------------------------------");
		foreach(ResourceName name:m_sPrefabsToMigrate)
			Print(name);
		
		Print("-------------------------Misc --------------------------------");
		foreach(ResourceName name:m_sMiscToMigrate)
			Print(name);
		#endif
		
		// ExportSharedData();
		dialog.SetProgress(0.98);
		m_WorldEditor.Save();//Save the world
		if (m_Config.m_ExportMapData)
			ExportMapData();
		dialog.SetProgress(0.99);
	}
	//-----------------------------------------------------------------------------------------------
	void ClearExportSourceData()
	{
		foreach(ResourceName resName:m_mCreatedPrefabs)
		{
			DeleteFilePlus(resName.GetPath());
		}
		foreach(string filePath:m_aCreatedFiles)
		{
			DeleteFilePlus(filePath);
		}
	} 
	//-----------------------------------------------------------------------------------------------
	void DeleteFilePlus(string path)
	{
		string pathMeta = path +".meta";
		
		FileIO.DeleteFile(path);
		FileIO.DeleteFile(pathMeta);
	}
	//-----------------------------------------------------------------------------------------------
	bool IsExportTargetFolderEmpty()
	{
		TStringArray filesAndDirs = {};
		FileIO.FindFiles(filesAndDirs.Insert,m_ExportDestination, string.Empty);
		return filesAndDirs.Count() == 0;
	}
	//--------------------------------------------------------
	static IEntityComponentSource FindComponentSource(IEntitySource prefabEntity, string componentClassName)
	{
		if (!prefabEntity)
			return null;

		IEntityComponentSource componentSource;
		for (int i, componentsCount = prefabEntity.GetComponentCount(); i < componentsCount; i++)
		{
			componentSource = prefabEntity.GetComponent(i);
			if (componentSource.GetClassName() == componentClassName)
				return componentSource;
		}

		return null;
	}
}
#endif
