[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_MapToolbarAction : SCR_EditorToolbarAction
{
	override bool IsServer()
	{
		return false;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		SCR_MapEditorComponent mapEditorComponent = SCR_MapEditorComponent.Cast(SCR_MapEditorComponent.GetInstance(SCR_MapEditorComponent));
		return mapEditorComponent && mapEditorComponent.GetMapAvailable();
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_MapEditorComponent mapEditorComponent = SCR_MapEditorComponent.Cast(SCR_MapEditorComponent.GetInstance(SCR_MapEditorComponent));
		if (mapEditorComponent)
		{
			mapEditorComponent.ToggleMap();
		}
	}
};