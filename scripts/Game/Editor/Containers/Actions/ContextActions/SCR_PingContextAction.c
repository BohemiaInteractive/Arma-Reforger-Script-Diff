[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_PingContextAction : SCR_BaseContextAction
{
	[Attribute("0")]
	bool m_IsUnlimitedOnly;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		// Only visible on gamepad
		if (flags & EEditorContextActionFlags.USING_MOUSE_AND_KEYBOARD)
		{
			return false;
		}
		
		//~ Check if unlimited editor else do not show (Otherwise players can see the action in photomode)
		if (m_IsUnlimitedOnly)
		{
			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (!editorManager || editorManager.IsLimited())
				return false;
		} 
		
		// Only show action if cursorWorldPosition is valid
		return cursorWorldPosition != vector.Zero;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PingEditorComponent pingComponent = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent));
		
		pingComponent.SendPing(m_IsUnlimitedOnly, cursorWorldPosition, hoveredEntity);
	}
};