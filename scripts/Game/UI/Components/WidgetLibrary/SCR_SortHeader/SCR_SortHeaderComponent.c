/*!
Sort header manages multiple sort element components.
At init it scans its hierarchy for any sort elements.
On clicks it ensures that only one sort element is toggled.
*/

class SCR_SortHeaderComponent : ScriptedWidgetComponent
{
	[Attribute("-1", UIWidgets.Auto, "ID of the element selected by default. When -1, none is selected at start.")]
	int m_iDefaultSortElement;
	
	// Event handlers
	ref ScriptInvoker m_OnChanged = new ScriptInvoker; // (SCR_SortHeaderComponent sortHeader)
	
	protected ref array<SCR_SortElementComponent> m_aSortElements;
	
	protected Widget m_wRoot;
	
	//---------------------------------------------------------------------------------------------------
	// P U B L I C 
	//---------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------
	ESortOrder GetSortOrder()
	{
		SCR_SortElementComponent comp = GetCurrentSortElement();
		
		if (!comp)
			return ESortOrder.NONE;
		
		return comp.GetSortOrder();
	}
	
	//---------------------------------------------------------------------------------------------------
	//! True when sort order is ASCENDING
	bool GetSortOrderAscending()
	{
		return GetSortOrder() == ESortOrder.ASCENDING;
	}
	
	//---------------------------------------------------------------------------------------------------
	int GetSortElementId()
	{
		for (int i = 0; i < m_aSortElements.Count(); i++)
		{
			if (m_aSortElements[i].GetSortOrder() != ESortOrder.NONE)
				return i;
		}
		return -1;
	}
	
	//---------------------------------------------------------------------------------------------------
	string GetSortElementName()
	{
		SCR_SortElementComponent comp = GetCurrentSortElement();
		
		if (!comp)
			return string.Empty;
		
		return comp.GetName();
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Finds element by name. Returns id, or -1 if not found.
	int FindElement(string name)
	{
		int count = m_aSortElements.Count();
		for (int i = 0; i < count; i++)
		{
			if (m_aSortElements[i].GetName() == name)
				return i;
		}
		
		return -1;
	}
	
	//---------------------------------------------------------------------------------------------------
	void SetCurrentSortElement(int id, ESortOrder order, bool useDefaultSortOrder = false, bool invokeOnChanged = true)
	{
		if (id < 0 || id >= m_aSortElements.Count())
			return;
		
		int count = m_aSortElements.Count();
		
		for (int i = 0; i < count; i++)
		{
			if (i != id)
				m_aSortElements[i].SetSortOrder(ESortOrder.NONE);
		}
		
		SCR_SortElementComponent comp = m_aSortElements[id];
		if (useDefaultSortOrder)
			comp.SetSortOrder(comp.GetDefaultSortOrder());
		else
			comp.SetSortOrder(order);
		
		if (invokeOnChanged)
			InvokeOnChanged();
	}
	
	//---------------------------------------------------------------------------------------------------
	void SetElementVisible(int id, bool visible)
	{
		if (id < 0 || id >= m_aSortElements.Count())
			return;
		
		m_aSortElements[id].GetRootWidget().SetVisible(visible);
	}
	
	//---------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	//! Sets focus on element with ID.
	void SetFocus(int id)
	{
		if (id < 0 || id >= m_aSortElements.Count())
			return;
		
		SCR_SortElementComponent comp = m_aSortElements[id];
		GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
	}
	
	//---------------------------------------------------------------------------------------------------
	// P R O T E C T E D
	//---------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_aSortElements = {};
		FindAllSortButtons(w, m_aSortElements);
		
		// Select default sort element
		if (m_iDefaultSortElement != -1)
			SetCurrentSortElement(m_iDefaultSortElement, ESortOrder.NONE, useDefaultSortOrder:true, invokeOnChanged: false);
		
		// Subscribe to events of buttons
		foreach (SCR_SortElementComponent comp : m_aSortElements)
		{
			comp.m_OnClicked.Insert(OnButtonClicked);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Find all sort button components recursively
	protected static void FindAllSortButtons(Widget w, array<SCR_SortElementComponent> components)
	{
		SCR_SortElementComponent comp = SCR_SortElementComponent.Cast(w.FindHandler(SCR_SortElementComponent));
		if (comp)
			components.Insert(comp);
		
		Widget child = w.GetChildren();
		while (child)
		{
			FindAllSortButtons(child, components);
			child = child.GetSibling();
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnButtonClicked(SCR_ModularButtonComponent compClicked)
	{
		// Untoggle all other buttons
		foreach (SCR_SortElementComponent comp : m_aSortElements)
		{
			if (comp != compClicked && comp.GetSortOrder() != ESortOrder.NONE)
				comp.SetSortOrder(ESortOrder.NONE);
		}
		
		InvokeOnChanged();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected SCR_SortElementComponent GetCurrentSortElement()
	{
		foreach (SCR_SortElementComponent comp : m_aSortElements)
		{
			if (comp.GetSortOrder() != ESortOrder.NONE)
				return comp;
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void InvokeOnChanged()
	{		
		m_OnChanged.Invoke(this);
	}
};