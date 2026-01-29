class SCR_WidgetHighlightUIComponent: ScriptedWidgetComponent
{
	protected Widget m_HighlightWidget;
	
	/*!
	Highlight a widget.
	\param widget Widget to be highlighted
	\param layout Layout prefab of the highlight
	\param parent Parent widget of the highlight. Must be a FrameSlot covering the whole screen!
	\return Highlight widget. Delete it when highlight is no longer needed.
	*/
	static Widget CreateHighlight(Widget widget, ResourceName layout, Widget parent = null)
	{
		if (!widget)
		{
			Print("Cannot create highlight, widget not found!", LogLevel.WARNING);
			return null;
		}
		
		/*
		Create the highlight in Workspace, not where target widget is.
		It's because we don't know which layout slot is used, and not all of them would be compatible (e.g., horizontal slot).
		Furthermore, highlight widget can be an outline *outside* of the area, and modifying target's clipping could be dangerous.
		*/
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!parent)
			parent = workspace;
		SCR_WidgetHighlightUIComponent highlight = new SCR_WidgetHighlightUIComponent();
		highlight.m_HighlightWidget = workspace.CreateWidgets(layout, parent);
		widget.AddHandler(highlight);
		return highlight.m_HighlightWidget;
	}
	/*!
	Highlight a widget.
	\param widgetName Name of the widget to be highlighted
	\param layout Layout prefab of the highlight
	\param parent Parent widget of the highlight. Must be a FrameSlot covering the whole screen!
	\return Highlight widget. Delete it when highlight is no longer needed.
	*/
	static Widget CreateHighlight(string widgetName, ResourceName layout, Widget parent = null)
	{
		Widget widget = GetGame().GetWorkspace().FindAnyWidget(widgetName);
		if (!widget)
		{
			Print(string.Format("Cannot create highlight, widget '%1' not found!", widgetName), LogLevel.WARNING);
			return null;
		}
		return CreateHighlight(widget, layout, parent);
	}
	
	protected void Update(Widget w)
	{
		//--- Exit if the parent was meanwhile deleted (e.g., when the menu is closed)
		if (!m_HighlightWidget || !w.GetParent())
		{
			HandlerDeattached(w);
			return;
		}
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		//--- Set position
		float posX, posY, posW, posH;
		w.GetScreenPos(posX, posY);
		w.GetScreenSize(posW, posH);
		FrameSlot.SetPos(m_HighlightWidget, workspace.DPIUnscale(posX), workspace.DPIUnscale(posY));
		FrameSlot.SetSize(m_HighlightWidget, workspace.DPIUnscale(posW), workspace.DPIUnscale(posH));
		
		//--- Set visibility
		bool isVisible = true;
		while (w)
		{
			if (!w.IsVisible() || w.GetOpacity() == 0)
			{
				isVisible = false;
				break;
			}
			w = w.GetParent();
		}
		m_HighlightWidget.SetVisible(isVisible);
	}
	override void HandlerAttached(Widget w)
	{
		Update(w);
		if (GetGame().GetCallqueue())
			GetGame().GetCallqueue().CallLater(Update, 1, true, w);
	}
	override void HandlerDeattached(Widget w)
	{
		if (GetGame())
		{
			if (GetGame().GetCallqueue())
				GetGame().GetCallqueue().Remove(Update);
		}
		
		if (m_HighlightWidget)
			m_HighlightWidget.RemoveFromHierarchy();
	}
};