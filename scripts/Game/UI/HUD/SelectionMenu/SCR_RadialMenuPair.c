//------------------------------------------------------------------------------------------------
//! Wrapper around list of radial menu pairs (entry, widget) to provide easy
//! way of handling entry-widget linking in the radial menu
//! ValueType should inherit from Widget
//! EntryType should inherit from BaseSelectionMenuEntry
class SCR_RadialMenuWidgetPairList
{
	protected ref array<Widget> m_aWidgets;
	protected ref array<BaseSelectionMenuEntry> m_aEntries;
	protected int m_iCount;
	
	//------------------------------------------------------------------------------------------------
	protected void RecalculateCount()
	{
		m_iCount = m_aWidgets.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aWidgets.Clear();
		m_aEntries.Clear();
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	int Count(bool recalculate = false)
	{
		if (recalculate)
			RecalculateCount();
		
		return m_iCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveAt(int index)
	{
		if (index < 0 || index >= m_iCount)
			return;
		
		m_aWidgets.Remove(index);
		m_aEntries.Remove(index);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddEntry(SCR_RadialMenuPair pair)
	{
		if (pair == null)
			return;
		
		m_aWidgets.Insert(pair.m_pWidget);
		m_aEntries.Insert(pair.m_pEntry);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddEntry(Widget widget, BaseSelectionMenuEntry entry)
	{
		ref auto pair = new SCR_RadialMenuPair(widget, entry);
		AddEntry(pair);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveByWidget(Widget widget)
	{
		if (widget == null)
			return;
		
		int index = m_aWidgets.Find(widget);
		if (index == -1)
			return;
		
		RemoveAt(index);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveByEntry(BaseSelectionMenuEntry entry)
	{	
		if (entry == null)
			return;
		
		int index = m_aEntries.Find(entry);
		if (index == -1)
			return;
		
		RemoveAt(index);
		
		RecalculateCount();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryAt(int index, BaseSelectionMenuEntry entry)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return;
		
		m_aEntries[index] = entry;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWidgetAt(int index, Widget value)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return;
		
		m_aWidgets[index] = value;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetWidgetAt(int index)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return null;
		
		return m_aWidgets[index];
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSelectionMenuEntry GetEntryAt(int index)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return null;
		
		return m_aEntries[index];
	}	
	
	//------------------------------------------------------------------------------------------------
	ref SCR_RadialMenuPair GetAt(int index)
	{
		// Invalid index
		if (index < 0 || index >= m_iCount)
			return null;
		
		ref auto pair = new SCR_RadialMenuPair(m_aWidgets[index], m_aEntries[index]);
		return pair;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuWidgetPairList()
	{
		m_aWidgets = new array<Widget>();
		m_aEntries = new array<BaseSelectionMenuEntry>();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuWidgetPairList()
	{
		m_aWidgets.Clear();
		m_aEntries.Clear();
		
		m_aWidgets = null;
		m_aEntries = null;
	}
};

//------------------------------------------------------------------------------------------------
//! Container or SCR_RadialMenu, keeps entry and widget pair
//! Both widget and entry can be null
//! Value type should most likely inherit from any Widget class
class SCR_RadialMenuPair
{
	Widget m_pWidget;
	BaseSelectionMenuEntry m_pEntry;
	
	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return (!m_pWidget && !m_pEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuPair(Widget widget, BaseSelectionMenuEntry entry)
	{
		m_pWidget = widget;
		m_pEntry = entry;
	}
};