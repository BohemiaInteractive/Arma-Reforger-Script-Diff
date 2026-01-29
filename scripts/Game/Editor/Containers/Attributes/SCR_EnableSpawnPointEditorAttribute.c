/**
Allows to enable and disable spawnpoints
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableSpawnPointEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(editableEntity.GetOwner());
		if (!spawnPoint)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(spawnPoint.IsSpawnPointEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(editableEntity.GetOwner());
		if (!spawnPoint)
			return;
		
		spawnPoint.SetSpawnPointEnabled_S(var.GetBool());
	}
}
