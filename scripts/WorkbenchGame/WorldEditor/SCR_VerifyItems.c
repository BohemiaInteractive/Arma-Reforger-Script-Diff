#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Verify Items", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF560)]
class SCR_VerifyItemsPlugin : WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.FileNamePicker, desc: "Root directory to search", params:"unregFolders" )]
	string RootPath;
	
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();

		WBProgressDialog progress = new WBProgressDialog("Verifying items...", worldEditor);
		progress.SetProgress(0.0);
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.rootPath = RootPath;
		filter.fileExtensions =  { "et" };
		filter.recursive = true;
		
		ref array<ResourceName> resources = {};
		ResourceDatabase.SearchResources(filter, resources.Insert);
		
		int n;
		float prevProgress, currProgress;

		for (int i = 0, count = resources.Count(); i < count; ++i)
		{
			Resource resource = Resource.Load(resources[i]);
			if (!resource.IsValid())
				continue;
			
			IEntitySource entitySource = resource.GetResource().ToEntitySource();
			if (!entitySource)
				continue;
			
			IEntityComponentSource inventoryCompSource = SCR_BaseContainerTools.FindComponentSource(entitySource, InventoryItemComponent);
			if (!inventoryCompSource)
				continue;
			
			n++;
			
			bool enabled;
			inventoryCompSource.Get("Enabled", enabled);
			if (!enabled)
				continue;
			
			if (entitySource.GetAncestor() && entitySource.GetAncestor().ToEntitySource())
			{
				IEntityComponentSource ancestorInventoryCompSource = SCR_BaseContainerTools.FindComponentSource(entitySource.GetAncestor().ToEntitySource(), InventoryItemComponent);
				if (ancestorInventoryCompSource && ancestorInventoryCompSource == inventoryCompSource)
					continue;
			}
			
			IEntityComponentSource rplCompSource = SCR_BaseContainerTools.FindComponentSource(entitySource, RplComponent);
			if (!rplCompSource)
				continue;
			
			rplCompSource.Get("Enabled", enabled);
			if (!enabled)
				continue;
			
			bool parentNodeFromParentEntity;
			rplCompSource.Get("Parent Node From Parent Entity", parentNodeFromParentEntity);
			
			if (parentNodeFromParentEntity)
			{
				PrintFormat("%1 \"Parent Node From Parent Entity\" property on RplComponent of the item is set to true", resources[i], level: LogLevel.WARNING);
			}

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}

		Print(string.Format("Verify Items: End, %1 items processed", n), LogLevel.DEBUG);
	}
}
#endif // WORKBENCH
