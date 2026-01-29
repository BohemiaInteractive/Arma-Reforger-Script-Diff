#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Entities Setup",
	category: "World Setup",
	description: "Set up world's Entities",
	shortcut: "", // "Ctrl+T", same as Script Editor Template
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF0AC)] // F7A2 is already used
class SCR_WorldSetupPlugin_Entities : SCR_WorldSetupPluginBasePlugin
{
	[Attribute(defvalue: DEFAULT_CONFIG, params: "conf class=SCR_WorldSetupPluginConfig")]
	protected ResourceName m_sConfig;

	[Attribute(defvalue: "0", desc: "Create Prefab's child from the provided one in the selected addon and use it instead of using the provided Prefab directly")]
	protected bool m_bCopyPrefabsToAddon;

	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "Addon in which the Prefabs will be copied (if copied)", enums: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 2))]
	protected int m_iAddon;

	// temp work variables
	protected ref SCR_WorldSetupPluginConfig m_Config;
	protected ref set<string> m_ClassNames = new set<string>();
	protected ref set<string> m_Prefabs = new set<string>();

	protected static const ResourceName DEFAULT_CONFIG = "{1DD914C62E44CDEB}Configs/Workbench/WorldEditor/WorldSetupPlugin/0_MinimalWorldSetup.conf";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		if (!Init())
			return;

		if (!Workbench.ScriptDialog(
				"World Setup (required entities creation)",
				"This plugin helps generating default entities to setup a freshly created world properly.\n" +
					"Pick a config of your choice (keep the default one if it is good enough).\n\n" +
					"Entities will be created in the currently selected layer.",
				this))
			return;

		if (!LoadConfig())
			return;

		CreateEntities();
		Cleanup();
	}

	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		// has API?
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("Could not obtain worldEditorAPI", LogLevel.ERROR);
			return false;
		}

		// has world?
		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		if (!baseWorld)
		{
			Print("No world is currently loaded", LogLevel.WARNING);
			return false;
		}

		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);
		if (worldPath.IsEmpty())
		{
			Print("No world is currently loaded or the world has not yet been saved to storage", LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadConfig()
	{
		if (m_sConfig.IsEmpty())
			m_sConfig = DEFAULT_CONFIG;

		Resource resource = Resource.Load(m_sConfig);
		if (!resource.IsValid())
		{
			Print("Could not load config", LogLevel.ERROR);
			return false;
		}

		m_Config = new SCR_WorldSetupPluginConfig();
		BaseContainerTools.WriteToInstance(m_Config, resource.GetResource().ToBaseContainer());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateEntities()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		// array<IEntitySource> entitySources = {};
		// array<IEntity> entities = {};
		IEntitySource worldEntitySource;
		IEntitySource entitySource;
		BaseContainer ancestor;
		for (int i, count = worldEditorAPI.GetEditorEntityCount(); i < count; i++)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (!worldEntitySource && entitySource.GetClassName().ToType() == GenericWorldEntity)
				worldEntitySource = entitySource;

			// entitySources.Insert(entitySource);
			// entities.Insert(worldEditorAPI.SourceToEntity(entitySource));
			m_ClassNames.Insert(entitySource.GetClassName());

			ancestor = entitySource.GetAncestor();
			if (ancestor)
				m_Prefabs.Insert(ancestor.GetResourceName());
		}

		// create Entities
		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();
		foreach (SCR_WorldSetupPluginConfig_Entity entry : m_Config.m_aEntities)
		{
			CreateEntityFromEntry(entry);
		}
		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);

		Print("Config applied successfully", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource CreateEntityFromEntry(notnull SCR_WorldSetupPluginConfig_Entity entry)
	{
		if (entry.m_sPrefab.IsEmpty())
		{
			Print("An entry does not have class name or Prefab", LogLevel.WARNING);
			return null;
		}

		if (entry.m_bMustBeUniqueByPrefab && m_Prefabs.Contains(entry.m_sPrefab)) // TODO: inheritance?
		{
			Print("Entry skipped as another Entity originates from the same Prefab " + entry.m_sPrefab, LogLevel.WARNING);
			return null;
		}

		Resource resource = Resource.Load(entry.m_sPrefab);
		if (!resource.IsValid())
		{
			Print("Entry has an invalid Prefab " + entry.m_sPrefab, LogLevel.WARNING);
			return null;
		}

		string className = resource.GetResource().ToBaseContainer().GetClassName();

		if (entry.m_bMustBeUniqueByClassName && m_ClassNames.Contains(className))
		{
			Print("Entry skipped as a " + className + " entity is already present", LogLevel.WARNING);
			return null;
		}

		ResourceName prefab = entry.m_sPrefab;

		// duplicate the Prefab
		if (m_bCopyPrefabsToAddon)
			prefab = CreatePrefabChildInAddon(prefab, m_iAddon);

		if (prefab.IsEmpty())
		{
			Print("Child Prefab could not be created ", LogLevel.WARNING);
			return null;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		IEntitySource entitySource = worldEditorAPI.CreateEntity(prefab, string.Empty, worldEditorAPI.GetCurrentEntityLayerId(), null, entry.m_vPosition, entry.m_vAngles);
		if (!entitySource)
		{
			Print("Entity failed to be created by World Editor API " + prefab, LogLevel.WARNING);
			return null;
		}

		if (entry.m_aAdditionalValues && !entry.m_aAdditionalValues.IsEmpty())
		{
			foreach (SCR_WorldSetupPluginConfig_EntitySourceKeyValue kvp : entry.m_aAdditionalValues)
			{
				if (!worldEditorAPI.SetVariableValue(entitySource, null, kvp.m_sKey, kvp.m_sValue))
					PrintFormat("Cannot set variable value \"%1\" (value: %2)", kvp.m_sKey, kvp.m_sValue, level: LogLevel.WARNING);
			}
		}

		m_ClassNames.Insert(className);
		m_Prefabs.Insert(prefab);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	protected void Cleanup()
	{
		m_Config = null;
		m_ClassNames.Clear();
		m_Prefabs.Clear();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	protected bool ButtonRun()
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
