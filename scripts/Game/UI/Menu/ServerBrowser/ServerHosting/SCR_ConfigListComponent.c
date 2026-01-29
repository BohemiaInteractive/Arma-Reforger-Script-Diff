/*!
Compomenent with simple configuratino that will create list of given widgets
*/

class SCR_ConfigListComponent : ScriptedWidgetComponent
{	
	[Attribute("VerticalLayout")]
	protected string m_sListParent;
	
	[Attribute()]
	protected ref array<ref SCR_WidgetListEntry> m_aInitialEntryList;
	
	protected Widget m_Root;
	protected ScrollLayoutWidget m_wScrollWidget;
	protected VerticalLayoutWidget m_wList;
	
	//-------------------------------------------------------------------------------------------
	// Overridden widget api
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		
		m_wList = VerticalLayoutWidget.Cast(w.FindAnyWidget(m_sListParent));
		if (!m_wList)
			m_wList = VerticalLayoutWidget.Cast(w);
		
		for (int i = 0, count = m_aInitialEntryList.Count(); i < count; i++)
		{
			m_aInitialEntryList[i].CreateWidget(m_wList);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		foreach (SCR_WidgetListEntry entry : m_aInitialEntryList)
		{
			entry.RemoveWidget();
		}
	}
	
	//-------------------------------------------------------------------------------------------
	// API
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	array<ref SCR_WidgetListEntry> GetInitialEntryList()
	{
		return m_aInitialEntryList;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Find variable of given property name and group tag - no group tag find any variable of given property name
	SCR_WidgetListEntry FindEntry(string propertyName, string groupTag = "")
	{
		for (int i = 0, count = m_aInitialEntryList.Count(); i < count; i++)
		{
			if (groupTag == "")
			{
				// Has given property name
				if (m_aInitialEntryList[i].GetPropertyName() == propertyName)
					return m_aInitialEntryList[i];
			}
			else 
			{
				// Has propery and group
				if (m_aInitialEntryList[i].GetPropertyName() == propertyName && m_aInitialEntryList[i].GetGroupTag() == groupTag)
					return m_aInitialEntryList[i];
			}
		}
		
		return null;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Go through list and check if values are correct 
	//! Return first invalid entry widget  
	//! Return null if all values are valid
	Widget GetInvalidEntry()
	{
		// Go through entries list
		for (int i = 0, count = m_aInitialEntryList.Count(); i < count; i++)
		{
			if (!m_aInitialEntryList[i].CheckValidity())
			{
				// Invalid entry widget
				return m_aInitialEntryList[i].GetEntryRoot();
			}
		}
		
		// Is valid
		return null;
	}
};