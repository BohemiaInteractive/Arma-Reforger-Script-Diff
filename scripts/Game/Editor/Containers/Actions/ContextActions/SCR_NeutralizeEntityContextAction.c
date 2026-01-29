//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_NeutralizeEntityContextAction : SCR_SelectedEntitiesContextAction
{
	override int GetParam()
	{
		return GetGame().GetPlayerController().GetPlayerId();
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity && selectedEntity.CanDestroy();
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity && selectedEntity.CanDestroy() && !selectedEntity.IsDestroyed();
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!InitPerform()) 
			return;
				
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (CanBePerformed(entity, cursorWorldPosition, flags))
			{
				entity.Destroy(param);
			}
		}
	}
};