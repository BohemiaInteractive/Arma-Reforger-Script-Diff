class SCR_WidgetHelper
{
	//------------------------------------------------------------------------------------------------
	//! Return a widget's children. The search is not recursive by default.
	//! \param[in] widget
	//! \param[in] widgetArray this array is NOT cleared, only appended
	//! \param[in] recursive set to true to obtain sub-children as well
	static void GetAllChildren(notnull Widget widget, notnull out array<ref Widget> widgetArray, bool recursive = false)
	{
		Widget child = widget.GetChildren();
		while (child)
		{
			widgetArray.Insert(child);
			if (recursive)
				GetAllChildren(child, widgetArray, true);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all child widgets from the provided widget
	//! \param[in] widget
	static void RemoveAllChildren(notnull Widget widget)
	{
		while (widget.GetChildren())
		{
			widget.GetChildren().RemoveFromHierarchy();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] widget
	//! \param[in] imageIndex
	static void ResizeToImage(notnull ImageWidget widget, int imageIndex = 0)
	{
		int x, y;
		widget.GetImageSize(imageIndex, x, y);
		widget.SetSize(x, y);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the root widget (before the WorkspaceWidget)
	//! \param[in] Widget
	//! \return the "topmost" parent (in hierarchy notion, not in z-index notion)
	static Widget GetRootWidget(notnull Widget widget)
	{
		WorkspaceWidget topTopParent = widget.GetWorkspace();
		Widget parent;
		while (widget && widget != topTopParent)
		{
			parent = widget;
			widget = parent.GetParent();
		}
		return parent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get widget or one of its children that has the provided (case-sensitive) name
	//! \param[in] widget
	//! \param[in] widgetName case-sensitive
	//! \return the provided widget (if the name matches) or one of its children (if the name matches) or null if not found
	static Widget GetWidgetOrChild(notnull Widget widget, string widgetName)
	{
		if (widget.GetName() == widgetName)
			return widget;

		return widget.FindAnyWidget(widgetName);
	}
}

//class SCR_WidgetHelperT<Class T>
//{
//	static T GetScriptedComponent(notnull Widget widget)
//	{
//		return T.Cast(widget.FindHandler(T));
//	}
//}
