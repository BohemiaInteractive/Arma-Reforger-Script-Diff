/*
#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Convert to/from Editable Entities...", category: "In-game Editor", shortcut: "Ctrl+Shift+E", wbModules: { "WorldEditor" })]
class SCR_ConvertEditableEntitiesPlugin : PrefabEditingPluginBase
{
	[Attribute(defvalue: "{90B4D95CF3610F3D}Configs/Workbench/EditablePrefabs/EditablePrefabsConfig.conf", desc: "", params: "conf")]
	private ResourceName m_Config;

	//------------------------------------------------------------------------------------------------
	protected void Convert(bool toEditable)
	{
		EditablePrefabsConfig config = EditablePrefabsConfig.GetConfig(m_Config);
		if (!config) return;
		config.GetLinks();

		array<ResourceName> selectedPrefabs = new array<ResourceName>;
		if (!GetPrefabs(selectedPrefabs)) return;

		foreach (ResourceName prefab : selectedPrefabs)
		{
			config.UpdateEditableChildPrefabs(prefab, toEditable);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Editable Entities Converter", "Scans all entities within selected prefabs and replaces them by...\n[To Editable] ... prefabs configured for use in in-game editor\n[To Non-editable] ... original prefabs", this)) return;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("To Editable")]
	protected void ButtonToEditable()
	{
		Convert(true);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("To Non-editable")]
	protected void ButtonToNonEditable()
	{
		Convert(false);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
[WorkbenchPluginAttribute(name: "Convert to/from Editable Entities...", category: "In-game Editor", shortcut: "Ctrl+Shift+E", wbModules: { "ResourceManager" })]
class SCR_ConvertEditableEntitiesPluginResourceManager : SCR_ConvertEditableEntitiesPlugin
{
	//------------------------------------------------------------------------------------------------
	override void GetSelected(out array<ResourceName> selection)
	{
		GetSelectedResourceBrowser(selection);
	}
}
#endif
*/
