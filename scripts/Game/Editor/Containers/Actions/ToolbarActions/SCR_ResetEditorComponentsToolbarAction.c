[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_ResetEditorComponentsToolbarAction: SCR_EditorToolbarAction
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
		return GetGame().GetGameMode() != null;
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_EditorModeEntity editorModeEntity = SCR_EditorModeEntity.GetInstance();
		array<SCR_BaseEditorComponent> editorComponents = {};
		for (int i, count = editorModeEntity.FindEditorComponents(editorComponents); i < count; i++)
		{
			editorComponents[i].ResetEditorComponent();
		}
	}
};
