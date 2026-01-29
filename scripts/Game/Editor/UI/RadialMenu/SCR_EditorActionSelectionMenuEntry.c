/*!
\addtogroup EditorRadialMenu
\{
*/

/*!
A wrapper class utilized by SCR_ActionsRadialMenuEditorComponent for creating a specialized Entry (SCR_SelectionMenuEntry) class to be utilized by the Editor.
*/
class SCR_EditorActionSelectionMenuEntry : SCR_SelectionMenuEntry
{
	protected ref SCR_BaseEditorAction m_Action;
	protected SCR_BaseActionsEditorComponent m_ActionsEditorComponent;
	protected vector m_vCursorWorldPos;
	protected int m_iFlags;

	//------------------------------------------------------------------------------------------------
	/*!
	Gets the underlying Editor Action this entry is created for.
	\return the Editor Action controlled by this Entry.
	*/
	SCR_BaseEditorAction GetAction()
	{
		return m_Action;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Getter for the underlying Editor Action's Shortcut.
	\return the Shortcut string for the Editor Action this Entry is created for.
	*/
	string GetActionShortcut()
	{
		if (!m_Action)
			return string.Empty;

		return m_Action.GetShortcut();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPerform()
	{
		if (!m_ActionsEditorComponent || !m_Action)
			return;

		// Try to find current cursor world position 
		SCR_MenuLayoutEditorComponent editorMenuLayout = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent, true));
		if (editorMenuLayout)
			editorMenuLayout.GetCursorWorldPos(m_vCursorWorldPos);
		
		// Perform
		m_ActionsEditorComponent.ActionPerform(m_Action, m_vCursorWorldPos, m_iFlags);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EditorActionSelectionMenuEntry(notnull SCR_BaseEditorAction action, notnull SCR_BaseActionsEditorComponent actionEditorComponent, vector cursorPosition, int actionFlags)
	{
		m_Action = action;
		m_vCursorWorldPos = cursorPosition;
		m_iFlags = actionFlags;
		m_ActionsEditorComponent = actionEditorComponent;
	}
};
/*!
\}
*/