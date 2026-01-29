[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SnapToTerrainContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		vector pos;
		return selectedEntity.GetPos(pos)
			&& selectedEntity.GetEntityType() != EEditableEntityType.CHARACTER //--- ToDo: Don't compare to specific types
			&& !selectedEntity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE); //--- No applicable for slots or location labels
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		//--- ToDo: Only when not currently snapped?
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		GenericEntity owner = selectedEntity.GetOwner();
		if (!owner) return;
		
		vector transform[4];
		selectedEntity.GetTransform(transform);
		
		if (!SCR_TerrainHelper.SnapToTerrain(transform, owner.GetWorld())) return;
		
		selectedEntity.SetTransformWithChildren(transform);
	}
};