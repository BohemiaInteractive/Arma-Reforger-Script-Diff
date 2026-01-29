[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_FocusHereContextAction : SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return false;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_CameraEditorComponent editorCamera = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (!editorCamera)
		{
			Print("SCR_FocusHereContextAction - failed to find editor cam base", LogLevel.WARNING);
			return;
		}
		
		SCR_ManualCamera baseCamera = editorCamera.GetCamera();
		if (!baseCamera)
		{
			Print("SCR_FocusHereContextAction - failed to find base editor cam", LogLevel.WARNING);
			return;
		}
		
		SCR_FocusManualCameraComponent focus = SCR_FocusManualCameraComponent.Cast(baseCamera.FindCameraComponent(SCR_FocusManualCameraComponent));
		if (!focus)
		{
			Print("SCR_FocusHereContextAction - failed to find editor cam focus", LogLevel.WARNING);
			return;
		}
		
		focus.SetFocusToCursor();
	}
};