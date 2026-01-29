//------------------------------------------------------------------------------------------------
class SCR_EditorModeRadialEntry : ScriptedSelectionMenuEntry
{
	private SCR_EditorModeEntity m_EditorModeEntity;
	private SCR_EditorManagerEntity m_EditorManagerEntity;
	
	//! Callback for when this entry is supposed to be performed
	override void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		super.OnPerform(user, sourceMenu);
		m_EditorManagerEntity.SetCurrentMode(m_EditorModeEntity.GetModeType());
	}

	//! Can this entry be shown?
	override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		// Only contains entries that are always shown
		return true;
	}

	//! Can this entry be performed?
	override  bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}
	
	//! If overridden and true is returned, outName is returned when BaseSelectionMenuEntry.GetEntryName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override  bool GetEntryNameScript(out string outName)
	{
		SCR_UIInfo modeInfo = m_EditorModeEntity.GetInfo();
		if (modeInfo) outName = modeInfo.GetName();
		return true;
	}
	
	//! If overridden and true is returned, outDescription is returned when BaseSelectionMenuEntry.GetEntryDescription is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override  bool GetEntryDescriptionScript(out string outDescription)
	{
		outDescription = string.Empty;
		return false;
	}
	
	//! If overridden and true is returned, outIconPath is returned when BaseSelectionMenuEntry.GetEntryIconPath is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override  bool GetEntryIconPathScript(out string outIconPath)
	{
		SCR_UIInfo modeInfo = m_EditorModeEntity.GetInfo();
		if (modeInfo) outIconPath = modeInfo.GetIconPath();
		return true;
	}
	
	void SCR_EditorModeRadialEntry(SCR_EditorModeEntity editorModeEntity, SCR_EditorManagerEntity editorManager)
	{
		m_EditorModeEntity = editorModeEntity;
		m_EditorManagerEntity = editorManager;
	}
}
