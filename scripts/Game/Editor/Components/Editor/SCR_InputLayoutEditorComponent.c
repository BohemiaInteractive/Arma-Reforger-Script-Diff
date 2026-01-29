[ComponentEditorProps(category: "GameScripted/Editor", description: "Access to the editor by pressin input actions. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_InputLayoutEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components

Manage GUI for visualizing user input for accessing the editor.
*/
class SCR_InputLayoutEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(params: "layout")]
	protected ResourceName m_Layout;
	
	protected Widget m_Widget;
	
	override void EOnEditorInit()
	{
		m_Widget = GetGame().GetWorkspace().CreateWidgets(m_Layout);
		if (m_Widget && !m_Widget.FindHandler(SCR_EditorToggleUIComponent))
		{
			Print(string.Format("Widget '%1' does not have SCR_EditorToggleUIComponent!", m_Layout.GetPath()), LogLevel.WARNING);
			m_Widget.RemoveFromHierarchy();
		}
	}
	override void EOnEditorDelete()
	{
		if (m_Widget) m_Widget.RemoveFromHierarchy();
	}
};