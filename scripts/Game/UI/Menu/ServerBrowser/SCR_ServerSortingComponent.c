//------------------------------------------------------------------------------------------------
class SCR_ServerSortingComponent : SCR_SelectionWidgetComponent
{
	[Attribute()]
	ref array<string> m_aElementsFilters;
	
	protected SCR_SelectableButtonComponent m_wLastFocused;
	
	protected ref array<ref Widget> m_aWidgetCells = new array<ref Widget>();
	protected ref array<ref SCR_SelectableButtonComponent> m_aButtons = new array<ref SCR_SelectableButtonComponent>();
	
	protected bool m_bIsFocused = false;
	protected bool m_bIsAscendent;
	
	ref ScriptInvoker m_OnFocus = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		// Get widgets cells 
		AddWidgetCells();

		// Disabling buttons 
		if (!w.IsEnabled())
		{
			foreach (SCR_SelectableButtonComponent button : m_aButtons)
			{
				button.SetSelected(false);
			}
			
			m_iSelectedItem = -1;
		}
		
		// Set initial selection
		if(m_iSelectedItem > -1)
		{
			if(m_aButtons.Count() > 0 && m_iSelectedItem < m_aButtons.Count())
			{
				//m_wLastFocused = m_aButtons[m_iSelectedItem]; 
				m_aButtons[m_iSelectedItem].SetSelected(true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddWidgetCells()
	{
		foreach (string str : m_aElementNames)
		{
			Widget w = m_wRoot.FindAnyWidget(str);
			if(!w)
				return;
			
			SCR_SelectableButtonComponent button = SCR_SelectableButtonComponent.Cast(w.FindHandler(SCR_SelectableButtonComponent));
			if(button)
			{
				button.m_OnClicked.Insert(OnElementClicked);
				m_aButtons.Insert(button);
			}
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnElementClicked(Widget w)
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
		SCR_SelectableButtonComponent button = SCR_SelectableButtonComponent.Cast(w.FindHandler(SCR_SelectableButtonComponent));
		
		if (!button)
			return;
		
		if (!m_aButtons || m_aButtons.Count() < 1)
			return;
		
		int buttonId = m_aButtons.Find(button);
		
		// Call focus 
		m_OnFocus.Invoke(w);
		
		button.SetSelected(false);
		
		if (buttonId >= 0)
			SetCurrentItem(buttonId, true, true);
		
		for (int x = 0; x < m_aButtons.Count(); x++)
		{
			m_aButtons[x].SetSelected(false);
		}	
		
		// Order ascendence - switch or default false
		if (m_wLastFocused == button)
			m_bIsAscendent = !m_bIsAscendent;
		else
			m_bIsAscendent = false; 
		
		m_wLastFocused = button;
		
		if(m_aElementsFilters && buttonId < m_aElementsFilters.Count())
			m_OnChanged.Invoke(m_aElementsFilters[buttonId], m_bIsAscendent);
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetLastFocusedButton() 
	{ 
		if(!m_wLastFocused)
			return null;
		return m_wLastFocused.m_wRoot; 
	}
};