/*
How to use:

Variant 1:
  Put a LoadingOverlay.layout into your layout manually, adjust text, size, hide background and blur if needed.

Variant 2: 
  Call SCR_LoadingOverlay.ShowForWidget() to render the overlay over a specific widget. The target widget can be of any kind.
  Call HideForWidget() when you want to hide and delete the overlay.
  Call public methods to show/hide temporary, or adjust text, blur, background.
  When target widget is destroyed, the loading overlay of it is also destroyed automatically.
*/


//! This component is attached to the target widget and holds a reference to the overlay targeted at this widget
class SCR_LoadingOverlayHelperComponent : ScriptedWidgetComponent
{	
	SCR_LoadingOverlay m_OverlayComponent;
	
	Widget m_wTarget; // This component is attached to this widget
	
	//------------------------------------------------------------------------------------------------
	void Init(Widget targetWidget, SCR_LoadingOverlay overlayComponent)
	{
		m_wTarget = targetWidget;
		m_OverlayComponent = overlayComponent;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!GetGame().GetWorkspace())
			return;
		
		// When the target widget is deleted, we also delete the overlay
		if (m_OverlayComponent)
		{
			if (m_OverlayComponent.m_wRoot)
				GetGame().GetWorkspace().RemoveChild(m_OverlayComponent.m_wRoot);
		}
	}
};



//------------------------------------------------------------------------------------------------
class SCR_LoadingOverlay : ScriptedWidgetComponent
{	
	Widget m_wRoot;
	
	ref SCR_LoadingOverlayWidgets widgets;	
	
	SCR_LoadingOverlayHelperComponent m_Helper;
	
	protected bool m_bRenderTopMost = false;
	
	protected bool m_bShown = true; // At start it is shown by default
	
	// -------- Attributes for the component when placed through editor ---------
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "The overlay is shown at start")]
	bool m_bShownAtStart;
	
	[Attribute("true", uiwidget: UIWidgets.CheckBox, desc: "Should background image be visible?")]
	protected bool m_bShowBackground;
	
	//------------------------------------------------------------------------------------------------
	//! Shows an overlay over target widget
	//! You can call it multiple times for same widget. If an overlay already exists, new one will not be created, and the old overlay will be returned.
	static SCR_LoadingOverlay ShowForWidget(Widget targetWidget, string text = string.Empty, bool showBlur = true, bool showBackground = true)
	{
		if (!targetWidget)
			return null;
		
		SCR_LoadingOverlayHelperComponent helper;
		SCR_LoadingOverlay comp;
		Widget overlayRoot;

		// First check if an overlay is already present for this widget
		helper = SCR_LoadingOverlayHelperComponent.Cast(targetWidget.FindHandler(SCR_LoadingOverlayHelperComponent));
		
		if (!helper)
		{
			// Add helper component to target widget
			helper = new SCR_LoadingOverlayHelperComponent;
			targetWidget.AddHandler(helper);
		}
		else
		{
			comp = helper.m_OverlayComponent;
			overlayRoot = comp.m_wRoot;
		}
		
		// Create the overlay component if it's not found
		if (!comp)
		{
			overlayRoot = GetGame().GetWorkspace().CreateWidgets(SCR_LoadingOverlayWidgets.s_sLayout, GetGame().GetWorkspace());
			comp = SCR_LoadingOverlay.Cast(overlayRoot.FindHandler(SCR_LoadingOverlay));
		}
		
		
		if (!overlayRoot || !comp || !helper)
			return null;
		
		helper.Init(targetWidget, comp);
		comp.Init(overlayRoot, helper);
		
		comp.SetText(text);
		comp.ShowBlur(showBlur);
		comp.ShowBackground(showBackground);
		comp.SetShown(true);
		
		return comp;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Hides overlay for given target widget
	static void HideForWidget(Widget targetWidget)
	{
		SCR_LoadingOverlayHelperComponent helper = SCR_LoadingOverlayHelperComponent.Cast(targetWidget.FindHandler(SCR_LoadingOverlayHelperComponent));
		
		// If no helper component found at the target widget, we can't do anything
		if (!helper)
			return;
		
		SCR_LoadingOverlay comp = helper.m_OverlayComponent;
		
		comp.HideAndDelete();
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Hides the overlay. The overlay will be destroyed, and the SCR_LoadingOverlay component will be deleted.
	void HideAndDelete()
	{
		// Remove the helper component from target
		if (m_Helper)
		{
			if (m_Helper.m_wTarget)
			{
				m_Helper.m_wTarget.RemoveHandler(m_Helper);
			}
		}
		
		// Remove the overlay from workspace
		if (m_wRoot)
		{
			// This component is going to be deleted now as well
			GetGame().GetWorkspace().RemoveChild(m_wRoot);
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Shows or hides the overlay. The overlay is not destroyed if hidden and can be shown again.
	void SetShown(bool show)
	{
		// Avoid calling Show many times
		if (m_bShown == show)
			return;
		
		m_bShown = show;
		
		m_wRoot.SetVisible(show);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		this.widgets.m_Text.SetText(text);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void ShowBackground(bool show)
	{
		this.widgets.m_BackgroundImage.SetVisible(show);
	}

	
	
	//------------------------------------------------------------------------------------------------
	void ShowBlur(bool show)
	{
		this.widgets.m_Blur.SetVisible(show);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Sets the fake button as focused widget, can be used if you need to reset focus from the
	//! widgets under this loading overlay
	void SetFocus()
	{
		//GetGame().GetWorkspace().SetFocusedWidget(null);
		GetGame().GetWorkspace().AddModal(m_wRoot, m_wRoot);
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot); // Our root widget is a button
	}
	
	
	// ----------- Private methods below -------------
	
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// If attached to workspace root, this will render over everything
		// Also in this case the ShowForWidget method calls the Init
		if (w.GetParent() == GetGame().GetWorkspace())
		{
			m_bRenderTopMost = true;
		}
		else
		{
			// If not attached to workspace root, this will act as if placed in a reagular widget hierarchy
			this.Init(w, null);
			
			this.SetShown(m_bShownAtStart);
			ShowBackground(m_bShowBackground);
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void Init(Widget overlayRoot, SCR_LoadingOverlayHelperComponent helper)
	{
		m_wRoot = overlayRoot;
		m_Helper = helper;
	
		widgets = new SCR_LoadingOverlayWidgets;
		widgets.Init(m_wRoot);
		
		// If this is supposed to be rendered over everything,
		// set Z order to some high value
		if (helper && m_bRenderTopMost)
		{
			int zOrder = 9001; // todo redo Z values with enum
			if (helper.m_wTarget == GetGame().GetWorkspace())
				zOrder += 1000;
			m_wRoot.SetZOrder(zOrder);
			
			GetGame().GetCallqueue().CallLater(SetSizeFromTarget, 0);
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void SetSizeFromTarget()
	{
		if (!m_Helper)
			return;
		
		if (!m_Helper.m_wTarget)
			return;
		
		Widget targetWidget = m_Helper.m_wTarget;
		
		float x, y, sizex, sizey;
		targetWidget.GetScreenPos(x, y);
		targetWidget.GetScreenSize(sizex, sizey);
		
		WorkspaceWidget ww = GetGame().GetWorkspace();
		
		x = ww.DPIUnscale(x);
		y = ww.DPIUnscale(y);
		sizex = ww.DPIUnscale(sizex);
		sizey = ww.DPIUnscale(sizey);
		
		FrameSlot.SetPos(m_wRoot, x, y);
		FrameSlot.SetSize(m_wRoot, sizex, sizey);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		// This is only needed if we put the overlay to Workspace root and want to track the size and pos
		// of the targer widget
		if (!m_bRenderTopMost)
			return false;
		
		if (!m_wRoot || !m_Helper || !m_Helper.m_wTarget)
		{
			return false;
		}
		
		GetGame().GetCallqueue().CallLater(SetSizeFromTarget, 0);
		
		return true;
	}
	
	
	
	
	// Overriding events so that we can intercept them
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return true;
	}
	override bool OnModalClosed(Widget modalRoot) {return true;}
	override bool OnDoubleClick(Widget w, int x, int y, int button) {return true;}
	override bool OnItemSelected(Widget w, int row, int column, int oldRow, int oldColumn) {return true;}
	override bool OnFocus(Widget w, int x, int y)	{ return true; }
	override bool OnFocusLost(Widget w, int x, int y) {	return true; }
	override bool OnMouseEnter(Widget w, int x, int y) {return true;}
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y) {return true;}
	override bool OnMouseWheel(Widget w, int x, int y, int wheel) {return true;}
	override bool OnMouseButtonDown(Widget w, int x, int y, int button) {return true;}
	override bool OnMouseButtonUp(Widget w, int x, int y, int button) {return true;}
	//! control is one of ControlID
	override bool OnController(Widget w, int control, int value) {return true;}
	override bool OnChar(Widget w, int charCode) {return true;}
	override bool OnChange(Widget w, bool finished) {return true;}
	override bool OnModalClickOut(Widget modalRoot, int x, int y, int button) {return true;}

};