[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_LeaveModeToolbarAction: SCR_EditorToolbarAction
{
	[Attribute("0", UIWidgets.CheckBox ,desc: "True - will select different mode, False - close editor")]
	protected bool m_bSwitchToMode;
	
	[Attribute(string.Format("%1", EEditorMode.EDIT), desc: "Mode which should be open", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMode))]
	protected EEditorMode m_eModeToLeave;
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;
		
		// Switch to mode 
		if (m_bSwitchToMode)
		{
			editorManagerEntity.SetCurrentMode(m_eModeToLeave);
			return;
		}
		
		// Close editor 
		editorManagerEntity.Close();
	}
}