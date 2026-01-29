/** @ingroup Editor_UI Editor_UI_Menus
*/

/*!
Base class for editor menus and dialogs.
*/
class EditorMenuBase: MenuRootBase
{	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Actions
	
	/*!
	Open a dialog of given preset.
	\param preset
	*/
	MenuBase OpenDialog(ChimeraMenuPreset preset)
	{
		if (GetManager().FindMenuByPreset(preset)) return null; //--- Already opened
		return GetManager().OpenDialog(preset);
	}
	
	/*!
	Open a menu of given preset.
	\param preset
	*/
	MenuBase OpenMenu(ChimeraMenuPreset preset)
	{
		if (GetManager().FindMenuByPreset(preset)) return null; //--- Already opened
		return GetManager().OpenMenu(preset);
	}
	
	/*!
	Close this menu.
	*/
	void CloseSelf()
	{
		GetManager().CloseMenu(this);
	}
	
	/*!
	Delete all child widgets.
	\param parentWidget Parent Widget
	*/
	protected void DeleteAllChildWidgets(Widget parentWidget)
	{
		while (parentWidget.GetChildren())
		{
			delete parentWidget.GetChildren();
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Getters
	
	/*!
	Find the widget of specific name.
	\param widgetName Name of the widget
	*/
	Widget GetWidgetByName(string widgetName)
	{
		Widget root = GetRootWidget();
		if (!root) return null;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return null;
		
		return root.FindAnyWidget(widgetName);
	}
};