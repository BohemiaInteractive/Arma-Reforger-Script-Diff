#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Create/Update Selected Editable Prefabs", category: "In-game Editor", shortcut: "Ctrl+Shift+U", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF1B2)]
class SCR_CreateEditablePrefabsPlugin : SCR_PrefabEditingPluginBase
{
	[Attribute(defvalue: "{90B4D95CF3610F3D}Configs/Workbench/EditablePrefabs/EditablePrefabsConfig.conf", desc: "", params: "conf")]
	protected ResourceName m_sConfig;

	[Attribute(desc: "[UNUSED] When enabled, the process will only update existing editable entities, not create new ones.")]
	protected bool m_bOnlyUpdate;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		//TODO: m_bOnlyUpdate = false is Hotfix needs to be fixed and removed
		m_bOnlyUpdate = false;

		Workbench.OpenModule(WorldEditor); // also used by SCR_CreateEditablePrefabsPluginResourceManager which is a ResourceManager plugin
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		EditablePrefabsConfig config = EditablePrefabsConfig.GetConfig(m_sConfig);
		if (!config || !config.IsValid())
			return;

		config.GetLinks();
		if (!config.CreateWorld())
			return;

		//--- Get selected entities
		array<ResourceName> selectedPrefabs = {};
		if (!GetPrefabs(selectedPrefabs))
			return;

		map<string, SCR_EEditablePrefabResult> results = new map<string, SCR_EEditablePrefabResult>();

		WBProgressDialog progress = new WBProgressDialog("Processing Prefabs...", worldEditor);

		//--- Iterate through all entities
		api.BeginEntityAction();
		int count = selectedPrefabs.Count();
		float prevProgress, currProgress;
		foreach (int i, ResourceName selectedPrefab : selectedPrefabs)
		{
			config.CreateEditablePrefab(api, selectedPrefab, m_bOnlyUpdate, results);

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}
		api.EndEntityAction();

		worldEditor.Save(); //--- Save to world so it's not marked as edited

		config.LogResults(results);
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Create/Update Selected Editable Prefabs' plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return false;
	}
}

[WorkbenchPluginAttribute(name: "Create/Update Selected Editable Prefabs", category: "In-game Editor", shortcut: "Ctrl+Shift+U", wbModules: { "ResourceManager" })]
class SCR_CreateEditablePrefabsPluginResourceManager : SCR_CreateEditablePrefabsPlugin
{
	//------------------------------------------------------------------------------------------------
	override void GetSelected(out array<ResourceName> selection)
	{
		GetSelectedResourceBrowser(selection);
	}
}
#endif // WORKBENCH
