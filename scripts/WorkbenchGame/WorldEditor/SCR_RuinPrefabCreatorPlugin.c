#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Missing Ruin Prefabs Creator",
	category: "Building Destruction",
	description: "Create Prefabs for ruin models that do not have any",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF6BE)]
class SCR_RuinPrefabCreatorPlugin : WorkbenchPlugin
{
	protected static const ResourceName RUIN_BASE = "{A7C1BCA45681EB56}Prefabs/Structures/Core/BuildingRuin_base.et";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!SCR_WorldEditorToolHelper.GetWorldEditorAPI())
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		array<ResourceName> ruinModels = SCR_WorkbenchHelper.SearchWorkbenchResources({ "xob" }, { "_ruin" });
		array<ResourceName> ruinPrefabs = SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, { "_Ruin" });
		array<ResourceName> prefablessRuins = GetRuinModelsWithoutPrefabs(ruinModels, ruinPrefabs);

		Resource resource;
		BaseResourceObject resourceObject;
		IEntitySource entitySource;

		Print("treating " + prefablessRuins.Count() + " prefabless ruins", LogLevel.NORMAL);
		foreach (ResourceName resourceName : prefablessRuins)
		{
			string relativeFilePath = resourceName.GetPath();
			string absoluteFilePath;
			if (!Workbench.GetAbsolutePath(relativeFilePath, absoluteFilePath, false))
			{
				Print("Cannot get absolute file path for " + resourceName, LogLevel.WARNING);
				continue;
			}

			string absoluteFilePathPrefix = absoluteFilePath.Substring(0, absoluteFilePath.IndexOf(relativeFilePath));

			string fileName = FilePath.StripExtension(FilePath.StripPath(absoluteFilePath));
			string relativePath = FilePath.StripFileName(resourceName.GetPath()); // ends with slash
			if (!relativePath.StartsWith("Assets/")) // case-sensitive
			{
				Print("Cannot treat " + relativePath, LogLevel.NORMAL);
				continue;
			}

			string relativePrefabFilePath = "Prefabs" + relativePath.Substring(6, relativePath.Length() - 6) + fileName + ".et";
			string absolutePrefabFilePath = absoluteFilePathPrefix + relativePrefabFilePath;

			if (FileIO.FileExists(absolutePrefabFilePath))
			{
				Print("Skipping " + relativeFilePath + " as " + absolutePrefabFilePath + " already exists", LogLevel.NORMAL);
				continue;
			}

			ResourceName createdPrefab = SCR_PrefabHelper.CreatePrefabFromXOB(resourceName, absolutePrefabFilePath, RUIN_BASE, true);
			if (!createdPrefab) // .IsEmpty()
			{
				Print("Cannot create Prefab at " + absolutePrefabFilePath, LogLevel.WARNING);
				continue;
			}

			// good?
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Prints XOBs without Prefabs
	//! \param[in] parameter
	protected array<ResourceName> GetRuinModelsWithoutPrefabs(notnull array<ResourceName> ruinModels, notnull array<ResourceName> ruinPrefabs)
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
		foreach (ResourceName ruinXOB : ruinModels)
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

	/*
	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		// if (!Workbench.ScriptDialog("SCR_RuinPrefabCreatorPlugin", "<Description>", this))
		//	return;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected int ButtonClose()
	{
		return 1;
	}
	*/

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_RuinPrefabCreatorPlugin();
}
#endif
