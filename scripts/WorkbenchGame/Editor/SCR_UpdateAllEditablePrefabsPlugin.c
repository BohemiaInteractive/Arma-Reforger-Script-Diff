#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Update All Editable Prefabs", category: "In-game Editor", wbModules: { "WorldEditor", "ResourceManager" }, awesomeFontCode: 0xF1B3)]
class SCR_UpdateAllEditablePrefabsPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "{90B4D95CF3610F3D}Configs/Workbench/EditablePrefabs/EditablePrefabsConfig.conf", desc: "", params: "conf")]
	protected ResourceName m_sConfig;

	[Attribute(desc: "Enable to only check for renamed or delete files, don't update their content.")]
	protected bool m_bOnlyFileChanges;

	protected static const string META_EXTENSION = ".meta";

	//------------------------------------------------------------------------------------------------
	protected void DeleteEmptyDirectoriesCallback(string path, FileAttribute attributes)
	{
		if ((attributes & FileAttribute.DIRECTORY) && FileIO.DeleteFile(path)) //--- DeleteFile fails when the folder is not empty
			Print(string.Format("Folder DELETED: '%1' (was empty)", path), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteEmptyDirectories(string path)
	{
		FileIO.FindFiles(DeleteEmptyDirectoriesCallback, path, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Configure 'Update All Editable Prefabs' plugin", "Batch operation applied on all auto-generated editable prefabs\n(check the config to see which folders will be affected).\n\nWARNING: This operation can take several minutes to perform!", this))
			return;

		EditablePrefabsConfig config = EditablePrefabsConfig.GetConfig(m_sConfig);
		if (!config || !config.IsValid())
			return;

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
			return;

		Workbench.OpenModule(WorldEditor);
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		//--- Ask first
		//SCR_ConfirmationDialogWorkbench dialog = new SCR_ConfirmationDialogWorkbench();
		//if (!Workbench.ScriptDialog("", "Do you want to update all editable prefabs?", dialog))
		//	return;

		string targetPath = config.GetTargetPathAuto();

		WBProgressDialog progress = new WBProgressDialog("Processing Prefabs...", resourceManager);
		Print(string.Format("Updating editable entity prefabs STARTED at '%1'", targetPath), LogLevel.DEBUG);

		if (!config.CreateWorld())
			return;

		//--- Get all files
		array<ResourceName> resources = {};
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = { "et" };
		ResourceDatabase.SearchResources(filter, resources.Insert);
		int resourcesCount = resources.Count();

		string prefabPath;
		array<string> toRebuild = {};

		api.BeginEntityAction();
		float prevProgress, currProgress;
		foreach (int i, ResourceName prefab : resources)
		{
			prefabPath = prefab.GetPath();
			if (!prefabPath.StartsWith(targetPath))
				continue;

			prefabPath = config.VerifyEditablePrefab(api, prefab, m_bOnlyFileChanges);
			if (!prefabPath.IsEmpty())
				toRebuild.Insert(prefabPath);

			currProgress = i / resourcesCount;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}

		api.EndEntityAction();
		resources.Clear();

		resourceManager.RebuildResourceFiles(toRebuild, "PC");

		DeleteEmptyDirectories(targetPath);

		worldEditor.Save(); //--- Save to world so it's not marked as edited

		Print(string.Format("Updating editable entity prefabs FINISHED at '%1'", targetPath), LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run")]
	protected bool ButtonRun()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return false;
	}
}

class SCR_ConfirmationDialogWorkbench
{
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
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
