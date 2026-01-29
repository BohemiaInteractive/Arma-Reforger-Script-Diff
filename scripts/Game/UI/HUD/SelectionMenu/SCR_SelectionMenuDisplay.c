/*!
Radial menu class for handling HUD part of menu.
*/

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_SelectionMenuDisplay : SCR_InfoDisplayExtended
{	
	protected const float FADE_IN_SPEED = UIConstants.FADE_RATE_SUPER_FAST;
	
	[Attribute("", ".layout", desc: "Base layout that should be used for all entries that doesn't need custom layout.")]
	protected ResourceName m_sEntryLayout;
	
	// Widget name refs 
	[Attribute("")]
	protected string m_sEntriesParent;
	
	// Menu and entries
	protected ref SCR_SelectionMenu m_Menu;
	
	protected ref array<SCR_SelectionMenuEntry> m_aEntries = {};
	protected ref array<ref Widget> m_aEntryWidgets = {};
	
	protected SCR_SelectionMenuEntry m_LastSelectedEntry;
	protected int m_iLastSelectedId = -1;
	
	// Widgets 
	protected Widget m_wEntriesParent;
	
	//------------------------------------------------------------------------------------------------
	// Override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Set menu and register callback reactions for menu
	//! Clear callbacks if there was any other menu used
	void SetupMenu(SCR_SelectionMenu menu)
	{
		// Clear callbacks for previous menu 
		if (m_Menu)
		{
			m_Menu.GetOnOpen().Remove(OnMenuOpen);
			m_Menu.GetOnClose().Remove(OnMenuClose);
			m_Menu.GetOnUpdateEntries().Remove(OnMenuEntriesUpdate);
			m_Menu.GetOnSelect().Remove(OnMenuEntrySelected);
			m_Menu.GetOnPerform().Remove(OnMenuEntryPerform);
		}

		// Register to new menu 
		m_Menu = menu;
		
		if (!m_Menu)
		{
			DebugPrint("DisplayInit", "No menu was found!");
			return;
		}
		
		// Listen to menu 
		m_Menu.GetOnOpen().Insert(OnMenuOpen);
		m_Menu.GetOnClose().Insert(OnMenuClose);
		m_Menu.GetOnUpdateEntries().Insert(OnMenuEntriesUpdate);
		m_Menu.GetOnSelect().Insert(OnMenuEntrySelected);
		m_Menu.GetOnPerform().Insert(OnMenuEntryPerform);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup menu		
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		
		// Find widgets
		m_wEntriesParent = GetRootWidget().FindAnyWidget(m_sEntriesParent);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayOnSuspended()
	{
		super.DisplayOnSuspended();
		
		if (m_Menu)
			m_Menu.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Method ready for override to find menu at custom place
	protected void FindMenu() {}
	
	//------------------------------------------------------------------------------------------------
	//! Will create entries in HUD layout
	protected void CreateEntryWidgets()
	{
		if (!m_wEntriesParent)
		{
			DebugPrint("CreateEntryWidgets", "Can't create entries due to missing parent!");
			return;
		}
		
		if (!m_aEntries)
		{
			DebugPrint("CreateEntryWidgets", "Entries is null!");
			return;
		}
		
		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			// Pick custom resource
			ResourceName entryLayout = m_aEntries[i].GetCustomLayout();
			if (entryLayout.IsEmpty())
				entryLayout = m_sEntryLayout;
			
			// Create and setup
			Widget entry = GetGame().GetWorkspace().CreateWidgets(entryLayout, m_wEntriesParent);
			m_aEntryWidgets.Insert(entry);
			SetupEntryWidget(m_aEntries[i], entry, i);
			
			// Setup component
			SCR_SelectionMenuEntryComponent entryComponent = SCR_SelectionMenuEntryComponent.Cast(
				entry.FindHandler(SCR_SelectionMenuEntryComponent));
			
			m_aEntries[i].SetEntryComponent(entryComponent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateNewEntry(int i)
	{
		// Pick custom resource
		ResourceName entryLayout = m_aEntries[i].GetCustomLayout();
		if (entryLayout.IsEmpty())
			entryLayout = m_sEntryLayout;
		
		// Create and setup
		Widget entry = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, m_wEntriesParent);
		m_aEntryWidgets.Insert(entry);
		//SetupEntryWidget(m_aEntries[i], entry, i);
		
		// Setup component
		SCR_SelectionMenuEntryComponent entryComponent = SCR_SelectionMenuEntryComponent.Cast(
			entry.FindHandler(SCR_SelectionMenuEntryComponent));
		
		m_aEntries[i].SetEntryComponent(entryComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveEntry(Widget entry, int id)
	{
		m_aEntryWidgets[id].RemoveFromHierarchy();
		m_aEntryWidgets.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup entry widget properties specific for each menu
	//! Method can e.g. entries position - that can be different for horizontal and radial menus
	//! Ready for override
	protected void SetupEntryWidget(notnull SCR_SelectionMenuEntry entry, notnull Widget widget, int id){}
	
	//------------------------------------------------------------------------------------------------
	//! Remove all entries from HUD layout
	protected void ClearEntryWidgets()
	{
		for (int i = 0, count = m_aEntryWidgets.Count(); i < count; i++)
		{
			m_aEntryWidgets[i].RemoveFromHierarchy();
		}
		
		m_aEntryWidgets.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if used data are still same
	protected bool EntriesChanged(array<ref SCR_SelectionMenuEntry> entries)
	{
		int prevCount = m_aEntries.Count();
		
		// Check entries count
		if (prevCount == 0 || prevCount != entries.Count())
		{
			m_aEntries.Clear();
			for (int i = 0, count = entries.Count(); i < count; i++)
			{
				m_aEntries.Insert(entries[i]);
			}
			
			return true;
		}
		
		// Fill up entries
		array<SCR_SelectionMenuEntry> previousEntries = {};
		
		for (int i = 0; i < prevCount; i++)
		{
			previousEntries.Insert(m_aEntries[i]);
		}
		
		m_aEntries.Clear();
		
		for (int i = 0, count = entries.Count(); i < count; i++)
		{
			m_aEntries.Insert(entries[i]);
		}
		
		// Check if data is different
		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			if (previousEntries.IsEmpty() || !previousEntries.IsIndexValid(i))
				return true;
			
			if (previousEntries[i] != m_aEntries[i])
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpen()
	{		
		Show(true, FADE_IN_SPEED);
		
		ClearEntryWidgets();
		CreateEntryWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuClose()
	{
		Show(false, UIConstants.FADE_RATE_FAST);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuEntriesUpdate(SCR_SelectionMenu menu, array<ref SCR_SelectionMenuEntry> entries)
	{
		// Update entries
		if (EntriesChanged(entries))
		{	
			ClearEntryWidgets();
			CreateEntryWidgets();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! React on selected entry change
	protected void OnMenuEntrySelected(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry, int id) {}
	
	//------------------------------------------------------------------------------------------------
	//! React on selected entry change
	protected void OnMenuEntryPerform(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry) {}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenu GetMenu()
	{
		return m_Menu;
	}
	
	//------------------------------------------------------------------------------------------------
	// Debug 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void DebugPrint(string method, string msg)
	{
		Print(string.Format("[SCR_SelectionMenuDisplay] - %1() - '%2'", method, msg), LogLevel.DEBUG);
	}
};
