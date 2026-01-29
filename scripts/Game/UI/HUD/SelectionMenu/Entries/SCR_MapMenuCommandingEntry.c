//------------------------------------------------------------------------------------------------
class SCR_MapMenuCommandingEntry : SCR_SelectionMenuEntry
{
	string m_sIdentifier;	// string identifier
						
	//------------------------------------------------------------------------------------------------
	string GetEntryIdentifier()
	{
		return m_sIdentifier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapMenuCommandingEntry(string identifier = "")
	{
		m_sIdentifier = identifier;
	}
};
