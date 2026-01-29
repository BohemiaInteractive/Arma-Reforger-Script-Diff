[WorkbenchPluginAttribute("Change absolute Y to relative", "Modifies entity Y position from absolute value to relative.", "", "", {"WorldEditor"},"",0xf548)]
class EntityRelativePlugin: WorkbenchPlugin
{
	override void Run()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (!we)
		{
			Print("World editor module is not available.", LogLevel.ERROR);
			return;
		}

		WorldEditorAPI api = we.GetApi();
		int selectedCount = api.GetSelectedEntitiesCount();
		if (selectedCount == 0)
		{
			Print("You need to select at least one entity.", LogLevel.ERROR);
			return;
		}

		WBProgressDialog progress = new WBProgressDialog("Processing entities", we);
		
		api.BeginEntityAction("Changing entity Y from absolute to relative");

		Print("EntityRelativePlugin: processing " + selectedCount + " entities...");
		for (int i = 0; i < selectedCount; i++)
		{
			progress.SetProgress(i / selectedCount);
			
			IEntitySource src = api.GetSelectedEntity(i);
			IEntity e = api.SourceToEntity(src);
			
			vector pos;
			src.Get("coords", pos);
			
			float y;
			if (api.TryGetTerrainSurfaceY(pos[0], pos[2], y))
			{
				pos[1] = pos[1] - y;
				api.SetVariableValue(src, null, "coords", pos.ToString(false));
			}
			else
			{
				Print("Entity " + e.GetID() + " can't relatively positioned to the terrain.", LogLevel.WARNING);
			}
		}
		
		api.EndEntityAction();
	}

	override void Configure() { }
}