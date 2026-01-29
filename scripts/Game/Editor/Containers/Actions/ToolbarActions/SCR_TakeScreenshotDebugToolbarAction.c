[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TakeScreenshotDebugToolbarAction : SCR_EditorToolbarAction
{
	[Attribute("600")]
	protected int m_iResolutionX; // save dimension
	
	[Attribute("338")]
	protected int m_iResolutionY; // save dimension
	
	protected ref ImageWidget m_wScreenshotDebug;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SAVE_PHOTO_SCREENSHOT_SHOW))
			CreateDebug();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		DestroyDebug();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return m_wScreenshotDebug && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SAVE_PHOTO_SCREENSHOT_SHOW);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return m_wScreenshotDebug && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SAVE_PHOTO_SCREENSHOT_SHOW);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (!m_wScreenshotDebug)
			return;
		
		CaptureImageDebug();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Capture image of current camere view 
	void CaptureImageDebug()
	{
		SCR_PhotoSaveModeEditorUIComponent.CaptureImageDebug(m_iResolutionX, m_iResolutionY, m_wScreenshotDebug);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void CreateDebug()
	{
		if (!m_wScreenshotDebug)
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (!workspace)
				return;
			
			m_wScreenshotDebug = ImageWidget.Cast(workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.IGNORE_CURSOR, Color.White, 0));
			m_wScreenshotDebug.SetSize(m_iResolutionX, m_iResolutionY);
			FrameSlot.SetSizeToContent(m_wScreenshotDebug, true);
			FrameSlot.SetPos(m_wScreenshotDebug, 0, 0);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected void DestroyDebug()
	{
		if (m_wScreenshotDebug)
			m_wScreenshotDebug.RemoveFromHierarchy();
	}
}