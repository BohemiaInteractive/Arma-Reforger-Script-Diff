/*
This component must be attached to a size widget inside an overlay.
It will try to rescale the size layout to fit it inside the parent widget while keeping the specified aspect ratio.
*/

class SCR_ForceAspectRatioComponent : ScriptedWidgetComponent
{
	[Attribute("1.777", UIWidgets.EditBox)]
	protected float m_fRatio;
	
	protected SizeLayoutWidget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = SizeLayoutWidget.Cast(w);
		
		if (!m_wRoot)
			Print("SCR_ForceAspectRatioComponent must be attached to size widget!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		// This will also get called on updates of children, ignore them
		if (w != m_wRoot || !GetGame().InPlayMode())
			return true;
		
		GetGame().GetCallqueue().CallLater(UpdateSize, 0);	// This must not be performed during OnUpdate
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSize()
	{
		if (!m_wRoot)
			return;
		
		Widget wParent = m_wRoot.GetParent();
		
		// Bail if it's called after all widgets are destroyed.
		if (!m_wRoot)
			return;
		
		// Init size layout rules
		m_wRoot.EnableHeightOverride(true);
		m_wRoot.EnableWidthOverride(true);
		AlignableSlot.SetVerticalAlign(m_wRoot, LayoutVerticalAlign.Center);
		AlignableSlot.SetHorizontalAlign(m_wRoot, LayoutHorizontalAlign.Center);
		
		// Get size of parent area
		float parentWidth, parentHeight;
		wParent.GetScreenSize(parentWidth, parentHeight);
		
		// Ensure width and height are not zero to prevent division by zero.
		if (parentHeight == 0)
			parentHeight = 1;
		if (parentWidth == 0)
			parentWidth = 1;
		
		// Check ratio of parent widget
		float parentRatio = parentWidth / parentHeight;
		float width, height;
		if (parentRatio > m_fRatio)
		{
			width = parentHeight * m_fRatio;
			height = parentHeight;
		}
		else
		{
			width = parentWidth;
			height = parentWidth / m_fRatio;
		}
		
		auto workspace = GetGame().GetWorkspace();
		float widthUnscaled = workspace.DPIUnscale(width);
		float heightUnscaled = workspace.DPIUnscale(height);
		m_wRoot.SetWidthOverride(widthUnscaled);
		m_wRoot.SetHeightOverride(heightUnscaled);
	}
};