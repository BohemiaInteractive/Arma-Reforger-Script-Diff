/*
Extend error dialog class for simple dialog to parse messages 
*/

//------------------------------------------------------------------------------------------------
class ErrorDialogUI: DialogUI
{
	protected SCR_ErrorDialogDataComponent m_ErrorDialogData;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Find data component 
		m_ErrorDialogData = SCR_ErrorDialogDataComponent.Cast(GetRootWidget().FindHandler(SCR_ErrorDialogDataComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	// Set dialog texts based on ids of title and message
	void DisplayError(string title, string msg)
	{
		// Get texts 
		FindTextsInMsgConfig(title, msg);
		
		// Set texts 
		this.SetTitle(title);
		this.SetMessage(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	// Pass localized names if given names of title - message match to group - entry names
	protected void FindTextsInMsgConfig(out string title, out string msg)
	{
		// Check error data
		if (!m_ErrorDialogData)
			return;
		
		// Check message config 
	 	SCR_ErrorDialogMessages errorMessages = m_ErrorDialogData.m_ErrorDialogMessages;
		if (!errorMessages)
			return;
		
		ref array<ref SCR_ErrorDialogMessageGroup> groups = errorMessages.m_aKickCauseGroups;
		
		// Go through groups 
		foreach (SCR_ErrorDialogMessageGroup group : groups)
		{
			// Check group
			if (group.m_sName != title)
				continue;
			
			// Set localized title  
			title = group.m_sNameLocalized;
			
			// Get entries 
			ref array<ref SCR_ErrorDialogMessageEntry> entries = group.m_aKickCauseEntries;
			
			// Go through entries 
			foreach (SCR_ErrorDialogMessageEntry entry : entries)
			{
				// Set localized msg 
				if (entry.m_sName == msg)
					msg = entry.m_sNameLocalized;
			}
			
		}
	}
};