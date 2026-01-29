[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Layout", true, "GUI: \"%1\"")]
/** @ingroup Editor_Effects
*/
/*!
Editor UI effect.
*/
class SCR_UIEditorEffect: SCR_BaseEditorEffect
{
	[Attribute(params: "layout", desc: "GUI layout file")]
	private ResourceName m_Layout;
	
	protected Widget m_Widget;
	
	override bool EOnActivate(SCR_BaseEditorComponent editorComponent, vector position = vector.Zero, set<SCR_EditableEntityComponent> entities = null)
	{
		if (editorComponent.IsActive())
		{
			SCR_MenuEditorComponent menuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
			if (!menuManager) return false;
			
			EditorMenuBase menu = menuManager.GetMenu();
			if (!menu) return false;
		
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (!workspace) return false;
			
			Widget parent = menuManager.GetMenuComponent().GetHideableWidget();
			if (!parent) return false;
			
			if (m_Widget) m_Widget.RemoveFromHierarchy();
			m_Widget = workspace.CreateWidgets(m_Layout, parent);
		}
		else
		{
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
			if (!hudManager) return false;
			
			if (m_Widget) m_Widget.RemoveFromHierarchy();
			m_Widget = hudManager.CreateLayout(m_Layout, EHudLayers.LOW);
		}
		
		//--- ToDo: Remove
		AnimateWidget.Opacity(m_Widget, 0, 0.5);
		
		return true;
	}
};