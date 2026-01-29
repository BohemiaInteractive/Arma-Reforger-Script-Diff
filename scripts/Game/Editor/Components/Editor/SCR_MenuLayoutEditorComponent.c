[ComponentEditorProps(category: "GameScripted/Editor", description: "Layout for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_MenuLayoutEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute(category: "Editable Entity UI")]
	protected ref SCR_EditableEntityUIConfig m_EditableEntityUI;
	
	SCR_EditableEntityUIConfig GetEditableEntityUI()
	{
		return m_EditableEntityUI;
	}
};

/** @ingroup Editor_Components
*/
class SCR_MenuLayoutEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_AlwaysShownLayout;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_HideableLayout;
	
	protected Widget m_AlwaysShownWidget;
	protected Widget m_HideableWidget;
	protected SCR_CursorEditorUIComponent m_CursorComponent;
	
	/*!
	Get configuration of editable entity UI.
	\return Editable entity UI config
	*/
	SCR_EditableEntityUIConfig GetEditableEntityUI()
	{
		SCR_MenuLayoutEditorComponentClass prefabData = SCR_MenuLayoutEditorComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetEditableEntityUI();
		else
			return null;
	}
	
	/*!
	Get world position below cursor.
	\param[out] worldPos Vector to be filled with world position
	\param flags Trace flags to be used (send -1 for default flags)
	\return True if the cursor is above world position (e.g., not pointing at sky)
	*/
	bool GetCursorWorldPos(out vector worldPos, TraceFlags flags = -1)
	{
		if (m_CursorComponent)
			return m_CursorComponent.GetCursorWorldPos(worldPos, flags: flags);
		else
			return false;
	}
	/*!
	Get entity under cursor, even if it's not editable.
	To get editable entity under cursor, use SCR_BaseEditableEntityFilter of type EEditableEntityState.HOVER.
	\return Entity under cursor
	*/
	IEntity GetTraceEntity()
	{
		if (m_CursorComponent)
			return m_CursorComponent.GetTraceEntity();
		else
			return null;
	}
	
	override void EOnEditorPostActivate()
	{
		SCR_MenuEditorComponent menuEditor = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuEditor)
		{
			Print("SCR_MenuLayoutEditorComponent requires SCR_MenuEditorComponent!", LogLevel.ERROR);
			return;
		}
		
		EditorMenuBase menu = menuEditor.GetMenu();
		if (!menu) return;
	
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return;
		
		//--- Find widget which is parent for hiding - layout will be created under it, so it's hidden as well
		m_AlwaysShownWidget = workspace.CreateWidgets(m_AlwaysShownLayout, menuEditor.GetMenuComponent().GetAlwaysShownWidget());
		FrameSlot.SetAnchorMin(m_AlwaysShownWidget, 0, 0);
		FrameSlot.SetAnchorMax(m_AlwaysShownWidget, 1, 1);
		FrameSlot.SetOffsets(m_AlwaysShownWidget, 0, 0, 0, 0);
		
		m_HideableWidget = workspace.CreateWidgets(m_HideableLayout, menuEditor.GetMenuComponent().GetHideableWidget());
		FrameSlot.SetAnchorMin(m_HideableWidget, 0, 0);
		FrameSlot.SetAnchorMax(m_HideableWidget, 1, 1);
		FrameSlot.SetOffsets(m_HideableWidget, 0, 0, 0, 0);
		
		//--- Get cursor component
		m_CursorComponent = SCR_CursorEditorUIComponent.Cast(menu.GetRootComponent().FindComponent(SCR_CursorEditorUIComponent));
		
		//--- Fade in
		//m_Widget.SetOpacity(0);
		//AnimateWidget.Opacity(m_Widget, 1, 5);
	}
	
	override void EOnEditorPostDeactivate()
	{
		if (m_AlwaysShownWidget)
		{
			m_AlwaysShownWidget.RemoveFromHierarchy();
			m_AlwaysShownWidget = null;
		}
		if (m_HideableWidget)
		{
			m_HideableWidget.RemoveFromHierarchy();
			m_HideableWidget = null;
		}
	}
};