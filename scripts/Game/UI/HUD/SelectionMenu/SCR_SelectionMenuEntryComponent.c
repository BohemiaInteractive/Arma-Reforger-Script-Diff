/*!
Base widget component for selection menu entries 
Handles mainly mouse interaction with the entry layout
*/
//------------------------------------------------------------------------------------------------
class SCR_SelectionMenuEntryComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("0.4")]
	protected float m_fDisabledOpacity;
	
	[Attribute("Size")]
	protected string m_sSizeLayout;
	
	[Attribute("EntryText")]
	protected string m_sEntryText;
	
	protected SCR_SelectionMenuEntry m_Entry;
	
	protected bool m_bEnabled = true;
	protected float m_fOriginalSize;
	protected float m_fAdjustedSize;
	
	protected Widget m_wSizeLayout;
	protected TextWidget m_wEntryText;
	
	// Invokers 
	protected ref ScriptInvoker<SCR_SelectionMenuEntryComponent> m_OnMouseEnter;
	protected ref ScriptInvoker<SCR_SelectionMenuEntryComponent> m_OnMouseLeave;
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnMouseEnter()
	{
		if (m_OnMouseEnter)
			m_OnMouseEnter.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseEnter()
	{
		if (!m_OnMouseEnter)
			m_OnMouseEnter = new ScriptInvoker();

		return m_OnMouseEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnMouseLeave()
	{
		if (m_OnMouseLeave)
			m_OnMouseLeave.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseLeave()
	{
		if (!m_OnMouseLeave)
			m_OnMouseLeave = new ScriptInvoker();

		return m_OnMouseLeave;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnClick()
	{
		if (m_OnClick)
			m_OnClick.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClick()
	{
		if (!m_OnClick)
			m_OnClick = new ScriptInvoker();

		return m_OnClick;
	}
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		SetEnabled(m_bEnabled);
		
		m_wSizeLayout = m_wRoot.FindAnyWidget(m_sSizeLayout);
		m_wEntryText = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sEntryText));
		m_fOriginalSize = GetLayoutSize();
		m_fAdjustedSize = m_fOriginalSize;
		
		// Set anchor to allow for size/pos override
		if (m_wSizeLayout)
			FrameSlot.SetAnchor(m_wSizeLayout, 0.5, 0.5);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		InvokeOnMouseEnter();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		InvokeOnMouseLeave();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == 0)
			InvokeOnClick();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{	
		m_bEnabled = enabled;
		
		if (!m_wRoot)
			return;
		
		// Set visuals 
		if (m_bEnabled)
			m_wRoot.SetOpacity(1);	
		else
			m_wRoot.SetOpacity(m_fDisabledOpacity);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLayoutSize()
	{
		if (!m_wSizeLayout)
			return -1;
		
		return FrameSlot.GetSize(m_wSizeLayout)[0];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry holding data driving this visuals
	void SetEntry(SCR_SelectionMenuEntry entry)
	{
		m_Entry = entry;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryText(string str)
	{
		if (!m_wEntryText)
		{
			Print("Entry text widget was not found!", LogLevel.ERROR);
			return; 
		}
		
		m_wEntryText.SetVisible(!str.IsEmpty());
		m_wEntryText.SetText(str);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetOriginalSize()
	{
		return m_fOriginalSize;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAdjustedSize()
	{
		return m_fAdjustedSize;
	}
}