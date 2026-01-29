#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: " Ruin Prefab Assigner",
	category: "Building Destruction",
	description: "Assign Ruin Prefab to building Prefabs with a Ruin in the same directory",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF6BE)]
class SCR_RuinPrefabAssignerPlugin : WorkbenchPlugin
{
	protected static const string PREFAB_EXTENSION = "et";
	protected static const string PREFAB_EXTENSION_DOTTED = "." + PREFAB_EXTENSION;

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
//		if (!worldEditorAPI)
//		{
//			Print("World Editor API is not available", LogLevel.WARNING);
//			return;
//		}

		Resource resource;
		BaseResourceObject baseResourceObject;
		IEntitySource entitySource;
		IEntityComponentSource componentSource;
		BaseContainerList effects;
		BaseContainer effect;
		typename type;
		array<ref ContainerIdPathEntry> weApiPath = {};
		array<string> ruinPrefabEndings = { "_ruin", "_Ruin", "_ruins", "_Ruins" };
		array<string> basePrefabEndings = { "_base", "_Base" };
		array<ResourceName> ruinPrefabs = SCR_WorkbenchHelper.SearchWorkbenchResources({ PREFAB_EXTENSION }, { "_Ruin" }); // covers both _ruin and _ruins
		foreach (ResourceName ruinPrefab : ruinPrefabs)
		{
			string ruinRelativeFilePath = ruinPrefab.GetPath();
			if (!ruinRelativeFilePath)
				continue;

			string ruinFileName = FilePath.StripExtension(FilePath.StripPath(ruinRelativeFilePath));
			if (ruinFileName.EndsWith("_base"))
				continue;

			if (!SCR_StringHelper.EndsWithAny(ruinFileName, ruinPrefabEndings)) // ...it should
			{
				Print("skipping an invalid ruin filename (must END with _[Rr]uin(s), no? what's the standard?) - " + ruinRelativeFilePath, LogLevel.WARNING);
				continue;
			}

			string relativeDirectory = FilePath.StripFileName(ruinRelativeFilePath);
			if (!relativeDirectory.EndsWith(SCR_StringHelper.SLASH))
				relativeDirectory += SCR_StringHelper.SLASH;

			string buildingFileNameWithoutExtension = ruinFileName.Substring(0, ruinFileName.LastIndexOf(SCR_StringHelper.UNDERSCORE));
			string baseBuildingRelativeFilePath;
			foreach (string basePrefabEnding : basePrefabEndings)
			{
				if (FileIO.FileExists(relativeDirectory + buildingFileNameWithoutExtension + basePrefabEnding + PREFAB_EXTENSION_DOTTED))
				{
					baseBuildingRelativeFilePath = relativeDirectory + buildingFileNameWithoutExtension + basePrefabEnding + PREFAB_EXTENSION_DOTTED;
					break;
				}
			}

			if (!baseBuildingRelativeFilePath)
			{
				Print("Cannot find the base building Prefab for " + ruinPrefab, LogLevel.WARNING);
				continue;
			}

			ResourceName baseBuildingPrefab = Workbench.GetResourceName(baseBuildingRelativeFilePath);
			if (!baseBuildingPrefab)
			{
				Print("Skipping base building Prefab, not registered - " + baseBuildingRelativeFilePath, LogLevel.WARNING);
				continue;
			}

			resource = Resource.Load(baseBuildingPrefab);
			if (!resource.IsValid())
				continue;

			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
				continue;

			entitySource = baseResourceObject.ToBaseContainer();
			if (!entitySource)
				continue;

			int componentIndex = SCR_BaseContainerTools.FindComponentIndex(entitySource, SCR_DestructibleBuildingComponent);
			if (componentIndex < 0)
			{
				Print("Base building Prefab does not have an SCR_DestructibleBuildingComponent component - " + baseBuildingPrefab, LogLevel.WARNING);
				continue;
			}

			componentSource = entitySource.GetComponent(componentIndex);
			if (!componentSource)
			{
				Print("Base building Prefab does not have an SCR_DestructibleBuildingComponent component (but has an index?!) - " + baseBuildingPrefab, LogLevel.WARNING);
				continue;
			}

			effects = componentSource.GetObjectArray("m_aEffects");
			if (!effects)
			{
				Print("Base building Prefab does not have SCR_DestructibleBuildingComponent's m_aEffects initialised - " + baseBuildingPrefab, LogLevel.WARNING);
				continue;
			}

			bool found;
			int effectIndex = -1;
			ResourceName baseBuildingRuinPrefab;
			int effectsCount = effects.Count();
			for (int i; i < effectsCount; ++i)
			{
				effect = effects.Get(i);
				type = effect.GetClassName().ToType();
				if (!type || !type.IsInherited(SCR_TimedPrefab))
					continue;

				if (effect.Get("m_sRuinsPrefab", baseBuildingRuinPrefab))
				{
					if (baseBuildingRuinPrefab)
					{
						if (baseBuildingRuinPrefab == ruinPrefab)
						{
							Print("Ruin already set - " + baseBuildingPrefab + " ruin is " + ruinFileName, LogLevel.NORMAL);
							found = true;
							break;
						}
						else
						{
							Print("Ruin already set to something different - " + baseBuildingPrefab + "\nwant to set: " + ruinPrefab + "\nis currently: " + baseBuildingRuinPrefab, LogLevel.NORMAL);
							found = true;
							break;
						}
					}
					else // found empty
					{
						effectIndex = i;
						break;
					}
				}
			}

			if (found)
				continue;

			weApiPath.Clear();
			weApiPath.Insert(new ContainerIdPathEntry("Components", componentIndex));
			if (effectIndex < 0) // create it
			{
				if (!worldEditorAPI.CreateObjectArrayVariableMember(entitySource, weApiPath, "m_aEffects", "SCR_TimedPrefab", effectsCount))
				{
					Print("Cannot create a new m_aEffects member - " + baseBuildingPrefab, LogLevel.WARNING);
					continue;
				}

				Print("Successful creation of a new m_aEffects member", LogLevel.NORMAL);
				effectIndex = effectsCount;
			}

			weApiPath.Insert(new ContainerIdPathEntry("m_aEffects", effectIndex));
			if (!worldEditorAPI.SetVariableValue(entitySource, weApiPath, "m_sRuinsPrefab", ruinPrefab))
			{
				Print("Cannot set m_sRuinsPrefab to the wanted value - " + baseBuildingPrefab, LogLevel.WARNING);
				continue;
			}

			// good?
		}
	}

	/*
	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		// if (!Workbench.ScriptDialog("SCR_RuinPrefabAssignerPlugin", "<Description>", this))
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
	protected void SCR_RuinPrefabAssignerPlugin();
}
#endif
