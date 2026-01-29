#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Register Placeable Entities...", category: "In-game Editor", wbModules: { "ResourceManager" })]
class RegisterPlaceableEntitiesPlugin : WorkbenchPlugin // TODO: SCR_
{
	[Attribute(desc: "File to which the resulting list will be saved.", params: "class=SCR_PlaceableEntitiesRegistry conf")]
	protected ResourceName m_sConfig;

	//------------------------------------------------------------------------------------------------
	protected void Scan()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Scanning...", resourceManager);

		//--- Open config
		if (m_sConfig.IsEmpty())
		{
			Print("Config file not found!", LogLevel.ERROR);
			return;
		}

		Resource configContainer = BaseContainerTools.LoadContainer(m_sConfig);
		if (!configContainer)
			return;

		SCR_PlaceableEntitiesRegistry registry = SCR_PlaceableEntitiesRegistry.Cast(BaseContainerTools.CreateInstanceFromContainer(configContainer.GetResource().ToBaseContainer()));
		if (!registry)
		{
			Print(string.Format("SCR_PlaceableEntitiesRegistry class not found in '%1'!", m_sConfig), LogLevel.ERROR);
			return;
		}

		string filesystem = SCR_AddonTool.ToFileSystem(registry.GetAddon());
		string directory = registry.GetSourceDirectory().GetPath();

		//--- Get all prefabs
		array<ResourceName> resources = {};
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = { "et" };
		filter.rootPath = filesystem + directory;
		ResourceDatabase.SearchResources(filter, resources.Insert);

		array<ResourceName> prefabs = {};

		//--- Iterate through all prefabs and their components
		Resource container;
		BaseResourceObject object;
		IEntitySource entity;
		IEntityComponentSource editableEntityComponent;
		IEntityComponentSource component;
		int resourcesCount = resources.Count();
		float prevProgress, currProgress;
		foreach (int i, ResourceName prefab : resources)
		{
			container = BaseContainerTools.LoadContainer(prefab);
			if (!container)
				continue;

			object = container.GetResource();
			entity = object.ToEntitySource();

			//--- Find required components
			int componentCount = entity.GetComponentCount();
			editableEntityComponent = null;
			for (int c = 0; c < componentCount; c++)
			{
				component = entity.GetComponent(c);

				if (component.GetClassName().ToType().IsInherited(SCR_EditableEntityComponent))
					editableEntityComponent = component;
			}

			//--- Register
			if (editableEntityComponent)
			{
				//--- Check if it's marked as PLACEABLE
				if (!SCR_EditableEntityComponentClass.HasFlag(editableEntityComponent, EEditableEntityFlag.PLACEABLE))
					continue;

				Print(string.Format("Registered '%1'", prefab), LogLevel.DEBUG);
				prefabs.Insert(prefab);
			}

			currProgress = i / resourcesCount;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}

		resources.Clear();

		//--- Save config to the file
		registry.SetPrefabs(prefabs);
		container = BaseContainerTools.CreateContainerFromInstance(registry);
		if (container)
		{
			BaseContainerTools.SaveContainer(container.GetResource().ToBaseContainer(), m_sConfig);
			resourceManager.SetOpenedResource(m_sConfig);
			resourceManager.Save();
			Print(string.Format("Entities from '%1' saved to '%2'", directory, m_sConfig.GetPath()), LogLevel.DEBUG);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (Workbench.ScriptDialog("Register Placeable Entities", "Register all entity prefabs that have SCR_EditableEntityComponent\nwith PLACEABLE flag into target config.\nThe game uses it to get the list of all entities available for placing.\n\nMake sure you don't have the target config open!", this))
			Scan();
	}

	//------------------------------------------------------------------------------------------------
	override void RunCommandline()
	{
		Scan();
		Workbench.Exit(0);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run")]
	protected bool OK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool Cancel()
	{
		return false;
	}
}
#endif // WORKBENCH
