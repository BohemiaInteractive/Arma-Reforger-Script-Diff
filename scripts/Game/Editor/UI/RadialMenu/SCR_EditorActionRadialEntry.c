//------------------------------------------------------------------------------------------------
class SCR_EditorActionRadialEntry : ScriptedSelectionMenuEntry
{
	private ref SCR_EditorActionData m_ActionData;
	private SCR_BaseActionsEditorComponent m_EditorComponent;
	private vector m_CursorWorldPosition;
	private int m_Flags;
	
	//! Callback for when this entry is supposed to be performed
	override void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		super.OnPerform(user, sourceMenu);
		m_EditorComponent.ActionPerform(m_ActionData.GetAction(), m_CursorWorldPosition, m_Flags);
	}

	//! Can this entry be shown?
	override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		// Context Action Radial Menu only contains entries that are always shown
		return true;
	}

	//! Can this entry be performed?
	override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return m_ActionData.GetCanBePerformed();
	}
	
	//! If overridden and true is returned, outName is returned when BaseSelectionMenuEntry.GetEntryName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetEntryNameScript(out string outName)
	{
		outName = m_ActionData.GetAction().GetInfo().GetName();
		return true;
	}
	
	//! If overridden and true is returned, outDescription is returned when BaseSelectionMenuEntry.GetEntryDescription is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetEntryDescriptionScript(out string outDescription)
	{
		outDescription = m_ActionData.GetAction().GetInfo().GetDescription();
		return true;
	}
	
	//! If overridden and true is returned, outIconPath is returned when BaseSelectionMenuEntry.GetEntryIconPath is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetEntryIconPathScript(out string outIconPath)
	{
		outIconPath = m_ActionData.GetAction().GetInfo().GetIconPath();
		return true;
	}
	
	void SCR_EditorActionRadialEntry(SCR_EditorActionData actionData, SCR_BaseActionsEditorComponent component, vector cursorWorldPosition, EEditorContextActionFlags flags = 0)
	{
		m_ActionData = actionData;
		m_EditorComponent = component;
		m_CursorWorldPosition = cursorWorldPosition;
		m_Flags = flags;
	}
	
	void ~SCR_EditorActionRadialEntry()
	{
		m_ActionData = null;
	}
};