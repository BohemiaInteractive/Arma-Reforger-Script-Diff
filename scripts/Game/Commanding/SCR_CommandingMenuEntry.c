//------------------------------------------------------------------------------------------------
class SCR_CommandingMenuEntry : ScriptedSelectionMenuEntry
{
	protected string m_sCommandName;
	
	//------------------------------------------------------------------------------------------------
	void SetCommandName(string commandName)
	{
		m_sCommandName = commandName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandName()
	{
		return m_sCommandName;
	}
}