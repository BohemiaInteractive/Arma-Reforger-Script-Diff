[WorkbenchPluginAttribute("Set entity to view orientation", "Modifies position and rotation of selected entity to match that of editor view.", "", "", {"WorldEditor"},"",0xf21d)]
class EntityOrientationTool: WorkbenchPlugin
{
	override void Run()
	{
		WorldEditor mod = Workbench.GetModule(WorldEditor);
		if (!mod)
		{
			Print("World editor module is not available.", LogLevel.ERROR);
			return;
		}

		WorldEditorAPI api = mod.GetApi();
		int selectedCount = api.GetSelectedEntitiesCount();
		if (selectedCount != 1)
		{
			Print("You can only set orientation of single entity, but you have " + selectedCount + " entities selected.", LogLevel.ERROR);
			return;
		}

		vector mat[4];
		IEntitySource src = api.GetSelectedEntity(0);
		IEntity e = api.SourceToEntity(src);
		e.GetWorld().GetCamera(19, mat);
		
		IEntity parent = api.SourceToEntity(src.GetParent());
		if (parent)
		{			
			vector mat_parent[4];
			parent.GetTransform(mat_parent);
			
			vector mat_local[4];
			Math3D.MatrixInvMultiply4(mat_parent, mat, mat_local);

			mat = mat_local;
		}

		vector yawPitchRoll = Math3D.MatrixToAngles(mat);
		vector pos = mat[3];
		
		if ((e.GetFlags() & EntityFlags.RELATIVE_Y) != 0)
		{
			float y;
			if (api.TryGetTerrainSurfaceY(pos[0], pos[2], y))
			{
				pos[1] = pos[1] - y;
			}
		}
		
		api.BeginEntityAction("Set entity to view orientation script");
		api.SetVariableValue(src, null, "coords", pos.ToString(false));
		api.SetVariableValue(src, null, "angles", string.Format("%1 %2 %3", yawPitchRoll[1], yawPitchRoll[0], yawPitchRoll[2]));
		api.EndEntityAction();
	}

	override void Configure() { }
}

[WorkbenchPluginAttribute(name: "Set camera orientation to entity", description: "Modifies position and rotation of editor view to match selected entity.", shortcut: "Ctrl+Shift+C", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf083)]
class CameraOrientationTool: WorkbenchPlugin
{
	override void Run()
	{
		WorldEditor mod = Workbench.GetModule(WorldEditor);
		if (!mod)
		{
			Print("World editor module is not available.", LogLevel.ERROR);
			return;
		}

		WorldEditorAPI api = mod.GetApi();
		int selectedCount = api.GetSelectedEntitiesCount();
		if (selectedCount != 1)
		{
			Print("You can only set orientation of single entity, but you have " + selectedCount + " entities selected.", LogLevel.ERROR);
			return;
		}

		vector mat[4];
		IEntity e = api.SourceToEntity(api.GetSelectedEntity(0));
		e.GetTransform(mat);
		
		api.SetCamera(mat[3], mat[2]);
	}

	override void Configure() { }
}