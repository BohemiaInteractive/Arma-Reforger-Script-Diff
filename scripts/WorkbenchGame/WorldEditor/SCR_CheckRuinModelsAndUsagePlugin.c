#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Check Ruin Models and Usage",
//	category: "Building Destruction",
	description: "Destruction configuration plugin to:\n- Find .xob _Ruin files that do not have an associated Prefab\n- Find house_base Prefabs that do not have the _Ruin Prefab associated",
	wbModules: { "ResourceManager" },
	awesomeFontCode: 0xE4D4)]
class SCR_CheckRuinModelsAndUsagePlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "1", desc: "Print Resource Names")]
	protected bool m_bPrintResourceNames;

	[Attribute(defvalue: "1", desc: "Search in all addons - if unchecked, search only in " + BASE_PREFIX + " and " + ADDON_PREFIX)]
	protected bool m_bSearchInAllAddons;

	protected static const string BASE_PREFIX = "$ArmaReforger:";
	protected static const string ADDON_PREFIX = "$BuildingDestruction:";
	protected static const string XOBS_DIR = "Assets";
	protected static const string PREFABS_DIR = "Prefabs";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		// - Find all _Ruin Prefabs
		array<ResourceName> ruinPrefabs = GetRuinPrefabs();
		int ruinPrefabsCount = ruinPrefabs.Count();

		array<ResourceName> prefablessRuinModels = GetRuinModelsWithoutPrefabs(ruinPrefabs);
		array<ResourceName> unassignedRuinPrefabs = GetUnassignedRuinPrefabs(ruinPrefabs);

		if (m_bPrintResourceNames)
		{
			foreach (ResourceName resourceName : prefablessRuinModels)
			{
				Print("No Prefab found for Ruin " + resourceName, LogLevel.NORMAL);
			}

			foreach (ResourceName resourceName : unassignedRuinPrefabs)
			{
				Print("No Prefab using Ruin " + resourceName, LogLevel.NORMAL);
			}
		}

		// always print that
		Print("Total _Ruin Prefabs: " + ruinPrefabs.Count(), LogLevel.NORMAL);
		Print("Ruin XOBs without Prefab: " + prefablessRuinModels.Count(), LogLevel.NORMAL);
		Print("Unused Ruin Prefabs: " + unassignedRuinPrefabs.Count(), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Prints XOBs without Prefabs
	//! \param[in] parameter
	protected array<ResourceName> GetRuinModelsWithoutPrefabs(notnull array<ResourceName> ruinPrefabs)
	{
		// - Find .xob _Ruin files that do not have an associated _Ruin Prefab

		Resource resource;
		BaseResourceObject baseResourceObject;
		IEntitySource entitySource;
		set<ResourceName> ruinXobSet = new set<ResourceName>(); // found xob set
		foreach (ResourceName ruinPrefab : ruinPrefabs)
		{
			resource = Resource.Load(ruinPrefab);
			if (!resource.IsValid())
			{
				Print("Invalid Ruin Prefab (invalid resource) " + ruinPrefab, LogLevel.WARNING);
				continue;
			}

			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
			{
				Print("Invalid Ruin Prefab (no baseResourceObject) " + ruinPrefab, LogLevel.WARNING);
				continue;
			}

			entitySource = baseResourceObject.ToEntitySource();
			if (!entitySource)
			{
				Print("Invalid Ruin Prefab (no entitySource) " + ruinPrefab, LogLevel.WARNING);
				continue;
			}

			ResourceName ruinXOB = GetModelFromPrefab(entitySource);
			if (!ruinXOB)
			{
				Print("Invalid Ruin Prefab (no model) " + ruinPrefab, LogLevel.NORMAL);
				continue;
			}

			ruinXobSet.Insert(ruinXOB);
		}

		array<ResourceName> result = {};
		array<ResourceName> ruinXOBs = GetRuinXOBs();
		foreach (ResourceName ruinXOB : ruinXOBs)
		{
			if (!ruinXobSet.Contains(ruinXOB))
				result.Insert(ruinXOB);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName GetModelFromPrefab(notnull IEntitySource entitySource)
	{
		IEntityComponentSource meshComp = SCR_BaseContainerTools.FindComponentSource(entitySource, MeshObject);
		if (!meshComp)
			return ResourceName.Empty;

		ResourceName result;
		meshComp.Get("Object", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ruinPrefabs
	protected array<ResourceName> GetUnassignedRuinPrefabs(notnull array<ResourceName> ruinPrefabs)
	{
		// - Check that all _Ruin Prefabs have an associated building _base Prefab

		array<ResourceName> houseBases = GetHouseBasePrefabs();

		Resource resource;
		BaseResourceObject baseResourceObject;
		IEntitySource entitySource;
		typename type;
		set<ResourceName> usedRuinPrefabSet = new set<ResourceName>(); // found Ruin set
		foreach (ResourceName houseBase : houseBases)
		{
			resource = Resource.Load(houseBase);
			if (!resource.IsValid())
			{
				Print("Invalid building Prefab (invalid resource) " + houseBase, LogLevel.WARNING);
				continue;
			}

			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
			{
				Print("Invalid building Prefab (no baseResourceObject) " + houseBase, LogLevel.WARNING);
				continue;
			}

			entitySource = baseResourceObject.ToEntitySource();
			if (!entitySource)
			{
				Print("Invalid building Prefab (no entitySource) " + houseBase, LogLevel.WARNING);
				continue;
			}

			type = entitySource.GetClassName().ToType();
			if (!type || !type.IsInherited(SCR_DestructibleBuildingEntity))
				continue;

			ResourceName ruinPrefab = GetRuinFromBaseBuildingPrefab(entitySource);
			if (!ruinPrefab)
			{
				// Print("Invalid building Prefab (no ruin) " + houseBase, LogLevel.NORMAL);
				continue;
			}

			usedRuinPrefabSet.Insert(ruinPrefab);
		}

		array<ResourceName> result = {};
		foreach (ResourceName ruinPrefab : ruinPrefabs)
		{
			if (!usedRuinPrefabSet.Contains(ruinPrefab))
				result.Insert(ruinPrefab);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName GetRuinFromBaseBuildingPrefab(notnull IEntitySource entitySource)
	{
		IEntityComponentSource destrComp = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_DestructibleBuildingComponent);
		if (!destrComp)
			return ResourceName.Empty;

		BaseContainerList effects = destrComp.GetObjectArray("m_aEffects");
		if (!effects)
			return ResourceName.Empty;

		BaseContainer effect;
		ResourceName result;
		typename type;
		for (int i, count = effects.Count(); i < count; ++i)
		{
			effect = effects.Get(i);
			type = effect.GetClassName().ToType();
			if (!type || !type.IsInherited(SCR_TimedPrefab))
				continue;

			if (effect.Get("m_sRuinsPrefab", result) && result)
				return result;
		}

		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetRuinPrefabs()
	{
		if (m_bSearchInAllAddons)
			return SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_Ruin" });

		array<ResourceName> result = {};
		result.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_Ruin" }, BASE_PREFIX + PREFABS_DIR));
		result.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_Ruin" }, ADDON_PREFIX + PREFABS_DIR));
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetRuinXOBs()
	{
		if (m_bSearchInAllAddons)
			return SCR_WorkbenchHelper.SearchWorkbenchResources({ "xob" }, { "_ruin" });

		array<ResourceName> result = {};
		result.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "xob" }, { "_ruin" }, BASE_PREFIX + XOBS_DIR));
		result.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "xob" }, { "_ruin" }, ADDON_PREFIX + XOBS_DIR));
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetHouseBasePrefabs()
	{
		if (m_bSearchInAllAddons)
			return SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_base" });

		array<ResourceName> result = {};
		result.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_base" }, BASE_PREFIX + PREFABS_DIR));
		result.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_base" }, ADDON_PREFIX + PREFABS_DIR));
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Check Ruin Models and Usage", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected int CloseButton()
	{
		return 1;
	}
}
#endif
