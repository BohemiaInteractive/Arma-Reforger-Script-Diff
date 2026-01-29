#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Snap and Orient Entities to Terrain", shortcut: "Ctrl+Page Down", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF2D1)]
class SCR_SnapAndOrientToTerrainPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return;

		worldEditorAPI.BeginEntityAction();
		
		IEntity parent;
		vector transform[4], parentTransform[4];
		vector angles, pos;
		EntityFlags flags;
		for (int i, count = worldEditorAPI.GetSelectedEntitiesCount(); i < count; i++)
		{
			IEntitySource entitySource = worldEditorAPI.GetSelectedEntity(i);
			IEntity entity = worldEditorAPI.SourceToEntity(entitySource);
			entity.GetWorldTransform(transform);
			if (SCR_TerrainHelper.SnapAndOrientToTerrain(transform, world))
			{
				parent = worldEditorAPI.SourceToEntity(worldEditorAPI.EntityToSource(entity).GetParent());
				if (parent)
				{
					parent.GetWorldTransform(parentTransform);
					Math3D.MatrixInvMultiply4(parentTransform, transform, transform);
				}

				angles = Math3D.MatrixToAngles(transform);
				worldEditorAPI.SetVariableValue(entitySource, null, "angles", string.Format("%1 %2 %3", angles[1], angles[0], angles[2]));

				pos = transform[3];
				//--- Y height relative to ground, use 0 instead of ASL height
				entitySource.Get("Flags", flags);
				if (flags & EntityFlags.RELATIVE_Y)
					pos[1] = 0;

				worldEditorAPI.SetVariableValue(entitySource, null, "coords", string.Format("%1 %2 %3", pos[0], pos[1], pos[2]));
			}
		}
		worldEditorAPI.EndEntityAction();
	}
}
#endif // WORKBENCH
