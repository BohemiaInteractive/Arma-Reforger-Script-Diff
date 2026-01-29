#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "MultiPhase Destruction Soundless Prefab Search",
	description: "Search Prefabs using multiphase destruction where Material Sound Type is set to NONE",
	wbModules: { "ResourceManager" },
	category: "Prefabs",
	awesomeFontCode: 0xF7CE)]
class SCR_MultiPhaseDestructionSoundlessPrefabSearchPlugin : WorkbenchPlugin
{
	[Attribute(desc: "Ignore SCR_DestructibleEntity entities that have destruction disabled", category: "Options")]
	protected bool m_bIgnoreDestructibleEntitiesWithDisabledDestruction;

	[Attribute(desc: "Ignore components that are disabled", category: "Options")]
	protected bool m_bIgnoreDisabledComponents;

	[Attribute(desc: "Include Prefabs having at least one Small Debris with Material Sound Type set to NONE", category: "Options")]
	protected bool m_bIncludeSilentSmallDebris;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("MultiPhase Destruction Soundless Prefab Search Options", string.Empty, this))
			return;

		DetectPrefabs();
	}

	//------------------------------------------------------------------------------------------------
	protected void DetectPrefabs()
	{
		Print("==================================================", LogLevel.NORMAL);
		Print("Looking for Prefabs using NONE as Destruction's Material Sound Type", LogLevel.NORMAL);

		if (m_bIncludeSilentSmallDebris)
			Print("Also looking for Small Debris under the same conditions", LogLevel.NORMAL);

		Print("==================================================", LogLevel.NORMAL);

		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs);

		array<ResourceName> resourceNames = {};
		foreach (string addonGUID : addonGUIDs)
		{
			string addonID = GameProject.GetAddonID(addonGUID);
			Print("Looking into " + addonID + "/Prefabs", LogLevel.NORMAL);
			resourceNames.InsertAll(SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, null, SCR_AddonTool.ToFileSystem(addonID) + "Prefabs/"));
		}

//		resourceNames.Insert("{54CE51ABCAEF05C0}PrefabLibrary/Props/Civilian/TableOld_01/TableOld_01_white.et");	// GenericEntity w/ component
//		resourceNames.Insert("{4A1FAB7E3F610EA9}PrefabLibrary/Infrastructure/Roads/CrashBarrier_01_4m_V1.et");		// SCR_DestructibleEntity
//		resourceNames.Insert("{4546AD2CC984193B}PrefabLibrary/Walls/Brick/BrickWall_02_2m.et");						// SCR_DestructibleEntity w/ Debris

		Resource resource;
		BaseContainer baseContainer;
		BaseContainer childContainer;
		BaseContainer childSubContainer;
		BaseContainerList baseContainerList;
		BaseContainerList childContainerList;
		SCR_EMaterialSoundTypeBreak value;
		bool found;
		bool isComponentEnabled;
		bool isDestructibleEntity;
		array<ResourceName> resultsGeneric = {};
		array<ResourceName> resultsDestructible = {};
		array<ResourceName> resultsGenericDebris = {};
		array<ResourceName> resultsDestructibleDestroySpawnObjectDebris = {};
		array<ResourceName> resultsDestructibleDestructionPhaseDebris = {};

		foreach (ResourceName resourceName : resourceNames)
		{
			resource = Resource.Load(resourceName);
			if (!resource.IsValid())
			{
				Print("INVALID PREFAB: error loading " + resourceName, LogLevel.ERROR);
				continue;
			}

			baseContainer = resource.GetResource().ToBaseContainer();
			if (!baseContainer)
			{
				Print("INVALID PREFAB: error loading " + resourceName, LogLevel.ERROR);
				continue;
			}

			isDestructibleEntity = baseContainer.GetClassName() == "SCR_DestructibleEntity";
			if (isDestructibleEntity)
			{
				if (m_bIgnoreDestructibleEntitiesWithDisabledDestruction)
				{
					if (!baseContainer.Get("Enabled", isComponentEnabled) || !isComponentEnabled)
						continue;
				}
			}
			else
			{
				baseContainerList = baseContainer.GetObjectArray("components");
				if (!baseContainerList)
					continue;

				for (int i, count = baseContainerList.Count(); i < count; i++)
				{
					childContainer = baseContainerList.Get(i);
					if (!childContainer)
						continue;

					if (childContainer.GetClassName() == "SCR_DestructionMultiPhaseComponent")
					{
						baseContainer = childContainer;
						break;
					}
				}

				if (!baseContainer)
					continue;

				if (m_bIgnoreDisabledComponents)
				{
					if (!baseContainer.Get("Enabled", isComponentEnabled) || !isComponentEnabled)
						continue;
				}
			}

			if (!baseContainer.Get("m_eMaterialSoundType", value))
				continue;

			if (value == SCR_EMaterialSoundTypeBreak.NONE)
			{
				if (isDestructibleEntity)
					resultsDestructible.Insert(resourceName);
				else
					resultsGeneric.Insert(resourceName);

				continue;
			}

			if (!m_bIncludeSilentSmallDebris)
				continue;

			// let's check for Small Debris

			if (isDestructibleEntity)
			{
				found = false;
				baseContainerList = baseContainer.GetObjectArray("m_aDestroySpawnObjects");
				if (baseContainerList)
				{
					// Small Debris iteration
					for (int i, count = baseContainerList.Count(); i < count; i++)
					{
						childContainer = baseContainerList.Get(i);
						if (!childContainer)
							continue;

						if (childContainer.GetClassName() != "SCR_DebrisSpawnable")
							continue;

						if (!childContainer.Get("m_eMaterialSoundType", value))
							continue;

						if (value == SCR_EMaterialSoundTypeBreak.NONE)
						{
							resultsDestructibleDestroySpawnObjectDebris.Insert(resourceName);
							found = true;
							break;
						}
					}

					if (found)
						continue;
				}

				baseContainerList = baseContainer.GetObjectArray("DamagePhases");
				if (!baseContainerList)
					continue;

				// Damage Phases iteration
				found = false;
				for (int i, count = baseContainerList.Count(); i < count; i++)
				{
					childContainer = baseContainerList.Get(i);
					if (!childContainer)
						continue;

					if (childContainer.GetClassName() != "SCR_BaseDestructionPhase")
						continue;

					childContainerList = childContainer.GetObjectArray("m_aPhaseDestroySpawnObjects");
					if (!childContainerList)
						continue;

					// Phase Destroy Spawn Objects iteration
					for (int j, countJ = childContainerList.Count(); j < countJ; j++)
					{
						childSubContainer = childContainerList.Get(j);
						if (!childSubContainer)
							continue;

						if (childSubContainer.GetClassName() != "SCR_DebrisSpawnable")
							continue;

						if (!childSubContainer.Get("m_eMaterialSoundType", value))
							continue;

						if (value == SCR_EMaterialSoundTypeBreak.NONE)
						{
							resultsDestructibleDestructionPhaseDebris.Insert(resourceName);
							found = true;
							break;
						}
					}

					if (found)
						break;
				}

				if (found)
					continue; // safety if code is added after these scopes
			}
			else
			{
				baseContainerList = baseContainer.GetObjectArray("m_DestroySpawnObjects");
				if (!baseContainerList)
					continue;

				// Phase Destroy Spawn Objects iteration
				found = false;
				for (int i, count = baseContainerList.Count(); i < count; i++)
				{
					childContainer = baseContainerList.Get(i);
					if (!childContainer)
						continue;

					if (childContainer.GetClassName() != "SCR_DebrisSpawnable")
						continue;

					if (!childContainer.Get("m_eMaterialSoundType", value))
						continue;

					if (value == SCR_EMaterialSoundTypeBreak.NONE)
					{
						resultsGenericDebris.Insert(resourceName);
						found = true;
						break;
					}
				}

				if (found)
					continue; // safety if code is added after these scopes
			}
		}

		foreach (ResourceName result : resultsGeneric)
		{
			Print("GenericEntity: NONE used as Material Sound Type in " + result, LogLevel.WARNING);
		}

		foreach (ResourceName result : resultsGenericDebris)
		{
			Print("GenericEntity Debris: NONE used as Material Sound Type in " + result, LogLevel.WARNING);
		}

		foreach (ResourceName result : resultsDestructible)
		{
			Print("DestructibleEntity: NONE used as Material Sound Type in " + result, LogLevel.WARNING);
		}

		foreach (ResourceName result : resultsDestructibleDestroySpawnObjectDebris)
		{
			Print("DestructibleEntity SpawnObject Debris: NONE used as Material Sound Type in " + result, LogLevel.WARNING);
		}

		foreach (ResourceName result : resultsDestructibleDestructionPhaseDebris)
		{
			Print("DestructibleEntity DestructionPhase Debris: NONE used as Material Sound Type in " + result, LogLevel.WARNING);
		}

		int genericCount = resultsGeneric.Count();
		int genericDebrisCount = resultsGenericDebris.Count();
		int destructibleCount = resultsDestructible.Count();
		int destructibleDestroySpawnObjectDebrisCount = resultsDestructibleDestroySpawnObjectDebris.Count();
		int destructibleDestructionPhaseDebrisCount = resultsDestructibleDestructionPhaseDebris.Count();

		int total = genericCount +
			genericDebrisCount +
			destructibleCount +
			destructibleDestroySpawnObjectDebrisCount +
			destructibleDestructionPhaseDebrisCount;

		PrintFormat("%1/%2 Prefabs found using NONE as Destruction's Material Sound Type", total, resourceNames.Count(), level: LogLevel.NORMAL);

		if (genericCount > 0)
			Print("" + genericCount + " Generic Entities", LogLevel.NORMAL);

		if (genericDebrisCount > 0)
			Print("" + genericDebrisCount + " Generic Entity Debris", LogLevel.NORMAL);

		if (destructibleCount > 0)
			Print("" + destructibleCount + " Destructible Entities", LogLevel.NORMAL);

		if (destructibleDestroySpawnObjectDebrisCount > 0)
			Print("" + destructibleDestroySpawnObjectDebrisCount + " Destructible Entity SpawnObject Debris", LogLevel.NORMAL);

		if (destructibleDestructionPhaseDebrisCount > 0)
			Print("" + destructibleDestructionPhaseDebrisCount + " Destructible Entity DestructionPhase Debris", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Process", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
#endif // WORKBENCH
