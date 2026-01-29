//------------------------------------------------------------------------------------------------
//! An abstract class to be implemented by the user to provide the WeaponSwitchingGroupManager 
//! with a valid means of communication with the game world for the purpose of weapon switching
class SCR_BaseGroup : ScriptedSelectionMenuEntry
{	
	//! Owner of this group
	protected IEntity m_pOwner;
	//! List of all entries in this group
	protected ref array<ref SCR_BaseGroupEntry> m_aEntries;
	//! Currently selected entry or -1 if none
	protected int m_iSelectedIndex;
	//! Currently selected entry or null if none
	protected SCR_BaseGroupEntry m_pSelectedEntry;
	//! The menu we belong to
	protected BaseSelectionMenu m_pSourceMenu;
	
	//------------------------------------------------------------------------------------------------
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		// TODO: Define actual logic?
		if (m_iSelectedIndex < 0)
			return false;
		
		if (m_iSelectedIndex > Count() - 1)
			return false;
		
		return m_aEntries[m_iSelectedIndex].CanBePerformed(user, sourceMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		if (m_pSelectedEntry)
		{
			outName = m_pSelectedEntry.GetEntryName();
			return true;
		}
		
		outName = "Empty";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override UIInfo GetUIInfoScript()
	{
		if (m_pSelectedEntry)
		{
			return m_pSelectedEntry.GetUIInfo();
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! How many items this group contains
	int Count() { return m_aEntries.Count(); }
	
	//------------------------------------------------------------------------------------------------
	//! Select next item
	void SelectNext()
	{
		auto index = m_iSelectedIndex+1;
		if (index > Count() -1)
			index = 0;
		
		Select(index);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Select next item
	void SelectPrevious()
	{
		auto index = m_iSelectedIndex-1;
		if (index < 0)
			index = Count() - 1;
		
		Select(index);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Select item from this group at given index as the current item
	//! \return Returns true when selection was successful, false otherwise.
	void Select(int index) 
	{ 
		// Deselect selected item
		if (index == -1)
		{
			if (m_pSelectedEntry)
				m_pSelectedEntry.OnDonePerform(m_pOwner, m_pSourceMenu);
			
			
			m_pSelectedEntry = null;
			m_iSelectedIndex = -1;
			return;
		}
		
		// Make sure selection is valid and not out of range
		if (index < 0 || index > Count() -1)
			return;
		
		// Try to actually select the entry
		SCR_BaseGroupEntry entry = m_aEntries[index];
		if (entry)
		{
			// Deselect previous entry
			if (m_pSelectedEntry && m_pSelectedEntry != entry) 
			{
				m_pSelectedEntry.OnDonePerform(m_pOwner, m_pSourceMenu);
				m_pSelectedEntry = null;
			}
			
			m_pSelectedEntry = entry;
			m_iSelectedIndex = index;
		}
	}
	
	//! Callback for when this entry is supposed to be performed
	protected override void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (m_pSelectedEntry)
		{
			m_pSelectedEntry.Perform(user, sourceMenu);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the owner of this group
	IEntity GetOwner() { return m_pOwner; }
	
	//------------------------------------------------------------------------------------------------
	//! Return the currently selected item
	SCR_BaseGroupEntry GetSelectedEntry() { return m_pSelectedEntry; }	
	
	//------------------------------------------------------------------------------------------------
	//! Create a group with provided entries
	void SCR_BaseGroup(IEntity owner, array<ref SCR_BaseGroupEntry> entries)
	{
		// Assign the owner entity
		m_pOwner = owner;
		
		// Prepare array and set all the entries
		m_aEntries = new array<ref SCR_BaseGroupEntry>();
		foreach (auto entry : entries)
			m_aEntries.Insert(entry);
	}
};
