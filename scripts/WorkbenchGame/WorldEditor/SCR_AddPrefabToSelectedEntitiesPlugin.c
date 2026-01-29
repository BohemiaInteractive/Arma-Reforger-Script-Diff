#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Add selected Prefabs to selected entities",
	description: "Add Resource Browser-selected Prefabs as first-level children to world-selected entities",
	shortcut: "Ctrl+Alt+Shift+V", // use Resource Context Menu
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF6BE,
	resourceTypes: { "et" })]
class SCR_AddPrefabToSelectedEntitiesPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		CreateSelectedPrefabsInSelectedEntities();
	}

	//------------------------------------------------------------------------------------------------
	// as the name says
	protected void CreateSelectedPrefabsInSelectedEntities(array<ResourceName> selectedResourceNames = null)
	{
		array<ResourceName> selectedResourceNamesArray = selectedResourceNames;

		WorldEditor worldEditor = SCR_WorldEditorToolHelper.GetWorldEditor();
		if (!worldEditor || worldEditor.IsPrefabEditMode())
			return;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return;

		if (worldEditorAPI.IsGameMode())
			return;

		if (!selectedResourceNamesArray || selectedResourceNamesArray.IsEmpty())
		{
			selectedResourceNamesArray = SCR_WorldEditorToolHelper.GetSelectedResources(true);
			if (!selectedResourceNamesArray || selectedResourceNamesArray.IsEmpty())
				return;
		}

		array<IEntitySource> selectedEntitySources = SCR_WorldEditorToolHelper.GetSelectedWorldEntitySources();
		if (!selectedEntitySources || selectedEntitySources.IsEmpty())
			return;

		bool beganAction = worldEditorAPI.BeginEntityAction();

		foreach (ResourceName resourceName : selectedResourceNamesArray)
		{
			foreach (int i, IEntitySource selectedEntitySource : selectedEntitySources)
			{
				if (!worldEditorAPI.CreateEntityExt(resourceName, string.Empty, 0, selectedEntitySource, vector.Zero, vector.Zero, TraceFlags.WORLD) && i == 0)
				{
					Print("Cannot create " + resourceName, LogLevel.WARNING);
					break; // skip the whole ResourceName
				}
			}
		}

		if (beganAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	override void OnResourceContextMenu(notnull array<ResourceName> resources)
	{
		CreateSelectedPrefabsInSelectedEntities(resources);
	}
}
#endif
