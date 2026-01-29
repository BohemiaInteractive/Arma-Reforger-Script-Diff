//! Final Reforger button. Supported states: Disaled, hovered, focused, clicked - each with own visualization
//! Works with WLib_Button widget prefab

//------------------------------------------------------------------------------------------------
class SCR_ButtonSplitComponent : SCR_ButtonComponent 
{
	protected ref array<ref Widget> m_aWidgetCells = new array<ref Widget>();
	protected ref array<bool> m_aPreventContentChange = new array<bool>; 
	
	protected ref array<SCR_ButtonComponent> m_aInnerButtons = new array<SCR_ButtonComponent>; 
	
	protected Color m_cColorContent;
	
	protected ref Widget m_wHoverWidget;
	
	[Attribute(defvalue: "0")]
	protected int m_iButtonPaddingLeft;
	[Attribute(defvalue: "0")]
	protected int m_iButtonPaddingTop;
	[Attribute(defvalue: "0")]
	protected int m_iButtonPaddingRigh;
	[Attribute(defvalue: "0")]
	protected int m_iButtonPaddingBottom;
	
	protected bool m_bColorizeFocusLost = true;
	
	//------------------------------------------------------------------------------------------------
	void SetColorizeFocusLost(bool colorize) { m_bColorizeFocusLost = colorize; }
	
	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		HorizontalLayoutSlot.SetPadding(m_wOverlay, m_iButtonPaddingLeft, m_iButtonPaddingTop, m_iButtonPaddingRigh, m_iButtonPaddingBottom);
		m_cColorContent = COLOR_CONTENT_DEFAULT;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (!m_bColorizeFocusLost)
			return false;
		
		super.OnMouseEnter(w, x, y);
		m_wHoverWidget = w;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		if (!m_bColorizeFocusLost)
		{	
			m_OnHoverLeave.Invoke();
			return false;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ColorizeWidgets(Color colorBackground, Color colorContent, float speed = -1)
	{
		if (speed < 0)
			speed = m_fAnimationRate;
		
		m_cColorContent = colorContent;
		ColorizeCellPart("Background", colorBackground, speed, false);
		ColorizeCellPart("Content", m_cColorContent, speed, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void InsertWidgetCells(array<string> cellNames)
	{
		Widget w;
		foreach (string cellName : cellNames)
		{
			w = m_wRoot.FindAnyWidget(cellName);
			if (w)
			{
				m_aWidgetCells.Insert(w);
				m_aPreventContentChange.Insert(false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------ 
	//! Find and setup focusing on root for every inner widget 
	void SetupInnerButton(string widgetName)
	{
		Widget w = m_wRoot.FindAnyWidget(widgetName);
		if(!w)
			return;  
		
		// Has this cell a button?
		SCR_ButtonComponent button = SCR_ButtonComponent.Cast(w.FindHandler(SCR_ButtonComponent));
		if(!button)
			return;  
		
		// Insert on click to focus root 
		button.m_OnFocus.Insert(OnInnerButtonFocus);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Focus button root 
	protected void OnInnerButtonFocus(Widget w)
	{
		if(!m_wHoverWidget)
			return;		
		GetGame().GetWorkspace().SetFocusedWidget(m_wHoverWidget);
		
		if(!m_wRoot)
			return;
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set color of every cell
	protected void ColorizeCellPart(string partName, Color c, float speed, float prevent)
	{		
		for (int i = 0; i < m_aWidgetCells.Count(); i++)
		{
			if (i > m_aPreventContentChange.Count())
				break;
			
			if (prevent && m_aPreventContentChange[i])
			{
				continue;
			}
			
			Widget w = m_aWidgetCells[i];
			Widget wPart = null;
			string name = w.GetName();
			if (!w)
				continue;
			
			wPart = w.FindAnyWidget(partName);
			
			if (!wPart)
				wPart = w.FindAnyWidget("Overlay");
			
			if (wPart)
				AnimateWidget.Color(wPart, c, speed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set color of one specific cell
	void ColorizeCellPart(string partName, Color c, float speed, float prevent, int id)
	{		
		if (prevent)
			return;
		
		Widget w = m_aWidgetCells[id];
		Widget wPart = null;
		
		if (w)
			wPart = w.FindAnyWidget(partName);
		
		if (!wPart)
			wPart = w.FindAnyWidget("Overlay");
		
		if (wPart)
			AnimateWidget.Color(wPart, c, speed);
	}
	
	//------------------------------------------------------------------------------------------------
	void PreventContentColoringOn(int iCellId, bool bPrevent)
	{
		m_aPreventContentChange[iCellId] = bPrevent;
	}
	
	//------------------------------------------------------------------------------------------------
	Color GetColorContent()
	{
		return Color.FromInt(m_cColorContent.PackToInt());
	}
};