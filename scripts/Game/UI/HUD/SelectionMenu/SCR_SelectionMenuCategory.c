//------------------------------------------------------------------------------------------------
/*!
Scripted selection menu category, that can contain multiple entries
*/
[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleUIInfo("Name")]
class SCR_SelectionMenuCategoryEntry : SCR_SelectionMenuEntry
{
	[Attribute("-1", desc: "Entry with this index will be used as deafult action")]
	protected int m_iDefaultEntry;
	
	[Attribute()]
	protected ref array<ref SCR_SelectionMenuEntry> m_aEntries;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
		
	//------------------------------------------------------------------------------------------------
	override protected void OnPerform()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void PerformDefaultEntry()
	{
		if (m_aEntries.IsIndexValid(m_iDefaultEntry))
			m_aEntries[m_iDefaultEntry].Perform();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddEntry(SCR_SelectionMenuEntry entry = null)
	{
		if (!m_aEntries)
			m_aEntries = new array<ref SCR_SelectionMenuEntry>();
		
		if (!entry)
			SCR_SelectionMenuEntry newEntry = new SCR_SelectionMenuEntry();
		
		m_aEntries.Insert(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull SCR_SelectionMenuEntry entry)
	{
		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	int GetDefaultAction()
	{
		return m_iDefaultEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_SelectionMenuEntry> GetEntries()
	{
		if (!m_aEntries)
			m_aEntries = new array<ref SCR_SelectionMenuEntry>();
		
		array<ref SCR_SelectionMenuEntry> entries = {};
		
		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}
		
		return entries;
	}
}