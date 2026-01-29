[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_OrientToTerrainContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		//--- ToDo: Only when not currently oriented?
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		GenericEntity owner = selectedEntity.GetOwner();
		if (!owner) return;
		
		vector transform[4];
		selectedEntity.GetTransform(transform);
		
		if (!SCR_TerrainHelper.OrientToTerrain(transform, owner.GetWorld())) return;
		
		selectedEntity.SetTransform(transform);
	}
};