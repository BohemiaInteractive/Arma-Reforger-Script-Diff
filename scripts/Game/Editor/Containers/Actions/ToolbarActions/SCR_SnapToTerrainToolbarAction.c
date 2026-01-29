// Script File
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SnapToTerrainToolbarAction : SCR_EditorToolbarAction
{
	override bool IsServer()
	{
		return false;
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return selectedEntities.Count() > 0;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			GenericEntity owner = entity.GetOwner();
			if (!owner) continue;
		
			vector transform[4];
			entity.GetTransform(transform);
		
			if (!SCR_TerrainHelper.SnapToTerrain(transform, owner.GetWorld())) return;
		
			entity.SetTransform(transform);
		}
	}
};