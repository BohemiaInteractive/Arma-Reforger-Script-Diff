class SCR_WidgetTools
{
	/*!
	Search for widget of specific type in itslef and all its children, recursively.
	\param w Queried widget
	\param type Type of desired widget
	\return Widget, or null if not found
	*/
	static Widget FindWidgetInChildren(Widget w, WidgetType type)
	{
		if (w.GetTypeID() == type)
			return w;
		
		Widget candidate;
		w = w.GetChildren();
		while (w)
		{
			candidate = FindWidgetInChildren(w, type);
			if (candidate)
				return candidate;
			
			w = w.GetSibling();
		}
		return null;
	}
	/*!
	Search for widget with UI component in widget and all its children, recursively.
	\param w Queried widget
	\param type Type of desired UI component
	\return Widget, or null if not found
	*/
	static Widget FindWidgetInChildren(Widget w, typename type)
	{
		if (w.FindHandler(type))
			return w;
		
		Widget result;
		w = w.GetChildren();
		while (w)
		{
			result = FindWidgetInChildren(w, type);
			if (result)
				return result;
			
			w = w.GetSibling();
		}
		return null;
	}
	/*!
	Search for widget of specific type in itself and all its parents.
	\param w Queried widget
	\param type Type of desired widget
	\return Widget, or null of not found
	*/
	static Widget FindWidgetInParents(Widget w, WidgetType type)
	{
		while (w)
		{
			if (w.GetTypeID() == type)
				return w;
			
			w = w.GetParent();	
		}
		return null;
	}
	
	/*!
	Search for UI component in widget and all its children, recursively.
	\param w Queried widget
	\param type Type of desired UI component
	\return UI component, or null of not found
	*/
	static ScriptedWidgetEventHandler FindHandlerInChildren(Widget w, typename type)
	{
		ScriptedWidgetEventHandler component = w.FindHandler(type);
		if (component)
			return component;
		
		w = w.GetChildren();
		while (w)
		{
			component = FindHandlerInChildren(w, type);
			if (component)
				return component;
			
			w = w.GetSibling();
		}
		return null;
	}
	/*!
	Search for UI component in widget and all its parents.
	\param w Queried widget
	\param type Type of desired UI component
	\return UI component, or null of not found
	*/
	static ScriptedWidgetEventHandler FindHandlerInParents(Widget w, typename type)
	{
		ScriptedWidgetEventHandler component;
		while (w)
		{
			component = w.FindHandler(type);
			if (component)
				return component;
			
			w = w.GetParent();	
		}
		return null;
	}
	/*!
	Check if a widget is in another widget's hierarchy.
	\param w Queried widget
	\param parent Queried parent
	\return True if Widget 'w' is 'parent' or one if its children, recursively.
	*/
	static bool InHierarchy(Widget w, Widget parent)
	{
		while (w)
		{
			if (w == parent)
				return true;
			
			w = w.GetParent();	
		}
		return false;
	}
	/*!
	Check if a widget is in another widget's hierarchy. If we reach the parent after performing more than 0 steps then the Widget is a child
	\param w Queried widget
	\param parent Queried parent
	\return True if Widget 'w' is one of Widget 'parent' children, recursively.
	*/
	static bool IsChildOf(Widget w, notnull Widget parent, out int depth = 0)
	{
		int nStepsUp;
		while (w)
		{
			if (w == parent && nStepsUp > 0)
			{
				depth = nStepsUp;
				return true;
			}
			
			w = w.GetParent();
			nStepsUp++;
		}
		return false;
	}
	/*!
	Remove all child widgets from hierarchy.
	\param w Parent widget
	*/
	static void RemoveChildrenFromHierarchy(Widget w)
	{
		Widget child = w.GetChildren();
		while (child)
		{
			child.RemoveFromHierarchy();
			child = child.GetSibling();
		}
	}
	/*!
	Find menu to which the widget belongs to.
	Must not be called for ScriptedWidgetComponent.HandlerAttached(), it's too early.
	\param w Queried widget
	\return Menu, or null if the widget is not part of any menu
	*/
	static MenuBase FindMenu(Widget w)
	{
		if (!w)
			return null;
		
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return null;
		
		if (!InHierarchy(w, GetGame().GetWorkspace()))
		{
			Debug.Error2("SCR_WidgetTools.GetMenu()", string.Format("Cannot find menu of widget '%1', it's not initialized yet. Are you perhaps calling it too early, e.g., from HandlerAttached()?", w.GetName()));
			return null;
		}
		
		MenuBase menuCandidate;
		array<ScriptMenuPresetEnum> values = {};
		for (int i, count = SCR_Enum.GetEnumValues(ChimeraMenuPreset, values); i < count; i++)
		{
			menuCandidate = menuManager.FindMenuByPreset(values[i]);
			if (menuCandidate && InHierarchy(w, menuCandidate.GetRootWidget()))
				return menuCandidate;
		}
		return null;
	}
	
	/*!
	Get full hierarchy path towards a widget.
	\param w Widget
	\param delimeter String added between widget names
	\return Path
	*/
	static string GetHierarchyLog(Widget w, string delimiter = " / ")
	{
		string log;
		while (w)
		{
			if (log.IsEmpty())
				log = w.GetName();
			else
				log = w.GetName() + delimiter + log;
			
			w = w.GetParent();
		}
		return log;
	}
	
	/*!
	Find widget handler in widget by name inside given widget 
	\param root Widget
	\param widgetName string
	\param type typename 
	\return ScriptedWidgetEventHandler handler 
	*/
	static ScriptedWidgetEventHandler FindHandlerOnWidget(Widget root, string widgetName, typename type)
	{
		if (!root)
			return null;
		
		Widget w = root.FindAnyWidget(widgetName);
		if (!w)
			return null;
		
		return w.FindHandler(type);
	}
};