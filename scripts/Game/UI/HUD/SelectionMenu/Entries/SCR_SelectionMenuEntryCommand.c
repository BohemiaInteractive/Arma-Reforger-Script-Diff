[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleUIInfo("Name")]
class SCR_SelectionMenuEntryCommand : SCR_SelectionMenuEntry
{
	protected ResourceName COMMAND_ENTRY_LAYOUT = "{A87A2A3CA0018423}UI/layouts/Common/RadialMenu/SelectionMenuEntryCommand.layout";
	
	[Attribute(desc: "Additional text to entry name")]
	protected string m_sCommandText;
	
	//------------------------------------------------------------------------------------------------
	override void SetEntryComponent(SCR_SelectionMenuEntryComponent entryComponent)
	{
		
		SCR_SelectionMenuEntryCommandComponent commandEntry = SCR_SelectionMenuEntryCommandComponent.Cast(entryComponent);
		if (commandEntry)
		{
			commandEntry.SetCommandText(m_sCommandText);
		}
		
		super.SetEntryComponent(entryComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommandText(string text)
	{
		m_sCommandText = text;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandText()
	{
		return m_sCommandText;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SelectionMenuEntryCommand()
	{
		m_sCustomLayout = COMMAND_ENTRY_LAYOUT;
	}
}