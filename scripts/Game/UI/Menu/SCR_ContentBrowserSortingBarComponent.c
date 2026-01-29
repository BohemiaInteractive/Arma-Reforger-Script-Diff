class SCR_ContentBrowserSortingBar : SCR_ServerFilterBarComponent
{
	
	[Attribute()]
	bool m_bDefaultSelected = true;
	
	SCR_ButtonImageComponent m_CurrentSelection;
	protected int m_iCurrentIndex;
	protected ref array<SCR_ButtonImageComponent> m_aButtons = new array<SCR_ButtonImageComponent>();
	
	ref ScriptInvoker m_OnSortingChange = new ScriptInvoker();
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_iCurrentIndex = m_aButtons.Count() - 1;
		m_CurrentSelection = m_aButtons[m_iCurrentIndex];
		
		m_aButtons[m_iCurrentIndex].SetToggled(true, false, false);
	}
	
	
	protected override void ListenToSelectables(notnull array<Widget> aWidgets)
	{
		foreach (Widget w : aWidgets)
		{	
			SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(w.FindHandler(SCR_ButtonImageComponent));		
			if (!comp)
				return;
			m_aButtons.Insert(comp);
			comp.m_OnToggled.Insert(OnSorting);
			//Print("attach listener");
		}
	}
	
	int GetSelectedIndex()
	{
		return m_iCurrentIndex;
	}
	
	protected void OnSorting(SCR_ButtonImageComponent comp, bool isSelected)
	{
		int index = -1;
		if (isSelected)
		{
			if (m_CurrentSelection)
				m_CurrentSelection.SetToggled(false, true, false);
			m_CurrentSelection = comp;
			index = m_aButtons.Find(comp);
			m_iCurrentIndex = index;
		}
		else
		{
			m_CurrentSelection.SetToggled(true, false, false);
			return;
		}
		
		m_OnSortingChange.Invoke(index);
		
	}
};