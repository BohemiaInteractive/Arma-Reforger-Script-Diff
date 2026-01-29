[ComponentEditorProps(category: "GameScripted/Editor", description: "GUI for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_MenuEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
class SCR_MenuEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(desc: "Menu created when the editor is opened.", defvalue: "-1", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	private ChimeraMenuPreset m_EditorMenuPreset;
	
	[Attribute(desc: "Visibility status when the editor is opened.", defvalue: "1")]
	private bool m_bVisible;
	
	[Attribute(desc: "Is visibility of the menu preserved whent editor is closed and opened again?", defvalue: "1")]
	private bool m_bVisiblePersistent;
	
	private bool m_bVisibleDefault;
	private EditorMenuBase m_EditorMenu;
	protected SCR_MenuEditorUIComponent m_EditorMenuComponent;
	private InputManager m_InputManager;
	
	private ref ScriptInvoker Event_OnVisibilityChange = new ScriptInvoker;
	
	/*!
	Set visibility of elements which can be hidden.
	\param visible True when visible
	\param instant True to set visibility instantly, false to animate it
	*/
	void SetVisible(bool visible, bool instant = false)
	{
		if (visible == m_bVisible) return;
		m_bVisible = visible;
		
		SCR_HideEditorUIComponent hideComponent = SCR_HideEditorUIComponent.Cast(m_EditorMenuComponent.GetHideableWidget().FindHandler(SCR_HideEditorUIComponent));
		if (hideComponent)
			hideComponent.SetVisible(m_bVisible, instant);
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (hudManager)
			hudManager.SetVisible(m_bVisible);
		
		Event_OnVisibilityChange.Invoke(m_bVisible);
	}
	/*!
	Toggle visibility of elements which can be hidden.
	*/
	void ToggleVisible()
	{
		SetVisible(!m_bVisible);
	}
	/*!
	Check if elements which can be hidden are visible.
	\return True when visible
	*/
	bool IsVisible()
	{
		return m_bVisible;
	}
	/*!
	Get associtaed menu.
	\return Menu
	*/
	EditorMenuBase GetMenu()
	{
		return m_EditorMenu;
	}
	/*!
	Get component with menu configuration.
	\return Menu config component
	*/
	SCR_MenuEditorUIComponent GetMenuComponent()
	{
		return m_EditorMenuComponent;
	}
	/*!
	Get ebent called when editor menu visibility changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnVisibilityChange()
	{
		return Event_OnVisibilityChange;
	}
	
	protected void OnModeChange()
	{
		SetVisible(true, true);
	}
	
	//--- EOnAfterEditorOpen() is used instead of EOnEditorOpen() to make sure GUI follows all other components
	override protected void EOnEditorPostActivate()
	{
		//--- Reset visibility when presistence is disabled
		if (!m_bVisiblePersistent) m_bVisible = m_bVisibleDefault;
		
		//--- Use the concept menu or the final one?
		ChimeraMenuPreset menuPreset = m_EditorMenuPreset;
		
		if (menuPreset < 0) menuPreset = ChimeraMenuPreset.EditorMenu;
		MenuBase baseMenu = GetGame().GetMenuManager().OpenMenu(menuPreset);
		m_EditorMenu = EditorMenuBase.Cast(baseMenu);
		if (!m_EditorMenu)
		{
			GetGame().GetMenuManager().CloseMenu(baseMenu);
			Print("Editor menu is not type EditorMenuBase!", LogLevel.ERROR);
		}
		
		m_EditorMenuComponent = SCR_MenuEditorUIComponent.Cast(m_EditorMenu.GetRootWidget().FindHandler(SCR_MenuEditorUIComponent));
		if (!m_EditorMenu)
		{
			GetGame().GetMenuManager().CloseMenu(baseMenu);
			Print("Editor menu root widget is missing SCR_MenuEditorUIComponent!", LogLevel.ERROR);
		}
		
		//--- Reveal GUI when switching modes
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.Cast(GetOwner());
		if (editorManager)
			editorManager.GetOnModeChange().Insert(OnModeChange);
	}
	override protected void EOnEditorDeactivate()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.Cast(GetOwner());
		if (editorManager)
			editorManager.GetOnModeChange().Remove(OnModeChange);
	}
	override protected void EOnEditorClose()
	{
		if (m_EditorMenu) GetGame().GetMenuManager().CloseMenu(m_EditorMenu);
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (hudManager) hudManager.SetVisible(true);
	}
	override protected void EOnEditorActivate()
	{
		m_InputManager = GetGame().GetInputManager();
	}
	override protected void EOnEditorInit()
	{
		m_bVisibleDefault = m_bVisible;
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_EDITOR_GUI, "Editor GUI", "Editor");
	}
};